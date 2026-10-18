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

class EnvironmentTestOutOfOrder : public EnvironmentBase {
public:
    EnvironmentTestOutOfOrder() : EnvironmentBase("TEST") {}

    virtual ~EnvironmentTestOutOfOrder() {}

    // Append in descending date order: valuesRef order must not be assumed date-ascending
    virtual void _calculate() {
        _addValid(Datetime(20010103), -2.0);
        _addValid(Datetime(20010101), 5.0);
        _addValid(Datetime(20010102), 3.0);
    }

    virtual EnvironmentPtr _clone() {
        return make_shared<EnvironmentTestOutOfOrder>();
    }
};

class EnvironmentTestOutOfOrder2 : public EnvironmentBase {
public:
    EnvironmentTestOutOfOrder2() : EnvironmentBase("TEST") {}

    virtual ~EnvironmentTestOutOfOrder2() {}

    virtual void _calculate() {
        _addValid(Datetime(20010103), 2.0);
        _addValid(Datetime(20010101), 1.0);
    }

    virtual EnvironmentPtr _clone() {
        return make_shared<EnvironmentTestOutOfOrder2>();
    }
};

static void check_expect(const EVPtr& ev, const std::vector<std::pair<Datetime, price_t>>& expect) {
    for (auto& p : expect) {
        CHECK_EQ(ev->getValue(p.first), p.second);
    }
}

/** @par Test points */
TEST_CASE("test_Environment_add") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));

    /** @arg Both ev are empty */
    EVPtr ret = ev1 + ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_Add");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg One of the ev is empty */
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

    /** @arg Neither ev is empty */
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

/** @par Test points */
TEST_CASE("test_Environment_sub") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));

    /** @arg Both ev are empty */
    EVPtr ret = ev1 - ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_Sub");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg One of the ev is empty */
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

    /** @arg Neither ev is empty */
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

/** @par Test points */
TEST_CASE("test_Environment_multi") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));
    std::vector<std::pair<Datetime, price_t>> expect{};

    /** @arg Both ev are empty */
    EVPtr ret = ev1 * ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_Multi");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg One of the ev is empty */
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

    /** @arg Neither ev is empty and both sides are valid on the common dates */
    ev1 = std::make_shared<EnvironmentTest1>();                  // d1:2, d2:3, d3:2 (all valid)
    EVPtr ev3 = std::make_shared<EnvironmentTestOutOfOrder2>();  // d1:1, d3:2 (all valid)
    REQUIRE((ev1 && ev3));
    ret->reset();
    ret = ev1 * ev3;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 2);

    expect = {
      {Datetime(20010101), 2.0},
      {Datetime(20010103), 4.0},
    };
    check_expect(ret, expect);
    CHECK(ret->isValid(Datetime(20010101)));
    CHECK(ret->isValid(Datetime(20010103)));

    /** @arg One side is valid and the other is invalid on the common date: a disagreement carries
     * no combined strength, so the day is left out and stays invalid */
    ev1 = std::make_shared<EnvironmentTest1>();  // d3: 2.0 valid
    ev2 = std::make_shared<EnvironmentTest2>();  // d3: -4.0 invalid
    REQUIRE((ev1 && ev2));
    ret->reset();
    ret = ev1 * ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);
    CHECK_FALSE(ret->isValid(Datetime(20010103)));

    /** @arg Both sides are invalid (negative) on the common date: the raw product would be
     * positive (+8.0), so the value is forced to -(abs(-2) * abs(-4)) = -8.0 and the day stays
     * invalid */
    EVPtr evNeg = std::make_shared<EnvironmentTestOutOfOrder>();  // d3: -2.0 invalid
    ev2 = std::make_shared<EnvironmentTest2>();                   // d3: -4.0 invalid
    REQUIRE((evNeg && ev2));
    ret->reset();
    ret = evNeg * ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 1);

    expect = {
      {Datetime(20010103), -8.0},
    };
    check_expect(ret, expect);
    CHECK_FALSE(ret->isValid(Datetime(20010103)));
}

/** @par Test points */
TEST_CASE("test_Environment_div") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));
    std::vector<std::pair<Datetime, price_t>> expect{};

    /** @arg Both ev are empty */
    EVPtr ret = ev1 / ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_Div");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg One of the ev is empty */
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

    /** @arg Neither ev is empty and both sides are valid on the common dates */
    ev1 = std::make_shared<EnvironmentTest1>();                  // d1:2, d2:3, d3:2 (all valid)
    EVPtr ev3 = std::make_shared<EnvironmentTestOutOfOrder2>();  // d1:1, d3:2 (all valid)
    REQUIRE((ev1 && ev3));
    ret->reset();
    ret = ev1 / ev3;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 2);

    expect = {
      {Datetime(20010101), 2.0},
      {Datetime(20010103), 1.0},
    };
    check_expect(ret, expect);
    CHECK(ret->isValid(Datetime(20010101)));
    CHECK(ret->isValid(Datetime(20010103)));

    /** @arg One side is valid and the other is invalid on the common date: a disagreement carries
     * no combined strength, so the day is left out and stays invalid */
    ev1 = std::make_shared<EnvironmentTest1>();  // d3: 2.0 valid
    ev2 = std::make_shared<EnvironmentTest2>();  // d3: -4.0 invalid
    REQUIRE((ev1 && ev2));
    ret->reset();
    ret = ev1 / ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);
    CHECK_FALSE(ret->isValid(Datetime(20010103)));

    /** @arg Both sides are invalid (negative) on the common date: the raw ratio would be positive
     * (+0.5), so the value is forced to -(abs(-2) / abs(-4)) = -0.5 and the day stays invalid */
    EVPtr evNeg = std::make_shared<EnvironmentTestOutOfOrder>();  // d3: -2.0 invalid
    ev2 = std::make_shared<EnvironmentTest2>();                   // d3: -4.0 invalid
    REQUIRE((evNeg && ev2));
    ret->reset();
    ret = evNeg / ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 1);

    expect = {
      {Datetime(20010103), -0.5},
    };
    check_expect(ret, expect);
    CHECK_FALSE(ret->isValid(Datetime(20010103)));
}

