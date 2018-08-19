/*
 * Null.h
 *
 *  Created on: 2012-8-23
 *      Author: fasiondog
 */

#ifndef NULL_H_
#define NULL_H_

#include <boost/limits.hpp>
#include <boost/type_traits.hpp>

namespace hku {

/**
 * 提供指定给定类型的Null值
 * @ingroup Common-Utilities
 * @ingroup DataType
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
 * @ingroup Common-Utilities
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
 * @ingroup Common-Utilities
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
 * @ingroup Common-Utilities
 */
template <>
class Null<long long> {
public:
    Null() {}
    operator long long() {
        return (std::numeric_limits<long long>::max)();
    }
};

/**
 * 提供unsigned long long（无符号64位整型）的Null值
 * @ingroup Common-Utilities
 */
template <>
class Null<unsigned long long> {
public:
    Null() {}
    operator unsigned long long() {
        return (std::numeric_limits<unsigned long long>::max)();
    }
};

#if !defined(BOOST_MSVC)
/**
 * 提供size_t的Null值
 * @ingroup Common-Utilities
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
 * @ingroup Common-Utilities
 */
template <>
class Null<double> {
public:
    Null() {}
    operator double() {
        return (std::numeric_limits<double>::max)();
    }
};

} /* namesapce hku */

#endif /* NULL_H_ */
