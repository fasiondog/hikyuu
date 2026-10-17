/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2026-10-18
 *      Author: fasiondog
 */

#include "../test_config.h"

#include <algorithm>
#include <limits>
#include <set>
#include <hikyuu/Block.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/analysis/analysis_sys.h>
#include <hikyuu/analysis/combinate.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_sys/signal/crt/SG_Bool.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_analysis_sys test_hikyuu_analysis_sys
 * @ingroup test_hikyuu_analysis_suite
 * @{
 */

/** A system whose buy condition keeps firing, used as the prototype of the cases below */
static SystemPtr analysis_sys_test_system(const TradeManagerPtr& tm) {
    return SYS_Simple(tm, MM_Nothing(), EnvironmentPtr(), ConditionPtr(),
                      SG_Bool(CLOSE() > OPEN(), CLOSE() < 0));
}

static KQuery analysis_sys_test_query() {
    return KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL));
}

static size_t analysis_sys_test_key_index(const string& key) {
    Performance per;
    const StringList& names = per.names();
    auto iter = std::find(names.begin(), names.end(), key);
    return iter == names.end() ? Null<size_t>() : (size_t)(iter - names.begin());
}

/** @par Test points */
TEST_CASE("test_analysisSystemList_empty") {
    /** @arg an empty system list gives an empty result, for both overloads */
    CHECK_UNARY(analysisSystemList(SystemList(), StockList(), KQuery()).empty());
    CHECK_UNARY(analysisSystemList(SystemList(), Stock(), KQuery()).empty());
}

/** @par Test points */
TEST_CASE("test_analysisSystemList_size_mismatch") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    SystemList sys_list{analysis_sys_test_system(crtTM())};

    /** @arg a system list whose size differs from the security list is rejected */
    CHECK_THROWS_AS((analysisSystemList(sys_list, StockList{stk, stk}, analysis_sys_test_query())),
                    hku::exception);
}

/** @par Test points */
TEST_CASE("test_analysisSystemList_list_overload") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = analysis_sys_test_query();

    SystemList sys_list{analysis_sys_test_system(crtTM()), analysis_sys_test_system(crtTM())};
    StockList stk_list{stk, stk};
    auto result = analysisSystemList(sys_list, stk_list, query);

    /** @arg every system / security pair produces one record carrying the security identity */
    REQUIRE(result.size() == 2);
    Performance per;
    for (const auto& record : result) {
        CHECK_UNARY(record.market_code == stk.market_code());
        CHECK_UNARY(record.name == stk.name());
        /** @arg every record carries a complete set of the statistics values */
        CHECK_UNARY(record.values.size() == per.names().size());
    }
}

/** @par Test points */
TEST_CASE("test_analysisSystemList_stock_overload") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = analysis_sys_test_query();

    auto sys_a = analysis_sys_test_system(crtTM());
    sys_a->name("SYS_A");
    auto sys_b = analysis_sys_test_system(crtTM());
    sys_b->name("SYS_B");
    SystemList sys_list{sys_a, sys_b};
    auto result = analysisSystemList(sys_list, stk, query);

    /** @arg every system produces one record carrying its own name */
    REQUIRE(result.size() == 2);
    CHECK_UNARY(result[0].name == "SYS_A");
    CHECK_UNARY(result[1].name == "SYS_B");
    CHECK_UNARY(result[0].market_code == stk.market_code());
}

/** @par Test points */
TEST_CASE("test_analysisSystemList_null_sys_stock") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = analysis_sys_test_query();

    SystemList sys_list{SystemPtr(), analysis_sys_test_system(crtTM())};
    StockList stk_list{stk, Stock()};
    auto result = analysisSystemList(sys_list, stk_list, query);

    /** @arg a null system or a null security yields an empty record instead of an exception */
    REQUIRE(result.size() == 2);
    CHECK_UNARY(result[0].market_code.empty());
    CHECK_UNARY(result[0].values.empty());
    CHECK_UNARY(result[1].market_code.empty());
    CHECK_UNARY(result[1].values.empty());
}

