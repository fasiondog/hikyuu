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

/** SQL 数据库引擎处理异常 */
class SQLException : public hku::exception {
public:
    SQLException() = delete;

    /**
     * 构造 SQLite 异常
     * @param errcode SQLite错误码
     * @param msg SQLite错误信息
     */
    SQLException(int errcode, const std::string& msg) : hku::exception(msg), m_errcode(errcode) {}

    /**
     * 构造 SQLite 异常
     * @param errcode SQLite错误码
     * @param msg SQLite错误信息
     */
    SQLException(int errcode, const char* msg) : hku::exception(msg), m_errcode(errcode) {}

    /** 获取 SQLite 错误码 */
    int errcode() const {
        return m_errcode;
    }

private:
    int m_errcode;
};

#define SQL_CHECK(expr, errcode, ...)                                                            \
    do {                                                                                         \
        if (!(expr)) {                                                                           \
            throw SQLException(                                                                  \
              errcode, fmt::format("CHECK({}) {} [{}] ({}:{})", #expr, fmt::format(__VA_ARGS__), \
                                   __FUNCTION__, __FILE__, __LINE__));                           \
        }                                                                                        \
    } while (0)

#define SQL_THROW(errcode, ...)                                                                \
    do {                                                                                       \
        throw SQLException(errcode,                                                            \
                           fmt::format("EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                       __FUNCTION__, __FILE__, __LINE__));                     \
    } while (0)

}  // namespace hku