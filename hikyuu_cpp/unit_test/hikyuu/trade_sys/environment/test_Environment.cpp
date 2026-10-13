/*
 * test_Environment.cpp
 *
 *  Created on: 2013-3-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <limits>
#include <stdexcept>

#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>

using namespace hku;

class EnvironmentTest : public EnvironmentBase {
public:
    EnvironmentTest() : EnvironmentBase("TEST") {
        setParam<int>("n", 10);
    }

    virtual ~EnvironmentTest() {}

    virtual void _calculate() {
        _addValid(Datetime(200001010000LL));
    }

    virtual EnvironmentPtr _clone() {
        return make_shared<EnvironmentTest>();
    }

    void setPythonObject(bool v) {
        m_is_python_object = v;
    }
};

/**
 * @defgroup test_Environment test_Environment
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Environment") {
    /** @arg The basic operation */
    EnvironmentPtr p = make_shared<EnvironmentTest>();
    CHECK_EQ(p->name(), "TEST");
    CHECK_EQ(p->isValid(Datetime(200001010000)), false);
    CHECK_EQ(p->isValid(Datetime(200001020000)), false);

    p->setQuery(KQuery(-100));
    CHECK_EQ(p->isValid(Datetime(200001010000)), true);
    CHECK_EQ(p->isValid(Datetime(200001020000)), false);
    CHECK_EQ(p->getParam<int>("n"), 10);

    /** @arg The clone operation */
    p->setParam<int>("n", 20);
    EnvironmentPtr p_clone = p->clone();
    CHECK_NE(p, p_clone);
    CHECK_EQ(p_clone->name(), "TEST");
    CHECK_EQ(p_clone->isValid(Datetime(200001010000)), true);
    CHECK_EQ(p_clone->isValid(Datetime(200001020000)), false);
    CHECK_EQ(p_clone->getParam<int>("n"), 20);
}

/** @par Test points */
TEST_CASE("test_Environment_copy_ctor_keeps_python_flag") {
    /** @arg The copy constructor copies m_is_python_object (EV-007) */
    EnvironmentTest base;
    base.setPythonObject(true);
    EnvironmentTest copy(base);
    CHECK(copy.isPythonObject());

    EnvironmentTest base2;
    base2.setPythonObject(false);
    EnvironmentTest copy2(base2);
    CHECK_FALSE(copy2.isPythonObject());
}

class EnvironmentInfTest : public EnvironmentBase {
public:
    EnvironmentInfTest() : EnvironmentBase("TEST_INF") {}
    void _calculate() override {
        _addValid(Datetime(200001010000LL), std::numeric_limits<price_t>::infinity());
        _addValid(Datetime(200001020000LL), std::nan(""));
        _addValid(Datetime(200001030000LL), 1.0);
    }
    EnvironmentPtr _clone() override {
        return make_shared<EnvironmentInfTest>();
    }
};

/** @par Test points */
TEST_CASE("test_Environment_reject_non_finite_value") {
    /** @arg _addValid rejects +inf / NaN; isValid requires isfinite (EV-009) */
    auto p = make_shared<EnvironmentInfTest>();
    p->setQuery(KQuery(-100));
    CHECK_FALSE(p->isValid(Datetime(200001010000LL)));
    CHECK_FALSE(p->isValid(Datetime(200001020000LL)));
    CHECK(p->isValid(Datetime(200001030000LL)));
}

class EnvironmentCalcFailTest : public EnvironmentBase {
public:
    EnvironmentCalcFailTest() : EnvironmentBase("TEST_FAIL") {}
    bool fail{true};
    void _calculate() override {
        if (fail) {
            throw std::runtime_error("calc failed");
        }
        _addValid(Datetime(200001010000LL));
    }
    EnvironmentPtr _clone() override {
        return make_shared<EnvironmentCalcFailTest>();
    }
};

/** @par Test points */
TEST_CASE("test_Environment_setQuery_retry_after_failure") {
    /** @arg A failed _calculate leaves m_query unchanged so the same query is retryable (EV-012) */
    auto p = make_shared<EnvironmentCalcFailTest>();
    KQuery q(-100);
    CHECK_THROWS(p->setQuery(q));
    p->fail = false;
    CHECK_NOTHROW(p->setQuery(q));
    CHECK(p->isValid(Datetime(200001010000LL)));
}

/** @} */
