/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-10
 *      Author: fasiondog
 */

#include "../test_config.h"

#include <algorithm>
#include <hikyuu/StockManager.h>
#include <hikyuu/analysis/analysis_sys.h>
#include <hikyuu/analysis/combinate.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_sys/signal/crt/SG_Bool.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_combinate test_hikyuu_combinate
 * @ingroup test_hikyuu_analysis_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_combinateIndex") {
    std::vector<float> nums;
    std::vector<std::vector<size_t>> result;
    std::vector<std::vector<size_t>> expect;

    /** @arg The input sequence length is 0 */
    result = combinateIndex(nums);
    CHECK_UNARY(result.empty());

    /** @arg The input sequence length is 1 */
    nums.push_back(0.1f);
    result = combinateIndex(nums);
    expect = {{0}};
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0, total = result.size(); i < total; i++) {
        CHECK_EQ(result[i].size(), expect[i].size());
        for (size_t j = 0, len = result[i].size(); j < len; j++) {
            CHECK_EQ(result[i][j], expect[i][j]);
        }
    }

    /** @arg The input sequence length is 2 */
    nums.push_back(0.2f);
    result = combinateIndex(nums);
    expect = {{0}, {0, 1}, {1}};
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0, total = result.size(); i < total; i++) {
        CHECK_EQ(result[i].size(), expect[i].size());
        for (size_t j = 0, len = result[i].size(); j < len; j++) {
            CHECK_EQ(result[i][j], expect[i][j]);
        }
    }

    /** @arg The input sequence length is 3 */
    nums.push_back(0.3f);
    result = combinateIndex(nums);
    expect = {{0}, {0, 1}, {1}, {0, 2}, {0, 1, 2}, {1, 2}, {2}};
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0, total = result.size(); i < total; i++) {
        CHECK_EQ(result[i].size(), expect[i].size());
        for (size_t j = 0, len = result[i].size(); j < len; j++) {
            CHECK_EQ(result[i][j], expect[i][j]);
        }
    }

    /** @arg The input sequence length is 4 */
    nums.push_back(0.4f);
    result = combinateIndex(nums);
    expect = {{0},       {0, 1}, {1},       {0, 2},       {0, 1, 2}, {1, 2}, {2}, {0, 3},
              {0, 1, 3}, {1, 3}, {0, 2, 3}, {0, 1, 2, 3}, {1, 2, 3}, {2, 3}, {3}};
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0, total = result.size(); i < total; i++) {
        CHECK_EQ(result[i].size(), expect[i].size());
        for (size_t j = 0, len = result[i].size(); j < len; j++) {
            CHECK_EQ(result[i][j], expect[i][j]);
        }
    }
}

/**
 * @par Test points
 * The combinate analysis must stop its performance statistics at the last trading day of the
 * query, like analysisSystemList does, and not at Datetime::now().
 *
 * Background: getFunds(datetime) values an open position with the last available close at or before
 * datetime and adjusts the positions by the ex-rights data up to that moment, so now() reaches past
 * the backtest window whenever the local data continues after it.
 */
TEST_CASE("test_combinateIndicatorAnalysis_end_date") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    // the local data has to continue after the query, or both ends give the same price
    KData all = stk.getKData(KQuery(0));
    REQUIRE(all.size() > 300);
    REQUIRE(all[all.size() - 1].datetime > Datetime(200002250000LL));
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL));

    // a buy that keeps filling and a sell that never fires, so the account is always open at the
    // end
    Indicator buy = CLOSE() > OPEN();
    Indicator sell = CLOSE() < 0;

    auto tm = crtTM();
    auto sys = SYS_Simple(tm, MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Bool(buy, sell));
    auto combinate = combinateIndicatorAnalysis(stk, query, tm, sys, {buy}, {sell}, 1);
    REQUIRE(combinate.size() == 1);

    auto ref_sys =
      SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Bool(buy, sell));
    auto ref = analysisSystemList({ref_sys}, stk, query);
    REQUIRE(ref.size() == 1);

    const Performance& per = combinate.begin()->second;

    PriceList got = per.values();
    const PriceList& expect = ref[0].values;
    const StringList& names = per.names();
    REQUIRE(got.size() == expect.size());
    REQUIRE(names.size() == got.size());

    // the account must still hold a position at the end of the query, otherwise the statistics end
    // moment cannot be observed at all
    size_t mv_idx =
      std::find(names.begin(), names.end(), "Open Position Net Value") - names.begin();
    REQUIRE(mv_idx < names.size());
    REQUIRE(expect[mv_idx] > 0.0);

    for (size_t i = 0; i < got.size(); ++i) {
        if (std::isnan(got[i]) && std::isnan(expect[i])) {
            continue;
        }
        CHECK_MESSAGE(got[i] == doctest::Approx(expect[i]).epsilon(0.0001), names[i]);
    }
}

/** @} */