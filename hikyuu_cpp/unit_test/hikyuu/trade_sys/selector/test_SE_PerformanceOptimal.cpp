/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-15
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <algorithm>
#include <stdexcept>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Optimal.h>
#include <hikyuu/trade_sys/selector/imp/optimal/OptimalSelectorBase.h>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include "../system/create_test_sys.h"

using namespace hku;

/**
 * @defgroup test_Selector_optimal test_Selector_optimal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SE_PerformanceOptimal") {
    auto se = SE_PerformanceOptimal();
    CHECK_EQ(se->name(), "SE_PerformanceOptimal");

    /** @arg Try to add an empty system */
    CHECK_THROWS(se->addSystem(SYSPtr()));
    CHECK_UNARY(se->getProtoSystemList().empty());

    /** @arg Try to add a system list containing an empty system */
    auto sys = create_test_sys(2, 3);
    sys->setStock(getStock("sz000001"));
    se->addSystemList(SystemList{sys});
    CHECK_EQ(se->getProtoSystemList().size(), 1);
    CHECK_THROWS(se->addSystemList(SystemList{sys, SYSPtr()}));
    CHECK_EQ(se->getProtoSystemList().size(), 2);

    /** @arg Try to add a system without a given security */
    sys = create_test_sys(2, 3);
    // CHECK_THROWS(se->addSystem(sys));
    se->addSystem(sys);
    se->calculate(SystemList(), KQueryByIndex(-50));
    OptimalSelectorBase* raw_se = dynamic_cast<OptimalSelectorBase*>(se.get());
    CHECK_UNARY(raw_se->getRunRanges().empty());

    /** @arg Try to add a system list without a given security */
    sys = create_test_sys(2, 3);
    se->addSystemList({sys});
    se->calculate(SystemList(), KQueryByIndex(-50));
    CHECK_UNARY(raw_se->getRunRanges().empty());

    /** @arg The candidate system list length is 0 */
    se->removeAll();
    REQUIRE(se->getProtoSystemList().empty());
    se->calculate(SystemList(), KQueryByIndex(-50));

    /** @arg There is a single candidate system */
    Stock stk = getStock("sz000001");
    sys = create_test_sys(2, 3);
    sys->setStock(stk);
    se->removeAll();
    se->addSystem(sys);
    REQUIRE(se->getProtoSystemList().size() == 1);
    KQuery query(-50);
    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 25);
    se->calculate(SystemList(), query);
    auto run_ranges = raw_se->getRunRanges();
    CHECK_EQ(run_ranges.size(), 1);
    auto dates = StockManager::instance().getTradingCalendar(query);
    CHECK_EQ(run_ranges[0].start, dates[0]);
    CHECK_EQ(run_ranges[0].run_start, dates[30]);
    CHECK_EQ(run_ranges[0].end, dates[49] + Minutes(1));
    for (size_t i = 0; i < 30; i++) {
        CHECK_UNARY(se->getSelected(dates[i]).empty());
    }
    for (size_t i = 30; i < 50; i++) {
        auto sw = se->getSelected(dates[i]);
        CHECK_EQ(sw.size(), 1);
        CHECK_EQ(sw[0].sys->name(), sys->name());
    }

    query = KQueryByIndex(-60);
    se->setParam<int>("train_len", 20);
    se->setParam<int>("test_len", 10);
    se->reset();
    se->calculate(SystemList(), query);
    run_ranges = raw_se->getRunRanges();
    CHECK_EQ(run_ranges.size(), 4);
    dates = StockManager::instance().getTradingCalendar(query);
    CHECK_EQ(run_ranges[0].start, dates[0]);
    CHECK_EQ(run_ranges[0].run_start, dates[20]);
    CHECK_EQ(run_ranges[0].end, dates[30]);
    CHECK_EQ(run_ranges[1].start, dates[10]);
    CHECK_EQ(run_ranges[1].run_start, dates[30]);
    CHECK_EQ(run_ranges[1].end, dates[40]);
    CHECK_EQ(run_ranges[2].start, dates[20]);
    CHECK_EQ(run_ranges[2].run_start, dates[40]);
    CHECK_EQ(run_ranges[2].end, dates[50]);
    CHECK_EQ(run_ranges[3].start, dates[30]);
    CHECK_EQ(run_ranges[3].run_start, dates[50]);
    CHECK_EQ(run_ranges[3].end, dates[59] + Minutes(1));
    for (size_t i = 0; i < 20; i++) {
        CHECK_UNARY(se->getSelected(dates[i]).empty());
    }
    for (size_t i = 20; i < 60; i++) {
        auto sw = se->getSelected(dates[i]);
        CHECK_EQ(sw.size(), 1);
        CHECK_EQ(sw[0].sys->name(), sys->name());
    }

    /** @arg Multiple candidate systems, take the maximum */
    se->removeAll();
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        sys = create_test_sys(param.first, param.second);
        sys->setStock(stk);
        // sys->setParam("trace", true);
        se->addSystem(sys);
    }
    REQUIRE(se->getProtoSystemList().size() == params.size());

    query = KQueryByIndex(-125);
    // se->setParam<bool>("trace", true);
    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 20);
    se->calculate(SystemList(), query);
    run_ranges = raw_se->getRunRanges();
    CHECK_EQ(run_ranges.size(), 5);
    dates = StockManager::instance().getTradingCalendar(query);
    CHECK_EQ(run_ranges[0].start, dates[0]);
    CHECK_EQ(run_ranges[0].run_start, dates[30]);
    CHECK_EQ(run_ranges[0].end, dates[50]);
    CHECK_EQ(run_ranges[1].start, dates[20]);
    CHECK_EQ(run_ranges[1].run_start, dates[50]);
    CHECK_EQ(run_ranges[1].end, dates[70]);
    CHECK_EQ(run_ranges[2].start, dates[40]);
    CHECK_EQ(run_ranges[2].run_start, dates[70]);
    CHECK_EQ(run_ranges[2].end, dates[90]);
    CHECK_EQ(run_ranges[3].start, dates[60]);
    CHECK_EQ(run_ranges[3].run_start, dates[90]);
    CHECK_EQ(run_ranges[3].end, dates[110]);
    CHECK_EQ(run_ranges[4].start, dates[80]);
    CHECK_EQ(run_ranges[4].run_start, dates[110]);
    CHECK_EQ(run_ranges[4].end, dates[124] + Minutes(1));
    for (size_t i = 0; i < 30; i++) {
        CHECK_UNARY(se->getSelected(dates[i]).empty());
    }
    for (size_t i = 30; i < 50; i++) {
        auto sw = se->getSelected(dates[i]);
        CHECK_EQ(sw.size(), 1);
        CHECK_EQ(sw[0].sys->name(), "test_sys_3_10");
    }
    for (size_t i = 50; i < 90; i++) {
        auto sw = se->getSelected(dates[i]);
        CHECK_EQ(sw.size(), 1);
        CHECK_EQ(sw[0].sys->name(), "test_sys_3_5");
    }

    /** @arg Multiple candidate systems, take the minimum */
    // se->setParam<bool>("trace", true);
    se->setParam<int>("mode", 1);
    se->reset();
    se->calculate(SystemList(), query);
    run_ranges = raw_se->getRunRanges();
    CHECK_EQ(run_ranges.size(), 5);
    dates = StockManager::instance().getTradingCalendar(query);
    CHECK_EQ(run_ranges[0].start, dates[0]);
    CHECK_EQ(run_ranges[0].run_start, dates[30]);
    CHECK_EQ(run_ranges[0].end, dates[50]);
    CHECK_EQ(run_ranges[1].start, dates[20]);
    CHECK_EQ(run_ranges[1].run_start, dates[50]);
    CHECK_EQ(run_ranges[1].end, dates[70]);
    CHECK_EQ(run_ranges[2].start, dates[40]);
    CHECK_EQ(run_ranges[2].run_start, dates[70]);
    CHECK_EQ(run_ranges[2].end, dates[90]);
    CHECK_EQ(run_ranges[3].start, dates[60]);
    CHECK_EQ(run_ranges[3].run_start, dates[90]);
    CHECK_EQ(run_ranges[3].end, dates[110]);
    CHECK_EQ(run_ranges[4].start, dates[80]);
    CHECK_EQ(run_ranges[4].run_start, dates[110]);
    CHECK_EQ(run_ranges[4].end, dates[124] + Minutes(1));
    for (size_t i = 0; i < 30; i++) {
        CHECK_UNARY(se->getSelected(dates[i]).empty());
    }
    for (size_t i = 30; i < 50; i++) {
        auto sw = se->getSelected(dates[i]);
        CHECK_EQ(sw.size(), 1);
        CHECK_EQ(sw[0].sys->name(), "test_sys_5_20");
    }
    for (size_t i = 50; i < 70; i++) {
        auto sw = se->getSelected(dates[i]);
        CHECK_EQ(sw.size(), 1);
        CHECK_EQ(sw[0].sys->name(), "test_sys_5_10");
    }
    for (size_t i = 70; i < 110; i++) {
        auto sw = se->getSelected(dates[i]);
        CHECK_EQ(sw.size(), 1);
        CHECK_EQ(sw[0].sys->name(), "test_sys_5_10");
    }
    for (size_t i = 110; i < 125; i++) {
        auto sw = se->getSelected(dates[i]);
        CHECK_EQ(sw.size(), 1);
        CHECK_EQ(sw[0].sys->name(), "test_sys_5_20");
    }
}

