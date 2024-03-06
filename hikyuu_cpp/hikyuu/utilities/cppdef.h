/**
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2021/05/19
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_CPP_DEF_H_
#define HKU_CPP_DEF_H_

#if defined(__clang__) || defined(__GNUC__)
#define CPP_STANDARD __cplusplus

#elif defined(_MSC_VER)
#define CPP_STANDARD _MSVC_LANG
#endif

#define CPP_STANDARD_03 199711L
#define CPP_STANDARD_11 201103L
#define CPP_STANDARD_14 201402L
#define CPP_STANDARD_17 201703L
#define CPP_STANDARD_20 202002L

#endif  // HKU_CPP_DEF_H_
