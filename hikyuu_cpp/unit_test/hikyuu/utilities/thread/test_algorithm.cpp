/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-27
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/utilities/thread/algorithm.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_algorithm test_hikyuu_algorithm
 * @ingroup test_hikyuu_utilities
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_parallelIndexRange") {
    /** @arg start=0, end=0 */
    std::vector<range_t> ranges = parallelIndexRange(0, 0);
    CHECK_UNARY(ranges.empty());

    /** @arg start=2, end=2 */
    ranges = parallelIndexRange(2, 2);
    CHECK_UNARY(ranges.empty());

    /** @arg start=1, end=0 */
    ranges = parallelIndexRange(1, 0);
    CHECK_UNARY(ranges.empty());

    /** @arg start=0, end=3 */
    ranges = parallelIndexRange(0, 3);
    CHECK_UNARY(ranges.size() > 0);

    std::vector<size_t> expect(3);
    for (size_t i = 0; i < expect.size(); ++i) {
        expect[i] = i;
    }

    std::vector<size_t> result;
    for (const auto& r : ranges) {
        for (size_t i = r.first; i < r.second; ++i) {
            result.push_back(i);
        }
    }

    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg start=0, end=100 */
    ranges = parallelIndexRange(0, 100);
    CHECK_UNARY(ranges.size() > 0);

    expect.resize(100);
    for (size_t i = 0; i < expect.size(); ++i) {
        expect[i] = i;
    }

    result.clear();
    for (const auto& r : ranges) {
        for (size_t i = r.first; i < r.second; ++i) {
            result.push_back(i);
        }
    }

    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg start=1, end=100 */
    ranges = parallelIndexRange(1, 100);
    CHECK_UNARY(ranges.size() > 0);

    expect.resize(99);
    for (size_t i = 0; i < expect.size(); ++i) {
        expect[i] = i + 1;
    }

    result.clear();
    for (const auto& r : ranges) {
        for (size_t i = r.first; i < r.second; ++i) {
            result.push_back(i);
        }
    }

    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg start=99, end=100 */
    ranges = parallelIndexRange(99, 100);
    CHECK_UNARY(ranges.size() > 0);

    expect.resize(1);
    expect[0] = 99;

    result.clear();
    for (const auto& r : ranges) {
        for (size_t i = r.first; i < r.second; ++i) {
            result.push_back(i);
        }
    }

    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg start=99, end=100 */
    ranges = parallelIndexRange(17, 50);
    CHECK_UNARY(ranges.size() > 0);

    expect.resize(50 - 17);
    for (size_t i = 0; i < expect.size(); ++i) {
        expect[i] = i + 17;
    }

    result.clear();
    for (const auto& r : ranges) {
        for (size_t i = r.first; i < r.second; ++i) {
            result.push_back(i);
        }
    }

    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], expect[i]);
    }
}

TEST_CASE("test_parallelIndexRange2") {
    std::vector<std::pair<size_t, size_t>> expect{{0, 1}, {1, 2}};
    auto result = parallelIndexRange(0, 2);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0, len = expect.size(); i < len; i++) {
        CHECK_EQ(result[i].first, expect[i].first);
        CHECK_EQ(result[i].second, expect[i].second);
    }

    size_t cpu_num = std::thread::hardware_concurrency();
    if (cpu_num == 32) {
        result = parallelIndexRange(0, 100);
        expect = {{0, 3},   {3, 6},   {6, 9},   {9, 12},  {12, 15}, {15, 18}, {18, 21}, {21, 24},
                  {24, 27}, {27, 30}, {30, 33}, {33, 36}, {36, 39}, {39, 42}, {42, 45}, {45, 48},
                  {48, 51}, {51, 54}, {54, 57}, {57, 60}, {60, 63}, {63, 66}, {66, 69}, {69, 72},
                  {72, 75}, {75, 78}, {78, 81}, {81, 84}, {84, 87}, {87, 90}, {90, 93}, {93, 96},
                  {96, 97}, {97, 98}, {98, 99}, {99, 100}};
        CHECK_EQ(result.size(), expect.size());
        for (size_t i = 0, len = expect.size(); i < len; i++) {
            CHECK_EQ(result[i].first, expect[i].first);
            CHECK_EQ(result[i].second, expect[i].second);
        }

    } else if (cpu_num == 8) {
        result = parallelIndexRange(0, 35);
        expect = {{0, 4},   {4, 8},   {8, 12},  {12, 16}, {16, 20}, {20, 24},
                  {24, 28}, {28, 32}, {32, 33}, {33, 34}, {34, 35}};
        CHECK_EQ(result.size(), expect.size());
        for (size_t i = 0, len = expect.size(); i < len; i++) {
            CHECK_EQ(result[i].first, expect[i].first);
            CHECK_EQ(result[i].second, expect[i].second);
        }
    }
}

TEST_CASE("test_parallel_for_index") {
    std::vector<int> values(100);
    for (size_t i = 0, len = values.size(); i < len; i++) {
        values[i] = i;
    }

    auto result = parallel_for_index(0, values.size(), [](size_t i) { return i + 1; });

    std::vector<int> expect(100);
    for (size_t i = 0, len = expect.size(); i < len; i++) {
        expect[i] = i + 1;
    }
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0, len = expect.size(); i < len; i++) {
        CHECK_EQ(result[i], expect[i]);
    }
}

/** @} */