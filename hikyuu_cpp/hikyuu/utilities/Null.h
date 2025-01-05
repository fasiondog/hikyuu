/*
 * Null.h
 *
 *  Created on: 2012-8-23
 *      Author: fasiondog
 */

#pragma once
#ifndef NULL_H_
#define NULL_H_

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <limits>
#include <type_traits>
#include "osdef.h"

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
    operator int() const {
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
    operator unsigned int() const {
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
    operator long long() const {
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
    operator unsigned long long() const {
        return (std::numeric_limits<unsigned long long>::max)();
    }
};

#if (HKU_OS_ANDROID && HKU_ARCH_ARM64) || (HKU_OS_LINUX && HKU_ARCH_ARM64) || HKU_OS_OSX || \
  HKU_OS_IOS || (defined(__GNUC__) && __GNUC__ >= 8) || (defined(__clang__) && !HKU_OS_ANDROID)
/**
 * 提供size_t的Null值
 */
template <>
class Null<std::size_t> {
public:
    Null() {}
    operator std::size_t() const {
        return (std::numeric_limits<std::size_t>::max)();
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
    operator double() const {
        return (std::numeric_limits<double>::quiet_NaN)();
        // return (std::numeric_limits<double>::max)();
    }

    bool operator==(double val) const {
        return std::isnan(val);
    }

    bool operator==(float val) const {
        return std::isnan(val);
    }
};

inline bool operator==(double val, const Null<double>&) {
    return std::isnan(val);
}

inline bool operator==(float val, const Null<double>&) {
    return std::isnan(val);
}

/**
 * 提供double的Null值
 */
template <>
class Null<float> {
public:
    Null() {}
    operator float() const {
        return (std::numeric_limits<float>::quiet_NaN)();
    }

    bool operator==(float val) const {
        return std::isnan(val);
    }

    bool operator==(double val) const {
        return std::isnan(val);
    }
};

inline bool operator==(float val, const Null<float>&) {
    return std::isnan(val);
}

inline bool operator==(double val, const Null<float>&) {
    return std::isnan(val);
}

/** @} */
}  // namespace hku

#endif /* NULL_H_ */
