/*
 * test_Slippage.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/slippage/SlippageBase.h>
#include <hikyuu/trade_sys/slippage/crt/SP_FixedPercent.h>

using namespace hku;

class SlippageTest : public SlippageBase {
public:
    SlippageTest() : SlippageBase("SlippageTest"), m_x(0) {}
    virtual ~SlippageTest() {}

    virtual price_t _getRealBuyPrice(const Datetime &datetime, price_t price) override {
        return m_x < 10 ? 0.0 : 1.0;
    }

    virtual price_t _getRealSellPrice(const Datetime &datetime, price_t price) override {
        return m_x < 10 ? 0.0 : 1.0;
    }

    virtual void _reset() override {
        m_x = 0;
    }

    virtual SlippagePtr _clone() override {
        SlippagePtr p = std::make_shared<SlippageTest>();
        p->reset();
        return p;
    }

    virtual void _calculate() override {}

    int getX() const {
        return m_x;
    }
    void setX(int x) {
        m_x = x;
    }

private:
    int m_x;
};

/**
 * @defgroup test_Slippage test_Slippage
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Slippage") {
    /** @arg The basic operation */
    SlippagePtr p(new SlippageTest);
    CHECK_EQ(p->name(), "SlippageTest");
    CHECK_EQ(p->getRealBuyPrice(Datetime(200101010000), 1.0), 0.0);
    CHECK_EQ(p->getRealSellPrice(Datetime(200101010000), 1.0), 0.0);
    SlippageTest *p_src = (SlippageTest *)p.get();
    CHECK_EQ(p_src->getX(), 0);

    p_src->setX(10);
    CHECK_EQ(p->getRealBuyPrice(Datetime(200101010000), 1.0), 1.0);
    CHECK_EQ(p->getRealSellPrice(Datetime(200101010000), 1.0), 1.0);
    CHECK_EQ(p_src->getX(), 10);
    p->reset();
    CHECK_EQ(p_src->getX(), 0);

    /** @arg Test the clone operation: the clone is a distinct object and resets its internal
     *        state (e.g. reseeds the RNG from the seed), so transient state is not copied */
    p_src->setX(10);
    SlippagePtr p_clone = p->clone();
    CHECK_EQ(p_clone->name(), "SlippageTest");
    p_src = (SlippageTest *)p_clone.get();
    CHECK_EQ(p_src->getX(), 0);
    CHECK_NE(p, p_clone);
}

/** @par Test points */
TEST_CASE("test_Slippage_clone_lazy_recalculate") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sz000001").getKData(KQuery(0, 10));
    CHECK_UNARY(!k.empty());
    Datetime d0 = k[0].datetime;
    price_t plan = 10.0;
    price_t slipped = plan * 1.001;

    auto sp = SP_FixedPercent(0.001);
    sp->setTO(k);
    CHECK_EQ(sp->getRealBuyPrice(d0, plan), slipped);

    /** @arg the clone carries the source TO, so its prices are already constrained */
    SlippagePtr c = sp->clone();
    CHECK_NE(sp, c);
    CHECK_UNARY(!c->getTO().empty());
    CHECK_EQ(c->getRealBuyPrice(d0, plan), slipped);

    /** @arg setTO rebinds the clone and keeps the same result */
    c->setTO(k);
    CHECK_EQ(c->getRealBuyPrice(d0, plan), slipped);
}

/** @} */
