/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-06-26
 *     Author: fasiondog
 */

#pragma once

#include <fmt/format.h>
#include "hikyuu/exception.h"

namespace hku {

/** MYSQL 处理异常 */
class MySQLException : public hku::exception {
public:
    MySQLException() = delete;

    /**
     * 构造 MYSQL 异常
     * @param errcode MYSQL 错误码
     * @param msg MYSQL 错误信息
     */
    MySQLException(int errcode, const std::string& msg) : hku::exception(msg), m_errcode(errcode) {}

    /**
     * 构造 MYSQL 异常
     * @param errcode MYSQL 错误码
     * @param msg MYSQL 错误信息
     */
    MySQLException(int errcode, const char* msg) : hku::exception(msg), m_errcode(errcode) {}

    /** 获取 MYSQL 错误码 */
    int errcode() const {
        return m_errcode;
    }

private:
    int m_errcode;
};

#define MYSQL_CHECK(expr, errcode, ...)                                                          \
    do {                                                                                         \
        if (!(expr)) {                                                                           \
            throw MySQLException(                                                                \
              errcode, fmt::format("CHECK({}) {} [{}] ({}:{})", #expr, fmt::format(__VA_ARGS__), \
                                   __FUNCTION__, __FILE__, __LINE__));                           \
        }                                                                                        \
    } while (0)

#define MYSQL_THROW(errcode, ...)                                                                \
    do {                                                                                         \
        throw MySQLException(errcode,                                                            \
                             fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                         __FUNCTION__, __FILE__, __LINE__));                     \
    } while (0)

}  // namespace hku