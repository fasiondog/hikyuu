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

namespace hku {

/**
 * @ingroup Utilities
 * @addtogroup Exception Exception 异常处理
 * @{
 */

#if !defined(__clang__) && !defined(__GNUC__)
class exception : public std::exception {
public:
    exception() : std::exception("Unknown exception!") {}
    exception(const std::string& msg)  // cppcheck-suppress noExplicitConstructor
    : std::exception(msg.c_str()) {}
    exception(const char* msg) : std::exception(msg) {}  // cppcheck-suppress noExplicitConstructor
};

#else
// llvm 中的 std::exception 不接受参数
class exception : public std::exception {
public:
    exception() : m_msg("Unknown exception!") {}
    exception(const char *msg) : m_msg(msg) {}         // cppcheck-suppress noExplicitConstructor
    exception(const std::string &msg) : m_msg(msg) {}  // cppcheck-suppress noExplicitConstructor
    virtual ~exception() noexcept {}
    virtual const char *what() const noexcept {
        return m_msg.c_str();
    }

protected:
    std::string m_msg;
};
#endif /* #ifdef __clang__ */

} /* namespace hku */

#endif /* HIKYUU_UTILITIES_EXCEPTION_H */
