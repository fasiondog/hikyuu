/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpHandle.h"

namespace hku {

inline void AuthorizeFilter(HttpHandle *handle) {
    const char *token = handle->getReqHeader("hku_token");
    HTTP_VALID_CHECK(token, HttpValidErrorCode::MISS_TOKEN, "Miss token!");
    HTTP_VALID_CHECK(
      strcmp(token, "7c98806c0711cf996d602890e0ab9119d9a86afe04296ba69a16f0d9d76be755") == 0,
      HttpValidErrorCode::UNAUTHORIZED, "Failed authorize!");
}

class RestHandle : public HttpHandle {
    CLASS_LOGGER(RestHandle)

public:
    RestHandle(nng_aio *aio) : HttpHandle(aio) {
        addFilter(AuthorizeFilter);
    }

    virtual void before_run() override {
        setResHeader("Content-Type", "application/json; charset=UTF-8");
    }
};

#define REST_HANDLE_IMP(cls) \
public:                      \
    cls(nng_aio *aio) : RestHandle(aio) {}

}  // namespace hku