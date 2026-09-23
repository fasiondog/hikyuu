/*
 * test_Signal.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cycle.h>
#include <hikyuu/KData.h>

using namespace hku;

class SignalTest : public SignalBase {
public:
    SignalTest() : SignalBase(), m_x(0) {}
    virtual ~SignalTest() {}

    void setX(int x) {
        m_x = x;
    }
    int getX() const {
        return m_x;
    }

    virtual void _reset() override {
        m_x = 0;
    }

    virtual SignalPtr _clone() override {
        SignalTest *p = new SignalTest;
        p->m_x = m_x;
        return SignalPtr(p);
    }

    virtual void _calculate(const KData &) override {}

private:
    int m_x;
};

/**
 * @defgroup test_Signal test_Signal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Signal") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");

    SignalPtr p(new SignalTest);
    SignalTest *p_src = (SignalTest *)p.get();

    SUBCASE("Basic operation") {
        /** @arg The basic operation */
        CHECK_EQ(p_src->getX(), 0);
        CHECK_EQ(p->name(), "SignalBase");
        p->name("SignalTest");
        CHECK_EQ(p->name(), "SignalTest");
        CHECK_EQ(p->shouldBuy(Datetime(200101010000)), false);
        p->_addBuySignal(Datetime(200101010000));
        CHECK_EQ(p->shouldBuy(Datetime(200101010000)), true);
        CHECK_EQ(p->getBuyValue(Datetime(200101010000)), 1.0);
        CHECK_EQ(p->shouldSell(Datetime(200101030000)), false);
        CHECK_EQ(p->getSellValue(Datetime(200101030000)), 0.0);
        p->_addSellSignal(Datetime(200101030000));
        CHECK_EQ(p->shouldSell(Datetime(200101030000)), true);

        /** @arg The clone operation */
        p_src->setX(10);
        SignalPtr p_clone = p->clone();
        CHECK_NE(p, p_clone);
        p_src = (SignalTest *)p_clone.get();
        CHECK_EQ(p_src->getX(), 10);
        CHECK_EQ(p_clone->shouldBuy(Datetime(200101010000)), true);
        CHECK_EQ(p_clone->shouldSell(Datetime(200101030000)), true);

        /** @arg Insert a duplicate buy date */
        p->reset();
        p->setParam<bool>("alternate", true);
        REQUIRE(!p->shouldBuy(Datetime(200201010000)));
        p->_addBuySignal(Datetime(200201010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200201010000)));
        p->_addBuySignal(Datetime(200201010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200201010000)));

        p->reset();
        p->setParam<bool>("alternate", false);
        REQUIRE(!p->shouldBuy(Datetime(200201010000)));
        p->_addBuySignal(Datetime(200201010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200201010000)));
        p->_addBuySignal(Datetime(200201010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200201010000)));

        /** @arg Insert a duplicate sell date */
        p->reset();
        p->setParam<bool>("alternate", true);
        p->_addBuySignal(Datetime(200201010000));
        REQUIRE(!p->shouldSell(Datetime(200201020000)));
        p->_addSellSignal(Datetime(200201020000));
        CHECK_UNARY(p->shouldSell(Datetime(200201020000)));
        p->_addSellSignal(Datetime(200201020000));
        CHECK_UNARY(p->shouldSell(Datetime(200201020000)));

        p->reset();
        p->setParam<bool>("alternate", false);
        REQUIRE(!p->shouldSell(Datetime(200201020000)));
        p->_addSellSignal(Datetime(200201020000));
        CHECK_UNARY(p->shouldSell(Datetime(200201020000)));
        p->_addSellSignal(Datetime(200201020000));
        CHECK_UNARY(p->shouldSell(Datetime(200201020000)));

        /** @arg Insert a buy date that already has a sell indication */
        p->reset();
        p->setParam<bool>("alternate", false);
        p->_addSellSignal(Datetime(200202010000));
        REQUIRE(p->shouldSell(Datetime(200202010000)));
        p->_addBuySignal(Datetime(200202010000));
        CHECK_UNARY(!p->shouldBuy(Datetime(200202010000)));

        p->reset();
        p->setParam<bool>("alternate", true);
        p->_addBuySignal(Datetime(200201010000));
        p->_addSellSignal(Datetime(200202010000));
        REQUIRE(p->shouldSell(Datetime(200202010000)));
        p->_addBuySignal(Datetime(200202010000));
        CHECK_UNARY(!p->shouldBuy(Datetime(200202010000)));

        /** @arg Insert a sell date that already has a buy indication */
        p->reset();
        p->setParam<bool>("alternate", false);
        p->_addBuySignal(Datetime(200202010000));
        p->_addSellSignal(Datetime(200202010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200202010000)));
        CHECK_UNARY(!p->shouldSell(Datetime(200202010000)));

        p->reset();
        p->setParam<bool>("alternate", true);
        p->_addBuySignal(Datetime(200202010000));
        p->_addSellSignal(Datetime(200202010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200202010000)));
        CHECK_UNARY(!p->shouldSell(Datetime(200202010000)));
    }

    SUBCASE("Short sell") {
        p->name("SignalTest");
        p->setParam<bool>("support_borrow_stock", true);
        p->_addSellSignal(Datetime(200101010000));
        p->_addSellSignal(Datetime(200101030000));
        CHECK_EQ(p->shouldSell(Datetime(200101010000)), true);
        CHECK_EQ(p->shouldSell(Datetime(200101030000)), false);

        p->reset();
        p->setParam<bool>("support_borrow_stock", false);
        p->_addSellSignal(Datetime(200101010000));
        p->_addBuySignal(Datetime(200101030000));
        p->_addSellSignal(Datetime(200101040000));
        CHECK_EQ(p->shouldSell(Datetime(200101010000)), false);
        CHECK_EQ(p->shouldSell(Datetime(200101040000)), true);
    }
}

