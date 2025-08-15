/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-15
 *      Author: fasiondog
 */

#pragma once

#include <arrow/api.h>
#include "hikyuu/utilities/Log.h"

namespace hku {

#define HKU_ARROW_TABLE_CHECK(arrow_ret)                                                 \
    do {                                                                                 \
        auto _status = arrow_ret.status();                                               \
        if (!_status.ok()) {                                                             \
            throw hku::exception(fmt::format("HKU_CHECK(arrow::Result) {} [{}] ({}:{})", \
                                             _status.message(), HKU_FUNCTION, __FILE__,  \
                                             __LINE__));                                 \
        }                                                                                \
    } while (0)

/// \brief Propagate any non-successful Status to the caller
#define HKU_ARROW_RETURN_NOT_OK(arrow_ret)                  \
    do {                                                    \
        arrow::Status _status = arrow::ToStatus(arrow_ret); \
        if (!_status.ok()) {                                \
            HKU_ERROR(_status.message());                   \
            return arrow_ret;                               \
        }                                                   \
    } while (false)

#define HKU_ARROW_IF_RETURN(expr)            \
    do {                                     \
        if (expr) {                          \
            return arrow::Status::Invalid(); \
        }                                    \
    } while (0)

#define HKU_ARROW_TRACE_IF_RETURN(expr, ...)              \
    do {                                                  \
        if (expr) {                                       \
            std::string errmsg(fmt::format(__VA_ARGS__)); \
            HKU_TRACE(errmsg);                            \
            return arrow::Status::Invalid(errmsg);        \
        }                                                 \
    } while (0)

#define HKU_ARROW_DEBUG_IF_RETURN(expr, ...)              \
    do {                                                  \
        if (expr) {                                       \
            std::string errmsg(fmt::format(__VA_ARGS__)); \
            HKU_DEBUG(errmsg);                            \
            return arrow::Status::Invalid(errmsg);        \
        }                                                 \
    } while (0)

#define HKU_ARROW_INFO_IF_RETURN(expr, ...)               \
    do {                                                  \
        if (expr) {                                       \
            std::string errmsg(fmt::format(__VA_ARGS__)); \
            HKU_INFO(errmsg);                             \
            return arrow::Status::Invalid(errmsg);        \
        }                                                 \
    } while (0)

#define HKU_ARROW_WARN_IF_RETURN(expr, ...)               \
    do {                                                  \
        if (expr) {                                       \
            std::string errmsg(fmt::format(__VA_ARGS__)); \
            HKU_WARN(errmsg);                             \
            return arrow::Status::Invalid(errmsg);        \
        }                                                 \
    } while (0)

#define HKU_ARROW_ERROR_IF_RETURN(expr, ...)              \
    do {                                                  \
        if (expr) {                                       \
            std::string errmsg(fmt::format(__VA_ARGS__)); \
            HKU_ERROR(errmsg);                            \
            return arrow::Status::Invalid(errmsg);        \
        }                                                 \
    } while (0)

}  // namespace hku