/** @par Test points */
TEST_CASE("test_SE_PerformanceOptimal_calculate_deterministic") {
    auto se = SE_PerformanceOptimal();
    Stock stk = getStock("sz000001");
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        auto sys = create_test_sys(param.first, param.second);
        sys->setStock(stk);
        se->addSystem(sys);
    }
    KQuery query(-125);
    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 20);
    auto dates = StockManager::instance().getTradingCalendar(query);

    se->calculate(SystemList(), query);
    vector<string> first_selected;
    first_selected.reserve(dates.size());
    for (const auto& date : dates) {
        auto sw = se->getSelected(date);
        first_selected.push_back(sw.empty() ? string() : sw[0].sys->name());
    }

    /** @arg repeated full recalculations with the same input select the same system for each date,
     *  i.e. the concurrent walk-forward training is not cross-clobbered by the shared EV
     *  (ISS-084) */
    for (size_t round = 0; round < 2; round++) {
        se->reset();
        se->calculate(SystemList(), query);
        for (size_t i = 0; i < dates.size(); i++) {
            auto sw = se->getSelected(dates[i]);
            if (first_selected[i].empty()) {
                CHECK_UNARY(sw.empty());
            } else {
                REQUIRE_UNARY(!sw.empty());
                CHECK_EQ(sw[0].sys->name(), first_selected[i]);
            }
        }
    }
}

