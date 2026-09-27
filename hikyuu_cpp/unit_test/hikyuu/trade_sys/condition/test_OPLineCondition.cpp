/*
 * test_OPLineCondition.cpp
 *
 *  Created on: 2026-09-28
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/condition/crt/CN_OPLine.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/EMA.h>

using namespace hku;

namespace {

// A simple alternating long/short SG that counts how many times it is calculated
class CountingSG : public SignalBase {
public:
    CountingSG() : SignalBase("TEST_COUNTING_SG") {}

    int calculateCount() const {
        return m_calc_count;
    }

    virtual void _calculate(const KData& kdata) override {
        m_calc_count++;
        bool hold = false;
        for (size_t i = 1; i < kdata.size(); ++i) {
            if (!hold && kdata[i].closePrice > kdata[i - 1].closePrice) {
                _addBuySignal(kdata[i].datetime);
                hold = true;
            } else if (hold && kdata[i].closePrice < kdata[i - 1].closePrice) {
                _addSellSignal(kdata[i].datetime);
                hold = false;
            }
        }
    }

    virtual SignalPtr _clone() override {
        return make_shared<CountingSG>();
    }

private:
    int m_calc_count{0};
};

}  // namespace

/**
 * @defgroup test_OPLineCondition test_OPLineCondition
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test point: the embedded backtest must not reset or re-run the outer SG */
TEST_CASE("test_OPLineCondition_embedded_backtest_keeps_outer_sg") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    REQUIRE_FALSE(stock.isNull());
    KData kdata = stock.getKData(KQuery(0, 60, KQuery::DAY));
    REQUIRE_EQ(kdata.size(), 60);

    /** @arg the outer system sets the SG TO first, then the CN runs its embedded backtest */
    SGPtr sg = make_shared<CountingSG>();
    sg->setTO(kdata);
    CHECK_EQ(std::dynamic_pointer_cast<CountingSG>(sg)->calculateCount(), 1);

    CNPtr cn = CN_OPLine(PRICELIST(kdata.size(), 0.0));
    cn->setSG(sg);
    cn->setTO(kdata);

    CHECK_EQ(cn->size(), kdata.size());
    /** @arg the outer SG is not reset and re-run by the embedded sys, still calculated once */
    CHECK_EQ(std::dynamic_pointer_cast<CountingSG>(sg)->calculateCount(), 1);

    /** @arg the outer SG still refers to the original KData */
    CHECK_EQ(sg->getTO(), kdata);

    /** @arg the embedded backtest did run and produced valid dates */
    CHECK_FALSE(cn->getDatetimeList().empty());
}

/** @par Test point: an OP line indicator with a discard window keeps the result size aligned */
TEST_CASE("test_OPLineCondition_op_with_discard") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    REQUIRE_FALSE(stock.isNull());
    KData kdata = stock.getKData(KQuery(0, 60, KQuery::DAY));
    REQUIRE_EQ(kdata.size(), 60);

    SGPtr sg = make_shared<CountingSG>();
    sg->setTO(kdata);
    CNPtr cn = CN_OPLine(EMA(5));
    cn->setSG(sg);

    /** @arg no throw and the value array is as long as the KData */
    CHECK_NOTHROW(cn->setTO(kdata));
    CHECK_EQ(cn->size(), kdata.size());
    CHECK_EQ(std::dynamic_pointer_cast<CountingSG>(sg)->calculateCount(), 1);
}

/** @} */
