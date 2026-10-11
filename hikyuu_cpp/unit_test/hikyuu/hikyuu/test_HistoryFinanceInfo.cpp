/*
 * test_HistoryFinanceInfo.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <type_traits>
#include <hikyuu/HistoryFinanceInfo.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_HistoryFinanceInfo test_hikyuu_HistoryFinanceInfo
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_HistoryFinanceInfo_assign") {
    /** @arg The copy assignment may allocate, so it must not be marked noexcept */
    static_assert(
      !noexcept(std::declval<HistoryFinanceInfo&>() = std::declval<const HistoryFinanceInfo&>()),
      "HistoryFinanceInfo copy assignment must not be noexcept");

    /** @arg The move constructor and move assignment remain noexcept */
    static_assert(noexcept(HistoryFinanceInfo(std::declval<HistoryFinanceInfo&&>())),
                  "HistoryFinanceInfo move ctor must remain noexcept");
    static_assert(
      noexcept(std::declval<HistoryFinanceInfo&>() = std::declval<HistoryFinanceInfo&&>()),
      "HistoryFinanceInfo move assignment must remain noexcept");

    HistoryFinanceInfo src;
    src.values = {1.0f, 2.0f, 3.0f};

    /** @arg Copy assignment copies the values */
    HistoryFinanceInfo dst;
    dst = src;
    CHECK_EQ(dst.values.size(), 3u);
    CHECK_EQ(dst.values[0], doctest::Approx(1.0f));

    /** @arg Move assignment moves the values */
    HistoryFinanceInfo moved;
    moved = std::move(src);
    CHECK_EQ(moved.values.size(), 3u);
    CHECK_EQ(moved.values[2], doctest::Approx(3.0f));
}

/** @} */