// A candidate signal whose calculation always throws, used to verify that a single failing
// candidate does not abort the whole parallel training
class ThrowingSignal : public SignalBase {
public:
    ThrowingSignal() : SignalBase("SE_ThrowingSignal") {}
    virtual ~ThrowingSignal() = default;

    virtual void _calculate(const KData&) override {
        throw std::runtime_error("signal calculate failed");
    }

    virtual SignalPtr _clone() override {
        return SignalPtr(new ThrowingSignal);
    }
};

/** @par Test points */
TEST_CASE("test_SE_PerformanceOptimal_candidate_throw") {
    auto se = SE_PerformanceOptimal();
    Stock stk = getStock("sz000001");

    auto good1 = create_test_sys(3, 5);
    good1->setStock(stk);
    auto good2 = create_test_sys(5, 20);
    good2->setStock(stk);
    auto bad = create_test_sys(3, 10);
    bad->setStock(stk);
    bad->setSG(SignalPtr(new ThrowingSignal));

    se->addSystemList({good1, bad, good2});

    KQuery query(-125);
    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 20);

    /** @arg A candidate throwing during the training run must not abort the whole calculation */
    CHECK_NOTHROW(se->calculate(SystemList(), query));

    /** @arg The healthy candidates are still evaluated and selected for the test windows */
    auto dates = StockManager::instance().getTradingCalendar(query);
    size_t selected_count = 0;
    for (size_t i = 30; i < dates.size(); i++) {
        auto sw = se->getSelected(dates[i]);
        if (!sw.empty()) {
            selected_count++;
            CHECK_UNARY(sw[0].sys->name() == good1->name() || sw[0].sys->name() == good2->name());
        }
    }
    CHECK_UNARY(selected_count > 0);
}

// A minimal environment that only records its query, used to check that the walk-forward training
// windows of the candidates do not overwrite the environment shared by the prototypes
class QuerySpyEnvironment : public EnvironmentBase {
public:
    QuerySpyEnvironment() : EnvironmentBase("SE_QuerySpyEnvironment") {}
    virtual ~QuerySpyEnvironment() = default;

