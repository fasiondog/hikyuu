/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-27
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/utilities/thread/algorithm.h>

using namespace hku;

TEST_CASE("test_parallelIndexRange") {
    std::vector<std::pair<size_t, size_t>> expect{{0, 2}};
    auto result = parallelIndexRange(0, 2);
    CHECK_EQ(result.size(), 1);
    for (size_t i = 0, len = expect.size(); i < len; i++) {
        CHECK_EQ(result[i].first, expect[i].first);
        CHECK_EQ(result[i].second, expect[i].second);
    }

    size_t cpu_num = std::thread::hardware_concurrency();
    if (cpu_num == 32) {
        result = parallelIndexRange(0, 100);
        expect = {{0, 32}, {32, 64}, {64, 96}, {96, 97}, {97, 98}, {98, 99}, {99, 100}};
        CHECK_EQ(result.size(), expect.size());
        for (size_t i = 0, len = expect.size(); i < len; i++) {
            CHECK_EQ(result[i].first, expect[i].first);
            CHECK_EQ(result[i].second, expect[i].second);
        }

    } else if (cpu_num == 8) {
        result = parallelIndexRange(0, 35);
        expect = {{0, 8}, {8, 16}, {16, 24}, {24, 32}, {32, 33}, {33, 34}, {34, 35}};
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