/** @par Test points */
TEST_CASE("test_findOptimalSystem_sort_mode") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = analysis_sys_test_query();

    // Two accounts with different initial cash, so that the two sort modes choose differently
    SystemList sys_list{analysis_sys_test_system(crtTM(Datetime(199911100000LL), 100000.0)),
                        analysis_sys_test_system(crtTM(Datetime(199911100000LL), 200000.0))};

    auto refs = analysisSystemList(sys_list, stk, query);
    REQUIRE(refs.size() == 2);
    size_t idx = analysis_sys_test_key_index("Current Total Assets");
    REQUIRE(idx != Null<size_t>());
    double v0 = refs[0].values[idx];
    double v1 = refs[1].values[idx];
    REQUIRE(v0 != v1);

    /** @arg sort_mode 0 selects the system with the highest statistics value */
    auto max_r = findOptimalSystem(sys_list, stk, query, "Current Total Assets", 0);
    REQUIRE(max_r.second);
    CHECK_UNARY(max_r.first == doctest::Approx(std::max(v0, v1)).epsilon(0.0001));
    CHECK_UNARY(max_r.second.get() == (v0 > v1 ? sys_list[0] : sys_list[1]).get());

    /** @arg any other sort_mode selects the system with the lowest statistics value */
    auto min_r = findOptimalSystem(sys_list, stk, query, "Current Total Assets", 1);
    REQUIRE(min_r.second);
    CHECK_UNARY(min_r.first == doctest::Approx(std::min(v0, v1)).epsilon(0.0001));
    CHECK_UNARY(min_r.second.get() == (v0 < v1 ? sys_list[0] : sys_list[1]).get());
}

/** @par Test points */
TEST_CASE("test_findOptimalSystem_edge") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = analysis_sys_test_query();

    /** @arg an empty system list returns the initial value and a null system */
    auto empty = findOptimalSystem(SystemList(), stk, query, "Current Total Assets", 0);
    CHECK_UNARY(!empty.second);
    CHECK_UNARY(empty.first == std::numeric_limits<double>::lowest());

    /** @arg a null security returns the initial value and a null system */
    auto null_stk = findOptimalSystem({analysis_sys_test_system(crtTM())}, Stock(), query,
                                      "Current Total Assets", 0);
    CHECK_UNARY(!null_stk.second);
    CHECK_UNARY(null_stk.first == std::numeric_limits<double>::lowest());

    /** @arg a null system is skipped while the valid one is still selected */
    auto skipped = findOptimalSystem({SystemPtr(), analysis_sys_test_system(crtTM())}, stk, query,
                                     "Current Total Assets", 0);
    REQUIRE(skipped.second);
    CHECK_GT(skipped.first, 0.0);
}

/** @par Test points */
TEST_CASE("test_findOptimalSystemMulti_sort_mode") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = analysis_sys_test_query();

    /** @arg an empty system list returns the initial value and a null system */
    auto empty = findOptimalSystemMulti(SystemList(), stk, query, "Current Total Assets", 0);
    CHECK_UNARY(!empty.second);
    CHECK_UNARY(empty.first == std::numeric_limits<double>::lowest());

    // Two accounts with different initial cash, so that the two sort modes choose differently
    SystemList sys_list{analysis_sys_test_system(crtTM(Datetime(199911100000LL), 100000.0)),
                        analysis_sys_test_system(crtTM(Datetime(199911100000LL), 200000.0))};
    auto max_r = findOptimalSystemMulti(sys_list, stk, query, "Current Total Assets", 0);
    auto min_r = findOptimalSystemMulti(sys_list, stk, query, "Current Total Assets", 1);

    /** @arg the two sort modes select the opposite systems */
    REQUIRE(max_r.second);
    REQUIRE(min_r.second);
    CHECK_GT(max_r.first, min_r.first);
    CHECK_UNARY(max_r.second.get() != min_r.second.get());
}

/** @par Test points */
TEST_CASE("test_combinateIndicatorAnalysisWithBlock_basic") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = analysis_sys_test_query();

    // Two buy conditions sharing the short name produce three combinations
    Indicator buy1 = CLOSE() > OPEN();
    Indicator buy2 = CLOSE() > MA(10);
    Indicator sell = CLOSE() < 0;

    Block blk(StockList{stk});
    auto tm = crtTM();
    auto records = combinateIndicatorAnalysisWithBlock(blk, query, tm, analysis_sys_test_system(tm),
                                                       {buy1, buy2}, {sell}, 1);

    /** @arg every security times combination pair produces one record */
    REQUIRE(records.size() == 3);
    std::set<string> names;
    Performance per;
    for (const auto& record : records) {
        CHECK_UNARY(record.market_code == stk.market_code());
        CHECK_UNARY(record.name == stk.name());
        CHECK_UNARY(record.values.size() == per.names().size());
        names.insert(record.combinateName);
    }
    /** @arg the colliding combination names of the records stay distinguishable */
    CHECK_UNARY(names.size() == 3);
}

/** @} */