    virtual void _calculate() override {}

    virtual EnvironmentPtr _clone() override {
        return EnvironmentPtr(new QuerySpyEnvironment);
    }
};

/** @par Test points */
TEST_CASE("test_SE_PerformanceOptimal_shared_ev") {
    auto se = SE_PerformanceOptimal();
    Stock stk = getStock("sz000001");
    auto ev = EnvironmentPtr(new QuerySpyEnvironment);
    KQuery origin = KQueryByIndex(-5);
    ev->setQuery(origin);

    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        auto sys = create_test_sys(param.first, param.second);
        sys->setStock(stk);
        sys->setEV(ev);
        se->addSystem(sys);
    }

    KQuery query(-125);
    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 20);

    /** @arg the per-window training queries of the candidates must not be written into the EV
     *  shared by the prototypes (SE-003) */
    CHECK_NOTHROW(se->calculate(SystemList(), query));
    CHECK_UNARY(ev->getQuery() == origin);
}

/** @par Test points */
TEST_CASE("test_SE_PerformanceOptimal_invalid_key") {
    auto se = SE_PerformanceOptimal("Unknown Performance Key");
    Stock stk = getStock("sz000001");
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        auto sys = create_test_sys(param.first, param.second);
        sys->setStock(stk);
        se->addSystem(sys);
    }

    KQuery query(-125);
    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 20);

    /** @arg an unknown statistic key yields Null (NaN) for every candidate, so none is selected
     *  and the calculation still completes without crashing (SE-N02) */
    CHECK_NOTHROW(se->calculate(SystemList(), query));
    auto dates = StockManager::instance().getTradingCalendar(query);
    for (const auto& date : dates) {
        CHECK_UNARY(se->getSelected(date).empty());
    }
}

/** @par Test points
 *  @note A behavior lock for the m_sys_dict unification (the derived member was folded into the
 *  base). The pre-fix shadowing was self-consistent (the derived _getSelected read the derived
 *  map), so no observable behavior difference exists and no discriminating black-box test can be
 *  constructed; this test only pins the unified behavior against regressions.
 */
TEST_CASE("test_SE_PerformanceOptimal_index_single") {
    auto se = SE_PerformanceOptimal();
    Stock stk = getStock("sz000001");
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        auto sys = create_test_sys(param.first, param.second);
        sys->setStock(stk);
        se->addSystem(sys);
    }

    KQuery query(-125);
    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 20);
    se->calculate(SystemList(), query);
    auto dates = StockManager::instance().getTradingCalendar(query);
    auto base = se->getSelected(dates[40]);
    REQUIRE_UNARY(base.size() == 1);

    /** @arg Each date stores a single selected system, so any index takes that same system */
    se->setParam<int>("index", 100);
    auto after = se->getSelected(dates[40]);
    CHECK_EQ(after.size(), 1);
    CHECK_EQ(after[0].sys->name(), base[0].sys->name());
    CHECK_EQ(after[0].weight, base[0].weight);
}

/** @par Test points */
TEST_CASE("test_SE_PerformanceOptimal_real_list_identity") {
    auto se = SE_PerformanceOptimal();
    Stock stk = getStock("sz000001");
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        auto sys = create_test_sys(param.first, param.second);
        sys->setStock(stk);
        se->addSystem(sys);
    }

    // The systems actually run by a portfolio, in the same order as the prototype list
    SystemList real_list;
    for (const auto& proto : se->getProtoSystemList()) {
        auto sys = proto->clone();
        sys->setStock(stk);
        real_list.emplace_back(sys);
    }

    KQuery query(-125);
    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 20);
    se->calculate(real_list, query);
    auto dates = StockManager::instance().getTradingCalendar(query);

    /** @arg the selection is the running instance, not the training copy */
    size_t selected_count = 0;
    for (size_t i = 30; i < dates.size(); i++) {
        auto sw = se->getSelected(dates[i]);
        if (sw.empty()) {
            continue;
        }
        selected_count++;
        CHECK_UNARY(std::find(real_list.begin(), real_list.end(), sw[0].sys) != real_list.end());
    }
    CHECK_GT(selected_count, 0);

    /** @arg without a real list the window-local copy is returned */
    se->reset();
    se->calculate(SystemList(), query);
    auto sw = se->getSelected(dates.back());
    REQUIRE_UNARY(sw.size() == 1);
    CHECK_UNARY(std::find(real_list.begin(), real_list.end(), sw[0].sys) == real_list.end());
}

