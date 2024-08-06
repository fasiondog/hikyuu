/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-06-26
 *     Author: fasiondog
 */

#pragma once

#include <fmt/format.h>
#include "hikyuu/utilities/exception.h"

namespace hku {

/** SQL 处理异常, 主要用于封装具体数据库引擎内部错误 */
class SQLException : public hku::exception {
public:
    /** 默认构造函数 */
    SQLException() : SQLException(0, "Unknow error!") {}

    /**
     * 构造 SQLite 异常
     * @param errcode SQLite错误码
     * @param msg SQLite错误信息
     */
    SQLException(int errcode, const std::string& msg)
    : hku::exception(fmt::format("{} (errcode: {})", msg, errcode)), m_errcode(errcode) {}

    /**
     * 构造 SQLite 异常
     * @param errcode SQLite错误码
     * @param msg SQLite错误信息
     */
    SQLException(int errcode, const char* msg)
    : hku::exception(fmt::format("{} (errcode: {})", msg, errcode)), m_errcode(errcode) {}

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
              errcode, fmt::format("SQL_CHECK({}) {} [{}] ({}:{})", #expr,                       \
                                   fmt::format(__VA_ARGS__), __FUNCTION__, __FILE__, __LINE__)); \
        }                                                                                        \
    } while (0)

#define SQL_THROW(errcode, ...)                                                                    \
    do {                                                                                           \
        throw SQLException(errcode,                                                                \
                           fmt::format("SQL_EXCEPTION: {} [{}] ({}:{})", fmt::format(__VA_ARGS__), \
                                       __FUNCTION__, __FILE__, __LINE__));                         \
    } while (0)

}  // namespace hku