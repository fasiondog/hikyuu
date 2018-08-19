/*
 * test_Condition.cpp
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
#include <hikyuu/trade_sys/condition/ConditionBase.h>

using namespace hku;

class ConditionTest: public ConditionBase {
public:
    ConditionTest(): ConditionBase("TEST") {
        setParam<int>("n", 10);
        m_flag = false;
    }

    ~ConditionTest() {}

    KData getKData() const {
        return m_kdata;
    }

    virtual void _calculate() {}
    virtual void _reset() {
        if (m_flag) {
            m_flag = false;
        } else {
            m_flag = true;
        }
    }

    virtual ConditionPtr _clone() {
        ConditionTest* p = new ConditionTest;
        p->m_flag = m_flag;
        return ConditionPtr(p);
    }

private:
    bool m_flag;

};

/**
 * @defgroup test_Condition test_Condition
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Condition ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    ConditionTest *p_src;

    /** @arg 基本操作 */
    ConditionPtr p(new ConditionTest);
    p_src = (ConditionTest *)p.get();
    BOOST_CHECK(p_src->getKData().getStock() == Null<Stock>());
    p->setTO(kdata);
    BOOST_CHECK(p_src->getKData().getStock() == stock);
    p->reset();

    BOOST_CHECK(p->name() == "TEST");
    BOOST_CHECK(p->getParam<int>("n") == 10);
    BOOST_CHECK(p->isValid(Datetime(200001010000)) == false);
    p->reset();
    //BOOST_CHECK(p->isValid(Datetime(200001010000)) == true);

    /** @arg 克隆操作 */
    p->setParam<int>("n", 20);
    ConditionPtr p_clone = p->clone();
    BOOST_CHECK(p != p_clone);
    p_src = (ConditionTest *)p_clone.get();
    BOOST_CHECK(p_src->getKData().getStock() == stock);

    p->setParam<int>("n", 30);
    BOOST_CHECK(p->getParam<int>("n") == 30);
    p->reset();
    BOOST_CHECK(p->isValid(Datetime(200001010000)) == false);

    BOOST_CHECK(p_clone->name() == "TEST");
    BOOST_CHECK(p_clone->getParam<int>("n") == 20);
    //BOOST_CHECK(p_clone->isValid(Datetime(200001010000)) == true);
    p_clone->reset();
    BOOST_CHECK(p_clone->isValid(Datetime(200001010000)) == false);
}

/** @} */



