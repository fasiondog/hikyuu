/*
 * test_Environment.cpp
 *
 *  Created on: 2013-3-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <limits>
#include <mutex>
#include <atomic>
#include <thread>
#include <stdexcept>

#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include <hikyuu/trade_sys/environment/build_in.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

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
    /** @arg The copy constructor copies m_is_python_object */
    EnvironmentTest base;
    base.setPythonObject(true);
    EnvironmentTest copy(base);
    CHECK(copy.isPythonObject());

    EnvironmentTest base2;
    base2.setPythonObject(false);
    EnvironmentTest copy2(base2);
    CHECK_FALSE(copy2.isPythonObject());
}

class EnvironmentCloneLockTest : public EnvironmentBase {
public:
    EnvironmentCloneLockTest() : EnvironmentBase("TEST_CLONE_LOCK") {}

    void _calculate() override {}

    EnvironmentPtr _clone() override {
        // clone() must invoke this hook with the mutex free: an exclusive try_lock fails while a
        // shared lock is held, which is what the hook would hit without the lock-order fix
        std::unique_lock<std::shared_mutex> lock(m_mutex, std::try_to_lock);
        auto p = make_shared<EnvironmentCloneLockTest>();
        p->m_hook_unlocked = lock.owns_lock();
        return p;
    }

    bool m_hook_unlocked{false};
};

/** @par Test points */
TEST_CASE("test_Environment_clone_invokes_clone_hook_without_lock") {
    /** @arg The _clone hook must not be invoked while the mutex is held (GIL lock-order safety) */
    auto p = make_shared<EnvironmentCloneLockTest>();
    auto p_clone = std::dynamic_pointer_cast<EnvironmentCloneLockTest>(p->clone());
    REQUIRE(p_clone);
    CHECK(p_clone->m_hook_unlocked);
}

class EnvironmentInfTest : public EnvironmentBase {
public:
    EnvironmentInfTest() : EnvironmentBase("TEST_INF") {}
    void _calculate() override {
        _addValid(Datetime(200001010000LL), std::numeric_limits<price_t>::infinity());
        _addValid(Datetime(200001020000LL), std::nan(""));
        _addValid(Datetime(200001030000LL), 1.0);
        // Plant a value that overflowed to +inf inside the storage, which _addValid cannot produce
        m_date_index[Datetime(200001040000LL)] = m_values.size();
        m_values.push_back(std::numeric_limits<price_t>::infinity());
    }
    EnvironmentPtr _clone() override {
        return make_shared<EnvironmentInfTest>();
    }
};

/** @par Test points */
TEST_CASE("test_Environment_reject_non_finite_value") {
    /** @arg _addValid rejects +inf / NaN; only a finite value > 0 is valid */
    auto p = make_shared<EnvironmentInfTest>();
    p->setQuery(KQuery(-100));
    CHECK_FALSE(p->isValid(Datetime(200001010000LL)));
    CHECK_FALSE(p->isValid(Datetime(200001020000LL)));
    CHECK(p->isValid(Datetime(200001030000LL)));

    /** @arg A value that overflowed to +inf inside the storage is still invalid */
    CHECK_FALSE(p->isValid(Datetime(200001040000LL)));
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
    /** @arg A failed _calculate leaves m_query unchanged so the same query is retryable */
    auto p = make_shared<EnvironmentCalcFailTest>();
    KQuery q(-100);
    CHECK_THROWS(p->setQuery(q));
    p->fail = false;
    CHECK_NOTHROW(p->setQuery(q));
    CHECK(p->isValid(Datetime(200001010000LL)));
}

class EnvironmentPartialCalcTest : public EnvironmentBase {
public:
    EnvironmentPartialCalcTest() : EnvironmentBase("TEST_PARTIAL") {}
    void _calculate() override {
        _addValid(Datetime(200001010000LL));
        throw std::runtime_error("calc failed after add");
    }
    EnvironmentPtr _clone() override {
        return make_shared<EnvironmentPartialCalcTest>();
    }
};

