/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-14
 *     Author: fasiondog
 */

#include "RestFilter.h"

namespace hku {

#define FILTER_CHECK(expr, errcode)         \
    {                                       \
        if (!expr) {                        \
            throw RestFilterError(errcode); \
        }                                   \
    }

void TokenAuthorizeFilter(HttpHandle *handle) {
    const char *token = handle->getReqHeader("token");
    FILTER_CHECK(token, RestFilterError::Code::MISS_TOKEN);
}

}  // namespace hku