/*
 * test_Signal.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */


#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_trade_sys_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>

using namespace hku;

class SignalTest: public SignalBase {
public:
    SignalTest(): SignalBase(), m_x(0) {}
    virtual ~SignalTest() {}

    void setX(int x) { m_x = x; }
    int getX() const { return m_x; }

    virtual void _reset() {
        m_x = 0;
    }

    virtual SignalPtr _clone() {
        SignalTest *p = new SignalTest;
        p->m_x = m_x;
        return SignalPtr(p);
    }

    virtual void _calculate() {

    }

private:
    int m_x;

};

/**
 * @defgroup test_Signal test_Signal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Signal) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");

    /** @arg 基本操作 */
    SignalPtr p(new SignalTest);
    SignalTest *p_src = (SignalTest *)p.get();
    BOOST_CHECK(p_src->getX() == 0);
    BOOST_CHECK(p->name() == "SignalBase");
    p->name("SignalTest");
    BOOST_CHECK(p->name() == "SignalTest");
    BOOST_CHECK(p->shouldBuy(Datetime(200101010000)) == false);
    p->_addBuySignal(Datetime(200101010000));
    BOOST_CHECK(p->shouldBuy(Datetime(200101010000)) == true);
    BOOST_CHECK(p->shouldSell(Datetime(200101030000)) == false);
    p->_addSellSignal(Datetime(200101030000));
    BOOST_CHECK(p->shouldSell(Datetime(200101030000)) == true);

    /** @arg 克隆操作 */
    p_src->setX(10);
    SignalPtr p_clone = p->clone();
    BOOST_CHECK(p != p_clone);
    p_src = (SignalTest *)p_clone.get();
    BOOST_CHECK(p_src->getX() == 10);
    BOOST_CHECK(p_clone->shouldBuy(Datetime(200101010000)) == true);
    BOOST_CHECK(p_clone->shouldSell(Datetime(200101030000)) == true);
}

/** @} */





