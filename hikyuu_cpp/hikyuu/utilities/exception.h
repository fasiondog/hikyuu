/*
 * exception.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-23
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_EXCEPTION_H
#define HIKYUU_UTILITIES_EXCEPTION_H

#include <string>
#include <exception>

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4275)
#endif

/**
 * @ingroup Utilities
 * @addtogroup Exception Exception 异常处理
 * @{
 */

class HKU_UTILS_API exception : public std::exception {
public:
    exception() : m_msg("Unknown exception!") {}
    exception(const char *msg) : m_msg(msg) {}         // cppcheck-suppress noExplicitConstructor
    exception(const std::string &msg) : m_msg(msg) {}  // cppcheck-suppress noExplicitConstructor
    virtual ~exception() noexcept {}
    virtual const char *what() const noexcept;

protected:
    std::string m_msg;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_EXCEPTION_H */
