/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240224 added by fasiondog
 */

#pragma once

#include "doctest/doctest.h"
#include <hikyuu/DataType.h>
#include <hikyuu/indicator/Indicator.h>

using namespace hku;

#define ENABLE_BENCHMARK_TEST 0  // 是否开启性能测试相关用例执行，默认不开启

// 跨编译器：强制编译器不优化、不删除指定变量
// 用于 benchmark，防止编译器把“没用”的中间计算删掉
#if defined(_MSC_VER)
#define DO_NOT_OPTIMIZE(var)                                             \
    do {                                                                 \
        auto p = const_cast<void*>(reinterpret_cast<const void*>(&var)); \
        __asm__ volatile("" : : "r"(p));                                 \
    } while (0)
#elif defined(__GNUC__) || defined(__clang__)
#define DO_NOT_OPTIMIZE(var) __asm__ volatile("" : : "g"(&var) : "memory");
#else
#define DO_NOT_OPTIMIZE(var)
#endif

inline void check_indicator(const Indicator& result, const Indicator& expect) {
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(), total = result.size(); i < total; ++i) {
        if (std::isnan(expect[i])) {
            CHECK_UNARY(std::isnan(result[i]));
        } else {
            CHECK_EQ(result[i], doctest::Approx(expect[i]).epsilon(0.00001));
        }
    }
}

inline void print_indicator(const Indicator& result) {
    HKU_INFO("Indicator: {}", result);
    for (size_t i = 0, total = result.size(); i < total; ++i) {
        HKU_INFO("{}: {:<.6f}", i, result[i]);
    }
}
inline void print_check_indicator(const Indicator& result, const Indicator& expect) {
    for (size_t i = 0, total = result.size(); i < total; ++i) {
        HKU_INFO("{}: {:<.6f}(result), {:<.6f}(expect)", i, result[i], expect[i]);
    }
}