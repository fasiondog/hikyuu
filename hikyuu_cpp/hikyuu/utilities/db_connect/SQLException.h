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

/** SQL handling exception, it is mainly used to encapsulate the internal error of a concrete
 * database engine */
class SQLException : public hku::exception {
public:
    /** Default constructor */
    SQLException() : SQLException(0, "Unknow error!") {}

    /**
     * Construct a SQLite exception
     * @param errcode SQLite error code
     * @param msg SQLite error message
     */
    SQLException(int errcode, const std::string& msg)
    : hku::exception(fmt::format("{} (errcode: {})", msg, errcode)), m_errcode(errcode) {}

    /**
     * Construct a SQLite exception
     * @param errcode SQLite error code
     * @param msg SQLite error message
     */
    SQLException(int errcode, const char* msg)
    : hku::exception(fmt::format("{} (errcode: {})", msg, errcode)), m_errcode(errcode) {}

    /** Get the SQLite error code */
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