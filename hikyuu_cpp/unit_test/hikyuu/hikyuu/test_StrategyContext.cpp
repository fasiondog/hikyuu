/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-07
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <type_traits>
#include <utility>
#include "hikyuu/StrategyContext.h"

/**
 * @defgroup test_hikyuu_StrategyContext test_hikyuu_StrategyContext
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_StrategyContext") {
    StrategyContext sc;
    sc.setKTypeList(
      {KQuery::MONTH, KQuery::MIN5, KQuery::DAY, KQuery::MIN, KQuery::WEEK, KQuery::MIN60});

    vector<KQuery::KType> expect{KQuery::MONTH, KQuery::MIN5, KQuery::DAY,
                                 KQuery::MIN,   KQuery::WEEK, KQuery::MIN60};
    const auto ktypes = sc.getKTypeList();
    for (size_t i = 0, len = ktypes.size(); i < len; i++) {
        CHECK_EQ(ktypes[i], expect[i]);
    }
}

/** @par Test points */
TEST_CASE("test_StrategyContext_noexcept_contract") {
    /** @arg isAll and getAllNeedLoadStockCodeList may allocate, so they must not be noexcept */
    static_assert(!noexcept(std::declval<const StrategyContext&>().isAll()),
                  "isAll must not be noexcept");
    static_assert(!noexcept(std::declval<const StrategyContext&>().getAllNeedLoadStockCodeList()),
                  "getAllNeedLoadStockCodeList must not be noexcept");
}

/** @par Test points */
TEST_CASE("test_StrategyContext_dedup") {
    /** @arg Duplicate stock codes must be removed, preserving first-seen order (normalized to
     * uppercase, the canonical internal market_code form) */
    StrategyContext sc({"sh000001", "sz000002", "sh000001", "sh000001", "sz000002"});
    const auto& codes = sc.getStockCodeList();
    CHECK_EQ(codes.size(), 2u);
    CHECK_EQ(codes[0], "SH000001");
    CHECK_EQ(codes[1], "SZ000002");

    /** @arg Duplicate ktypes must be removed, preserving first-seen order */
    StrategyContext sc2({"sh600000"},
                        {KQuery::DAY, KQuery::MIN, KQuery::DAY, KQuery::WEEK, KQuery::MIN,
                         KQuery::WEEK});
    const auto& ktypes = sc2.getKTypeList();
    CHECK_EQ(ktypes.size(), 3u);
    CHECK_EQ(ktypes[0], KQuery::DAY);
    CHECK_EQ(ktypes[1], KQuery::MIN);
    CHECK_EQ(ktypes[2], KQuery::WEEK);

    /** @arg Mixed-case duplicates are also removed and codes are normalized to uppercase */
    StrategyContext sc3({"sh600000", "SH600000", "sz000001"});
    const auto& codes3 = sc3.getStockCodeList();
    CHECK_EQ(codes3.size(), 2u);
    CHECK_EQ(codes3[0], "SH600000");
    CHECK_EQ(codes3[1], "SZ000001");

    /** @arg "ALL" is case insensitive */
    StrategyContext sc4({"ALL"});
    CHECK_UNARY(sc4.isAll());
    StrategyContext sc5({"all"});
    CHECK_UNARY(sc5.isAll());

    /** @arg Codes overlapping the default must-load list are deduped in the need-load list */
    StrategyContext sc6({"sh000001", "sh000001"});
    auto need_load = sc6.getAllNeedLoadStockCodeList();
    CHECK_EQ(need_load.size(), 2u);
    CHECK_EQ(need_load[0], "SH000001");
    CHECK_EQ(need_load[1], "SH000300");
}

/** @} */