/*
 * test_Environment.cpp
 *
 *  Created on: 2013-3-10
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_trade_sys_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>

using namespace hku;

class EnvironmentTest: public EnvironmentBase {
public:
    EnvironmentTest(): EnvironmentBase("TEST") {
        setParam<int>("n", 10);
    }

    virtual ~EnvironmentTest() {}

    virtual void _calculate() {
        _addValid(Datetime(200001010000LL));
    }

    virtual EnvironmentPtr _clone() {
        return make_shared<EnvironmentTest>();
    }

};

/**
 * @defgroup test_Environment test_Environment
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Environment ) {
    /** @arg 基本操作 */
    EnvironmentPtr p = make_shared<EnvironmentTest>();
    BOOST_CHECK(p->name() == "TEST");
    BOOST_CHECK(p->isValid(Datetime(200001010000)) == false);
    BOOST_CHECK(p->isValid(Datetime(200001020000)) == false);

    p->setQuery(KQuery(-100));
    BOOST_CHECK(p->isValid(Datetime(200001010000)) == true);
    BOOST_CHECK(p->isValid(Datetime(200001020000)) == false);
    BOOST_CHECK(p->getParam<int>("n") == 10);

    /** @arg 克隆操作 */
    p->setParam<int>("n", 20);
    EnvironmentPtr p_clone = p->clone();
    BOOST_CHECK(p != p_clone);
    BOOST_CHECK(p_clone->name() == "TEST");
    BOOST_CHECK(p_clone->isValid(Datetime(200001010000)) == true);
    BOOST_CHECK(p_clone->isValid(Datetime(200001020000)) == false);
    BOOST_CHECK(p_clone->getParam<int>("n") == 20);
}


/** @} */