/** @par Test points */
TEST_CASE("test_Environment_and") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));
    std::vector<std::pair<Datetime, price_t>> expect{};

    /** @arg Both ev are empty */
    EVPtr ret = ev1 & ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_And");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg One of the ev is empty */
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

    /** @arg Neither ev is empty */
    ev1 = std::make_shared<EnvironmentTest1>();
    ev2 = std::make_shared<EnvironmentTest2>();
    REQUIRE((ev1 & ev2));
    ret->reset();
    ret = ev1 & ev2;
    ret->setQuery(query);
    ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);
}

/** @par Test points */
TEST_CASE("test_Environment_or") {
    EVPtr ev1, ev2;
    KQuery query(Datetime(20010101), Datetime(20010105));

    /** @arg Both ev are empty */
    EVPtr ret = ev1 | ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->name(), "EV_Or");
    auto ind = ret->getValues();
    CHECK_EQ(ind.size(), 0);

    /** @arg One of the ev is empty */
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

    /** @arg Neither ev is empty */
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

/** @par Test points */
TEST_CASE("test_Environment_nested") {
    /** @arg Nested combination reuses the reference-based path on child combinators */
    EVPtr ev1 = std::make_shared<EnvironmentTest1>();
    EVPtr ev2 = std::make_shared<EnvironmentTest2>();
    KQuery query(Datetime(20010101), Datetime(20010105));

    // inner = ev1 + ev2  => {01:2, 02:3, 03:-2, 04:-5}
    // ret = inner & ev1  => intersection where both >0 => {01, 02} (03 has inner=-2)
    EVPtr inner = ev1 + ev2;
    EVPtr ret = inner & ev1;
    ret->setQuery(query);
    CHECK_EQ(ret->getValues().size(), 2);
    CHECK(ret->isValid(Datetime(20010101)));
    CHECK(ret->isValid(Datetime(20010102)));
    CHECK_FALSE(ret->isValid(Datetime(20010103)));
    CHECK_FALSE(ret->isValid(Datetime(20010104)));

    // ret2 = ev1 | ev2 => union where either side >0 => {01, 02, 03} (ev2 values are negative)
    EVPtr ret2 = ev1 | ev2;
    ret2->setQuery(query);
    CHECK_EQ(ret2->getValues().size(), 3);
}

/** @par Test points */
TEST_CASE("test_Environment_out_of_order") {
    KQuery query(Datetime(20010101), Datetime(20010105));
    EVPtr ev1 = std::make_shared<EnvironmentTestOutOfOrder>();
    EVPtr ev2 = std::make_shared<EnvironmentTest2>();

    /** @arg Add with a child appending dates in non-ascending order */
    EVPtr ret = ev1 + ev2;
    ret->setQuery(query);
    std::vector<std::pair<Datetime, price_t>> expect{{Datetime(20010101), 5.0},
                                                     {Datetime(20010102), 3.0},
                                                     {Datetime(20010103), -6.0},
                                                     {Datetime(20010104), -5.0}};
    check_expect(ret, expect);

    /** @arg Sub with a child appending dates in non-ascending order */
    ret = ev1 - ev2;
    ret->setQuery(query);
    expect = {{Datetime(20010101), 5.0},
              {Datetime(20010102), 3.0},
              {Datetime(20010103), 2.0},
              {Datetime(20010104), 5.0}};
    check_expect(ret, expect);

    /** @arg Or with children appending dates in non-ascending order */
    EVPtr ev3 = std::make_shared<EnvironmentTestOutOfOrder2>();
    EVPtr ret2 = ev1 | ev3;
    ret2->setQuery(query);
    CHECK_EQ(ret2->getValues().size(), 3);
    CHECK(ret2->isValid(Datetime(20010103)));
}

/** @par Test points */
TEST_CASE("test_Environment_or_valid_only") {
    KQuery query(Datetime(20010101), Datetime(20010105));

    /** @arg Or emits only dates valid on at least one side (an invalid ev1 date is not kept as 0)
     */
    EVPtr ev1 = std::make_shared<EnvironmentTestOutOfOrder>();  // d1:5, d2:3 valid; d3:-2 invalid
    EVPtr ev2 = std::make_shared<EnvironmentTest2>();           // d3:-4, d4:-5 all invalid
    EVPtr ret = ev1 | ev2;
    ret->setQuery(query);
    CHECK_EQ(ret->getValues().size(), 2);
    CHECK(ret->isValid(Datetime(20010101)));
    CHECK(ret->isValid(Datetime(20010102)));
    CHECK_FALSE(ret->isValid(Datetime(20010103)));
}

/** @} */
