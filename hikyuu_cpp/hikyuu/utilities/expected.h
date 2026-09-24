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
// std::expected is used from C++23
#include <expected>
namespace stdx {
// Core type
using std::expected;
using std::unexpected;

// Exception type
using std::bad_expected_access;

// The in_place tag (used for the construction)
using std::in_place;
using std::in_place_t;
using std::in_place_type;
using std::in_place_type_t;
using std::unexpect;
using std::unexpect_t;
}  // namespace stdx

#else
// tl::expected is used below C++23
#include <tl/expected.hpp>
namespace stdx {
// Core type
using tl::expected;
using tl::unexpected;

// Exception type
using tl::bad_expected_access;

// The in_place tag (used for the construction)
using std::in_place;
using std::in_place_t;
using std::in_place_type;
using std::in_place_type_t;

// The unexpect tag of tl::expected
using tl::unexpect;
using tl::unexpect_t;
}  // namespace stdx
#endif

#endif /* HKU_UTILS_EXPECTED_H */
