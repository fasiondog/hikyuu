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

#if 0

class EnvironmentTest: public EnvironmentBase {
public:
    EnvironmentTest(): EnvironmentBase("TEST") {
        setParam<int>("n", 10);
        m_market = "SH";
    }

    ~EnvironmentTest() {}

    virtual bool isValid(const string& market, const Datetime& datetime) {
        return market == m_market ? true : false;
    }

    virtual void _reset() {
        if (m_market == "SH" ) {
            m_market = "SZ";
        } else {
            m_market = "SH";
        }
    }

    virtual void _calculate() {}

    virtual EnvironmentPtr _clone() {
        EnvironmentTest* p(new EnvironmentTest);
        p->m_market = m_market;
        return EnvironmentPtr(p);
    }

    KQuery::KType getKType() const {
        return m_ktype;
    }

    void settKType(KQuery::KType ktype) {
        m_ktype = ktype;
        _calculate();
    }

private:
    string m_market;
    KQuery::KType m_ktype;
};

/**
 * @defgroup test_Environment test_Environment
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Environment ) {
    /** @arg 基本操作 */
    EnvironmentPtr p(new EnvironmentTest);
    BOOST_CHECK(p->name() == "TEST");
    BOOST_CHECK(p->isValid("SH", Datetime(200001010000)) == true);
    BOOST_CHECK(p->isValid("SZ", Datetime(200001010000)) == false);
    BOOST_CHECK(p->getParam<int>("n") == 10);
    p->reset();
    BOOST_CHECK(p->isValid("SH", Datetime(200001010000)) == false);
    BOOST_CHECK(p->isValid("SZ", Datetime(200001010000)) == true);

    /** @arg 克隆操作 */
    p->setParam<int>("n", 20);
    p->setKType(KQuery::MIN);
    EnvironmentPtr p_clone = p->clone();
    BOOST_CHECK(p != p_clone);
    BOOST_CHECK(p_clone->name() == "TEST");
    BOOST_CHECK(p_clone->isValid("SH", Datetime(200001010000)) == false);
    BOOST_CHECK(p_clone->isValid("SZ", Datetime(200001010000)) == true);
    BOOST_CHECK(p_clone->getParam<int>("n") == 20);
    EnvironmentTest *p_tmp = (EnvironmentTest *)p_clone.get();
    BOOST_CHECK(p_tmp->getKType() == KQuery::MIN);
    p_clone->reset();
    BOOST_CHECK(p_clone->isValid("SH", Datetime(200001010000)) == true);
    BOOST_CHECK(p_clone->isValid("SZ", Datetime(200001010000)) == false);
}

#endif //if 0

/** @} */