/** @par Test points */
TEST_CASE("test_Environment_setQuery_cleans_partial_results_on_failure") {
    /** @arg A _calculate that fails after _addValid leaves no partial valid dates behind */
    auto p = make_shared<EnvironmentPartialCalcTest>();
    KQuery q(-100);
    CHECK_THROWS(p->setQuery(q));
    CHECK_FALSE(p->isValid(Datetime(200001010000LL)));
    CHECK_FALSE(p->isValid(Datetime(200001020000LL)));
}

/** @par Test points */
TEST_CASE("test_EV_Bool_reject_non_finite") {
    StockManager& sm = StockManager::instance();
    KQuery query(-100);
    KData kdata = sm.getStock("sh000001").getKData(query);
    REQUIRE(!kdata.empty());

    /** @arg A finite value greater than 0 marks every date as valid */
    EVPtr ok = EV_Bool(PRICELIST(kdata.size(), 1.0));
    ok->setQuery(query);
    CHECK(ok->isValid(kdata.front().datetime));
    CHECK(ok->isValid(kdata.back().datetime));

    /** @arg +inf satisfies the "> 0" test but must not mark any date as valid */
    EVPtr inf = EV_Bool(PRICELIST(kdata.size(), std::numeric_limits<price_t>::infinity()));
    inf->setQuery(query);
    CHECK_FALSE(inf->isValid(kdata.front().datetime));
    CHECK_FALSE(inf->isValid(kdata.back().datetime));

    /** @arg -inf and NaN stay invalid */
    EVPtr neg_inf = EV_Bool(PRICELIST(kdata.size(), -std::numeric_limits<price_t>::infinity()));
    neg_inf->setQuery(query);
    CHECK_FALSE(neg_inf->isValid(kdata.front().datetime));

    EVPtr nan = EV_Bool(PRICELIST(kdata.size(), std::nan("")));
    nan->setQuery(query);
    CHECK_FALSE(nan->isValid(kdata.front().datetime));
}

/** @par Test points */
TEST_CASE("test_EV_TwoLine_reject_size_mismatch") {
    StockManager& sm = StockManager::instance();
    KQuery query(-100);
    KData kdata = sm.getStock("sh000001").getKData(query);
    REQUIRE(kdata.size() > 10);

    /** @arg A short fast line is aligned to the KData dates by ALIGN; its 5 values carry the
     *       first 5 KData dates (inherited from its calculation context), so only those dates
     *       are valid */
    EVPtr mismatch = EV_TwoLine(PRICELIST(5, 1.0), PRICELIST(kdata.size(), 0.0));
    mismatch->setQuery(query);
    for (size_t i = 0; i < 5; ++i) {
        CHECK(mismatch->isValid(kdata[i].datetime));
    }
    for (size_t i = 5; i < kdata.size(); ++i) {
        CHECK_FALSE(mismatch->isValid(kdata[i].datetime));
    }

    /** @arg An empty fast line is aligned to an all-null series and marks no date as valid */
    EVPtr empty = EV_TwoLine(PRICELIST(), PRICELIST(kdata.size(), 0.0));
    CHECK_NOTHROW(empty->setQuery(query));
    CHECK_FALSE(empty->isValid(kdata.front().datetime));
    CHECK_FALSE(empty->isValid(kdata.back().datetime));

    /** @arg Matching lengths keep the normal fast > slow behaviour without any alignment */
    EVPtr ok = EV_TwoLine(PRICELIST(kdata.size(), 2.0), PRICELIST(kdata.size(), 1.0));
    ok->setQuery(query);
    CHECK(ok->isValid(kdata.front().datetime));
    CHECK(ok->isValid(kdata.back().datetime));
}

