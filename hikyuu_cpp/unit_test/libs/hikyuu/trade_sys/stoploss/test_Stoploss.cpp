/*
 * test_Stoploss.cpp
 *
 *  Created on: 2013-3-13
 *      Author: fasiondog
 */


#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_trade_sys_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/stoploss/StoplossBase.h>

using namespace hku;

class StoplossTest: public StoplossBase {
public:
    StoplossTest(): StoplossBase("StoplossTest"), m_x(0) {}
    virtual ~StoplossTest() {}

    virtual price_t getPrice(const Datetime& datetime, price_t price) {
        return m_x < 10 ? 0.0 : 1.0;
    }

    virtual void _reset() {
        m_x = 0;
    }

    virtual StoplossPtr _clone() {
        StoplossTest *p = new StoplossTest;
        p->m_x = m_x;
        return StoplossPtr(p);
    }

    virtual void _calculate() {}

    int getX() const { return m_x;}
    void setX(int x) { m_x = x; }

private:
    int m_x;
};

/**
 * @defgroup test_Stoploss test_Stoploss
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Stoploss) {
    /** @arg 基本操作 */
    StoplossPtr p(new StoplossTest);
    BOOST_CHECK(p->name() == "StoplossTest");
    BOOST_CHECK(p->getPrice(Datetime(200101010000), 1.0) == 0.0);
    StoplossTest *p_src = (StoplossTest *)p.get();
    BOOST_CHECK(p_src->getX() == 0);

    p_src->setX(10);
    BOOST_CHECK(p->getPrice(Datetime(200101010000), 1.0) == 1.0);
    BOOST_CHECK(p_src->getX() == 10);
    p->reset();
    BOOST_CHECK(p_src->getX() == 0);

    /** @arg 测试克隆操作 */
    p_src->setX(10);
    StoplossPtr p_clone = p->clone();
    BOOST_CHECK(p_clone->name() == "StoplossTest");
    p_src = (StoplossTest *)p_clone.get();
    BOOST_CHECK(p_src->getX() == 10);
    BOOST_CHECK(p != p_clone);
}

/** @} */


