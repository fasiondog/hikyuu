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
 * @addtogroup NullTools Null defines, the Null value definitions
 * @{
 */

/**
 * It provides the Null value of the given type
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
 * It provides the Null value of int
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
 * It provides the Null value of unsigned int
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
 * It provides the Null value of long long (the 64-bit integer)
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
 * The int64_t Null value
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
 * It provides the Null value of unsigned long long (the unsigned 64-bit integer)
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
  HKU_OS_IOS || (defined(__GNUC__) && __GNUC__ >= 8) ||                                     \
  (defined(__clang__) && !HKU_OS_ANDROID && !HKU_OS_WINDOWS)
/**
 * It provides the Null value of size_t
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
 * It provides the Null value of double
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
 * It provides the Null value of double
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
