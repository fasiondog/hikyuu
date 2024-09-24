/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-10
 *      Author: fasiondog
 */

#include "../test_config.h"

#include <hikyuu/analysis/combinate.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_combinate test_hikyuu_combinate
 * @ingroup test_hikyuu_analysis_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_combinateIndex") {
    std::vector<float> nums;
    std::vector<std::vector<size_t>> result;
    std::vector<std::vector<size_t>> expect;

    /** @arg 输入序列长度为0 */
    result = combinateIndex(nums);
    CHECK_UNARY(result.empty());

    /** @arg 输入序列长度为1 */
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

    /** @arg 输入序列长度为2 */
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

    /** @arg 输入序列长度为3 */
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

    /** @arg 输入序列长度为4 */
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

/** @} */