/** @par Test points */
TEST_CASE("test_Signal_clone_cycle_bounds") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData k = stock.getKData(KQuery(-30));
    REQUIRE(k.size() >= 15);

    SignalPtr p(new SignalTest);
    p->setParam<bool>("cycle", true);
    p->setTO(k);

    Datetime t0 = k[0].datetime;
    Datetime t1 = k[10].datetime;
    REQUIRE(t0 < t1);
    p->startCycle(t0, t1);

    CHECK_EQ(p->getCycleStart(), t0);
    CHECK_EQ(p->getCycleEnd(), t1);

    /** @arg After clone the cycle boundary must be copied completely, end must not become start */
    SignalPtr c = p->clone();
    CHECK_NE(p, c);
    CHECK_EQ(c->getCycleStart(), t0);
    CHECK_EQ(c->getCycleEnd(), t1);
}

/** @par Test points */
TEST_CASE("test_Signal_clone_operator_behavior") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData k = stock.getKData(KQuery(-30));
    REQUIRE(k.size() >= 15);

    Datetime t0 = k[0].datetime;
    Datetime t1 = k[10].datetime;
    REQUIRE(t0 < t1);

    // The parent signal: cycle=true and startCycle already called (an Operator parent holds it)
    SignalPtr parent(new SignalTest);
    parent->setParam<bool>("cycle", true);
    parent->setTO(k);
    parent->startCycle(t0, t1);
    CHECK_EQ(parent->getCycleStart(), t0);
    CHECK_EQ(parent->getCycleEnd(), t1);

    // The boundary must be complete after clone; before the fix end became t0
    SignalPtr cloned = parent->clone();
    CHECK_EQ(cloned->getCycleStart(), t0);
    CHECK_EQ(cloned->getCycleEnd(), t1);

    // Simulate the cascading of OperatorSignal::sub_sg_calculate:
    //   child->startCycle(parent.m_cycle_start, parent.m_cycle_end)
    // Before the fix: startCycle(t0, t0) triggers HKU_CHECK(start < close)
    // After the fix: startCycle(t0, t1) passes legally and produces a buy signal
    auto child = SG_Cycle();
    child->setTO(k);
    REQUIRE_NOTHROW(child->startCycle(cloned->getCycleStart(), cloned->getCycleEnd()));
    CHECK_EQ(child->getCycleStart(), t0);
    CHECK_EQ(child->getCycleEnd(), t1);
    CHECK_EQ(child->getBuyValue(t0), 1.0);
}

/** @} */
