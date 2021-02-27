/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include <hikyuu/Log.h>
#include <nng/nng.h>
#include <nng/supplemental/http/http.h>

namespace hku {

#define HTTP_FATAL_CHECK(rv, msg)                                        \
    {                                                                    \
        if (rv != 0) {                                                   \
            HKU_FATAL("[HTTP_FATAL] {} err: {}", msg, nng_strerror(rv)); \
            nng_fini();                                                  \
            exit(0);                                                     \
        }                                                                \
    }

#define HTTP_ERROR_CHECK(rv, msg)                                        \
    {                                                                    \
        if (rv != 0) {                                                   \
            HKU_ERROR("[HTTP_FATAL] {} err: {}", msg, nng_strerror(rv)); \
        }                                                                \
    }

}  // namespace hku