/*
 * test_Environment.cpp
 *
 *  Created on: 2013-3-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/environment/crt/EV_Logic.h>

using namespace hku;

/**
 * @defgroup test_Environment_logic test_Environment_logic
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

class EnvironmentTest1 : public EnvironmentBase {
public:
    EnvironmentTest1() : EnvironmentBase("TEST") {}

    virtual ~EnvironmentTest1() {}

    virtual void _calculate() {
        _addValid(Datetime(20010101), 2.0);
        _addValid(Datetime(20010102), 3.0);
        _addValid(Datetime(20010103), 2.0);
    }

    virtual EnvironmentPtr _clone() {
        return make_shared<EnvironmentTest1>();
    }
};

class EnvironmentTest2 : public EnvironmentBase {
public:
    EnvironmentTest2() : EnvironmentBase("TEST") {}

    virtual ~EnvironmentTest2() {}

    virtual void _calculate() {
        _addValid(Datetime(20010103), -4.0);
        _addValid(Datetime(20010104), -5.0);
    }

    virtual EnvironmentPtr _clone() {
        return make_shared<EnvironmentTest2>();
    }
};

static void check_expect(const EVPtr& ev, const std::vector<std::pair<Datetime, price_t>>& expect) {
    for (auto& p : expect) {
        CHECK_EQ(ev->getValue(p.first), p.second);
    }
}

/** @par 检测点 */
TEST_CASE("test_Environment_add") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));

    /** @arg 两个ev均为空 */
    EVPtr ret = ev1 + ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_Add");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg 其中一个 ev 为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    REQUIRE((ev1 && !ev2));
    ret->reset();
    ret = ev1 + ev2;
    ret->setQuery(query);

    std::vector<std::pair<Datetime, price_t>> expect{
      {Datetime(20010101), 2.0}, {Datetime(20010102), 3.0}, {Datetime(20010103), 2.0}};
    check_expect(ret, expect);

    ev1 = EVPtr();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((!ev1 && ev2));
    ret->reset();
    ret = ev1 + ev2;
    ret->setQuery(query);

    expect = {{Datetime(20010103), -4.0}, {Datetime(20010104), -5.0}};
    check_expect(ret, expect);

    /** @arg 两个ev均不为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((ev1 & ev2));
    ret->reset();
    ret = ev1 + ev2;
    ret->setQuery(query);

    expect = {{Datetime(20010101), 2.0},
              {Datetime(20010102), 3.0},
              {Datetime(20010103), -2.0},
              {Datetime(20010104), -5.0}};
    check_expect(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_Environment_sub") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));

    /** @arg 两个ev均为空 */
    EVPtr ret = ev1 - ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_Sub");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg 其中一个 ev 为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    REQUIRE((ev1 && !ev2));
    ret->reset();
    ret = ev1 - ev2;
    ret->setQuery(query);

    std::vector<std::pair<Datetime, price_t>> expect{
      {Datetime(20010101), 2.0}, {Datetime(20010102), 3.0}, {Datetime(20010103), 2.0}};
    check_expect(ret, expect);

    ev1 = EVPtr();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((!ev1 && ev2));
    ret->reset();
    ret = ev1 - ev2;
    ret->setQuery(query);

    expect = {{Datetime(20010103), 4.0}, {Datetime(20010104), 5.0}};
    check_expect(ret, expect);

    /** @arg 两个ev均不为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((ev1 & ev2));
    ret->reset();
    ret = ev1 - ev2;
    ret->setQuery(query);

    expect = {{Datetime(20010101), 2.0},
              {Datetime(20010102), 3.0},
              {Datetime(20010103), 6.0},
              {Datetime(20010104), 5.0}};
    check_expect(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_Environment_multi") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));
    std::vector<std::pair<Datetime, price_t>> expect{};

    /** @arg 两个ev均为空 */
    EVPtr ret = ev1 * ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_Multi");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg 其中一个 ev 为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    REQUIRE((ev1 && !ev2));
    ret->reset();
    ret = ev1 * ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    ev1 = EVPtr();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((!ev1 && ev2));
    ret->reset();
    ret = ev1 * ev2;
    ret->setQuery(query);

    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg 两个ev均不为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((ev1 & ev2));
    ret->reset();
    ret = ev1 * ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 1);

    expect = {
      {Datetime(20010103), -8.0},
    };
    check_expect(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_Environment_div") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));
    std::vector<std::pair<Datetime, price_t>> expect{};

    /** @arg 两个ev均为空 */
    EVPtr ret = ev1 / ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_Div");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg 其中一个 ev 为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    REQUIRE((ev1 && !ev2));
    ret->reset();
    ret = ev1 / ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    ev1 = EVPtr();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((!ev1 && ev2));
    ret->reset();
    ret = ev1 / ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg 两个ev均不为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((ev1 & ev2));
    ret->reset();
    ret = ev1 / ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 1);

    expect = {
      {Datetime(20010103), -0.5},
    };
    check_expect(ret, expect);
}

/** @par 检测点 */
TEST_CASE("test_Environment_and") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));
    std::vector<std::pair<Datetime, price_t>> expect{};

    /** @arg 两个ev均为空 */
    EVPtr ret = ev1 & ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_And");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg 其中一个 ev 为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    REQUIRE((ev1 && !ev2));
    ret->reset();
    ret = ev1 & ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    ev1 = EVPtr();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((!ev1 && ev2));
    ret->reset();
    ret = ev1 & ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg 两个ev均不为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((ev1 & ev2));
    ret->reset();
    ret = ev1 & ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);
}

/** @par 检测点 */
TEST_CASE("test_Environment_or") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));

    /** @arg 两个ev均为空 */
    EVPtr ret = ev1 | ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_Or");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg 其中一个 ev 为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    REQUIRE((ev1 && !ev2));
    ret->reset();
    ret = ev1 | ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 3);

    std::vector<std::pair<Datetime, price_t>> expect{
      {Datetime(20010101), 1.0}, {Datetime(20010102), 1.0}, {Datetime(20010103), 1.0}};
    check_expect(ret, expect);

    ev1 = EVPtr();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((!ev1 && ev2));
    ret->reset();
    ret = ev1 | ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg 两个ev均不为空 */
    ev1 = std::make_shared<EnvironmentTest1>();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((ev1 & ev2));
    ret->reset();
    ret = ev1 | ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 3);

    expect = {{Datetime(20010101), 1.0}, {Datetime(20010102), 1.0}};
    check_expect(ret, expect);
}

/** @} */
