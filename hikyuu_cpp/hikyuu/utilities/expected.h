/*
 * expected.h
 *
 *  Copyright (c) 2025, hikyuu.org
 *
 *  Created on: 2026-05-12
 *      Author: fasiondog
 */
#pragma once
#ifndef HKU_UTILS_EXPECTED_H
#define HKU_UTILS_EXPECTED_H

#include "cppdef.h"

#if CPP_STANDARD >= CPP_STANDARD_23
// C++23 及以上使用 std::expected
#include <expected>
namespace stdx {
// 核心类型
using std::expected;
using std::unexpected;

// 异常类型
using std::bad_expected_access;

// in_place 标签（用于构造）
using std::in_place;
using std::in_place_t;
using std::in_place_type;
using std::in_place_type_t;
using std::unexpect;
using std::unexpect_t;
}  // namespace stdx

#else
// C++23 以下使用 tl::expected
#include <tl/expected.hpp>
namespace stdx {
// 核心类型
using tl::expected;
using tl::unexpected;

// 异常类型
using tl::bad_expected_access;

// in_place 标签（用于构造）
using std::in_place;
using std::in_place_t;
using std::in_place_type;
using std::in_place_type_t;

// tl::expected 的 unexpect 标签
using tl::unexpect;
using tl::unexpect_t;
}  // namespace stdx
#endif

#endif /* HKU_UTILS_EXPECTED_H */