/** @par Test points */
TEST_CASE("test_Environment_shared_multiple_setQuery") {
    StockManager& sm = StockManager::instance();
    KQuery query(-100);
    KQuery query2(-90);
    KData kdata = sm.getStock("sh000001").getKData(query);
    KData kdata2 = sm.getStock("sh000001").getKData(query2);
    REQUIRE(!kdata.empty());
    REQUIRE(!kdata2.empty());

    /** @arg Repeated setQuery with the same query (what every shared system does on binding)
     *       keeps the state intact instead of wiping it */
    EVPtr ev = EV_Bool(PRICELIST(kdata.size(), 1.0));
    for (int i = 0; i < 3; ++i) {
        ev->setQuery(query);
        CHECK(ev->isValid(kdata.front().datetime));
        CHECK(ev->isValid(kdata.back().datetime));
    }
    CHECK_EQ(ev->getValues().size(), kdata.size());

    /** @arg Concurrent readers while the shared EV is re-bound with the same query stay
     *       race-free and the results remain consistent */
    std::atomic<bool> stop{false};
    std::vector<std::thread> readers;
    for (int t = 0; t < 4; ++t) {
        readers.emplace_back([&] {
            while (!stop.load(std::memory_order_relaxed)) {
                CHECK(ev->isValid(kdata.front().datetime));
            }
        });
    }
    for (int i = 0; i < 100; ++i) {
        ev->setQuery(query);
    }
    stop.store(true, std::memory_order_relaxed);
    for (auto& th : readers) {
        th.join();
    }
    CHECK(ev->isValid(kdata.front().datetime));

    /** @arg Alternating queries on the shared EV recalculate and stay consistent */
    for (int i = 0; i < 2; ++i) {
        ev->setQuery(query);
        CHECK_EQ(ev->getValues().size(), kdata.size());
        ev->setQuery(query2);
        CHECK_EQ(ev->getValues().size(), kdata2.size());
    }
}

/** @par Test points */
TEST_CASE("test_EV_invalid_market_check") {
    /** @arg The default market is valid */
    EVPtr ev_bool = EV_Bool(PRICELIST(10, 1.0));
    CHECK_EQ(ev_bool->getParam<string>("market"), "SH");
    EVPtr ev_two = EV_TwoLine(PRICELIST(10, 1.0), PRICELIST(10, 0.0));
    CHECK_EQ(ev_two->getParam<string>("market"), "SH");

    /** @arg An invalid market is rejected when the parameter is set, on both built-in EVs
     *       (setParam validates after the assignment, so the invalid value stays in place
     *       afterwards; only the throw is asserted here) */
    CHECK_THROWS(ev_bool->setParam<string>("market", "XX"));
    CHECK_THROWS(ev_two->setParam<string>("market", "XX"));
}

class EnvironmentCustomRuleTest : public EnvironmentBase {
public:
    EnvironmentCustomRuleTest() : EnvironmentBase("TEST_CUSTOM_RULE") {}

    void _calculate() override {
        _addValid(Datetime(200001010000LL), -1.0);
        _addValid(Datetime(200001020000LL), 1.0);
        _addValid(Datetime(200001030000LL), std::numeric_limits<price_t>::quiet_NaN());
    }

    EnvironmentPtr _clone() override {
        return make_shared<EnvironmentCustomRuleTest>();
    }

    // A custom rule: a negative value is valid, everything else is invalid
    bool _isValid(const Datetime& datetime, price_t value) const override {
        return value < 0.0;
    }
};

/** @par Test points */
TEST_CASE("test_Environment_custom_is_valid_rule") {
    /** @arg The subclass _isValid hook overrides the default "finite and > 0" rule */
    auto p = make_shared<EnvironmentCustomRuleTest>();
    p->setQuery(KQuery(-100));
    CHECK(p->isValid(Datetime(200001010000LL)));
    CHECK_FALSE(p->isValid(Datetime(200001020000LL)));
    CHECK_FALSE(p->isValid(Datetime(200001030000LL)));

    /** @arg A date that was never registered stays invalid regardless of the rule */
    CHECK_FALSE(p->isValid(Datetime(200001040000LL)));
}

/** @} */
