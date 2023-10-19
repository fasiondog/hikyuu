/*
 * test_Signal.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>

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

    virtual void _reset() {
        m_x = 0;
    }

    virtual SignalPtr _clone() {
        SignalTest *p = new SignalTest;
        p->m_x = m_x;
        return SignalPtr(p);
    }

    virtual void _calculate() {}

private:
    int m_x;
};

/**
 * @defgroup test_Signal test_Signal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_Signal") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");

    SignalPtr p(new SignalTest);
    SignalTest *p_src = (SignalTest *)p.get();

    SUBCASE("Basic operation") {
        /** @arg 基本操作 */
        CHECK_EQ(p_src->getX(), 0);
        CHECK_EQ(p->name(), "SignalBase");
        p->name("SignalTest");
        CHECK_EQ(p->name(), "SignalTest");
        CHECK_EQ(p->shouldBuy(Datetime(200101010000)), false);
        p->_addBuySignal(Datetime(200101010000));
        CHECK_EQ(p->shouldBuy(Datetime(200101010000)), true);
        CHECK_EQ(p->shouldSell(Datetime(200101030000)), false);
        p->_addSellSignal(Datetime(200101030000));
        CHECK_EQ(p->shouldSell(Datetime(200101030000)), true);

        /** @arg 克隆操作 */
        p_src->setX(10);
        SignalPtr p_clone = p->clone();
        CHECK_NE(p, p_clone);
        p_src = (SignalTest *)p_clone.get();
        CHECK_EQ(p_src->getX(), 10);
        CHECK_EQ(p_clone->shouldBuy(Datetime(200101010000)), true);
        CHECK_EQ(p_clone->shouldSell(Datetime(200101030000)), true);
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

/** @} */