/** @par Test points */
TEST_CASE("test_SE_PerformanceOptimal_bound_identity") {
    auto se = SE_PerformanceOptimal();
    Stock stk = getStock("sz000001");
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        auto sys = create_test_sys(param.first, param.second);
        sys->setStock(stk);
        se->addSystem(sys);
    }

    // The running systems are built in the reverse order of the prototypes and bound by identity
    SystemList real_list;
    auto protos = se->getProtoSystemList();
    for (size_t i = 0; i < protos.size(); i++) {
        auto sys = protos[protos.size() - 1 - i]->clone();
        sys->setStock(stk);
        sys->name("real_" + protos[protos.size() - 1 - i]->name());
        real_list.emplace_back(sys);
    }
    for (size_t i = 0; i < protos.size(); i++) {
        se->bindRealToProto(real_list[protos.size() - 1 - i], protos[i]);
    }

    KQuery query(-125);
    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 20);
    se->calculate(real_list, query);
    auto dates = StockManager::instance().getTradingCalendar(query);

    /** @arg the identity binding wins over the positional order: the first window picks the bound
     *  running system of the winning prototype (test_sys_3_10), not the one sitting at its index */
    auto sw = se->getSelected(dates[40]);
    REQUIRE_UNARY(sw.size() == 1);
    CHECK_EQ(sw[0].sys->name(), "real_test_sys_3_10");
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SE_PerformanceOptimal_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SE_PerformanceOptimal.xml";

    auto se1 = SE_PerformanceOptimal();
    Stock stk = getStock("sz000001");
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        auto sys = create_test_sys(param.first, param.second);
        sys->setStock(stk);
        // sys->setParam("trace", true);
        se1->addSystem(sys);
    }
    KQuery query(-125);
    se1->calculate(SystemList(), query);
    OptimalSelectorBase* raw_se1 = dynamic_cast<OptimalSelectorBase*>(se1.get());
    auto run_ranges1 = raw_se1->getRunRanges();

    // Currently a reset is needed after the calculation to serialize and reload normally
    se1->reset();

    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(se1);
    }

    SEPtr se2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(se2);
    }

    CHECK_EQ(se1->name(), se2->name());
    CHECK_EQ(se1->getProtoSystemList().size(), se2->getProtoSystemList().size());

    // se2->setParam<bool>("trace", true);
    se2->calculate(SystemList(), query);
    OptimalSelectorBase* raw_se2 = dynamic_cast<OptimalSelectorBase*>(se2.get());
    auto run_ranges2 = raw_se2->getRunRanges();
    CHECK_EQ(run_ranges1.size(), run_ranges2.size());
    for (size_t i = 0, len = run_ranges1.size(); i < len; i++) {
        CHECK_EQ(run_ranges1[i].start, run_ranges2[i].start);
        CHECK_EQ(run_ranges1[i].run_start, run_ranges2[i].run_start);
        CHECK_EQ(run_ranges1[i].end, run_ranges2[i].end);
    }

    auto dates = StockManager::instance().getTradingCalendar(query);
    se1->calculate(SystemList(), query);
    for (const auto& date : dates) {
        if (se1->getSelected(date).empty()) {
            CHECK_UNARY(se2->getSelected(date).empty());
        } else {
            CHECK_EQ(se1->getSelected(date)[0].sys->name(), se2->getSelected(date)[0].sys->name());
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @par Test points */
TEST_CASE("test_SE_PerformanceOptimal_recalculate") {
    auto se = SE_PerformanceOptimal();
    Stock stk = getStock("sz000001");
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        auto sys = create_test_sys(param.first, param.second);
        sys->setStock(stk);
        se->addSystem(sys);
    }

    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 20);
    KQuery query(-125);
    se->calculate(SystemList(), query);
    OptimalSelectorBase* raw_se = dynamic_cast<OptimalSelectorBase*>(se.get());
    REQUIRE_UNARY(raw_se);
    size_t ranges = raw_se->getRunRanges().size();
    REQUIRE_UNARY(ranges > 0);

    /** @arg A second calculate after an invalidation must not accumulate the run ranges */
    se->setParam<int>("test_len", 20);
    se->calculate(SystemList(), query);
    CHECK_EQ(raw_se->getRunRanges().size(), ranges);
}

/** @} */