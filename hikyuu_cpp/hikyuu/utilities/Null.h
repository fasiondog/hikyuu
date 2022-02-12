/*
 * Null.h
 *
 *  Created on: 2012-8-23
 *      Author: fasiondog
 */

#pragma once
#ifndef NULL_H_
#define NULL_H_

#include "../config.h"
#include <cstddef>
#include <limits>
#include <type_traits>

namespace hku {

/**
 * @ingroup Utilities
 * @addtogroup NullTools Null defines Null 值定义
 * @{
 */

/**
 * 提供指定给定类型的Null值
 */
template <typename T>
class Null {
public:
    Null() {}
    operator T() const {
        return T();
    }
};

/**
 * 提供int的Null值
 */
template <>
class Null<int> {
public:
    Null() {}
    operator int() {
        return (std::numeric_limits<int>::max)();
    }
};

/**
 * 提供unsigned int的Null值
 */
template <>
class Null<unsigned int> {
public:
    Null() {}
    operator unsigned int() {
        return (std::numeric_limits<unsigned int>::max)();
    }
};

/**
 * 提供long long（64位整型）的Null值
 */
template <>
class Null<long long> {
public:
    Null() {}
    operator long long() {
        return (std::numeric_limits<long long>::max)();
    }
};

#if !defined(_MSC_VER) && !HKU_OS_OSX
/**
 * int64_t Null值
 */
template <>
class Null<int64_t> {
public:
    Null() {}
    operator int64_t() {
        return (std::numeric_limits<int64_t>::max)();
    }
};
#endif

/**
 * 提供unsigned long long（无符号64位整型）的Null值
 */
template <>
class Null<unsigned long long> {
public:
    Null() {}
    operator unsigned long long() {
        return (std::numeric_limits<unsigned long long>::max)();
    }
};

#if !defined(_MSC_VER)
/**
 * 提供size_t的Null值
 */
template <>
class Null<size_t> {
public:
    Null() {}
    operator unsigned long long() {
        return (std::numeric_limits<size_t>::max)();
    }
};
#endif

/**
 * 提供double的Null值
 */
template <>
class Null<double> {
public:
    Null() {}
    operator double() {
        return (std::numeric_limits<double>::quiet_NaN)();
        // return (std::numeric_limits<double>::max)();
    }
};

/** @} */
}  // namespace hku

#endif /* NULL_H_ */
