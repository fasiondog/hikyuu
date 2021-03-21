/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-06
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpHandle.h"
#include "http/HttpFilter.h"

namespace hku {

class LoginHandle : public HttpHandle {
    CLASS_LOGGER(LoginHandle)

public:
    LoginHandle(nng_aio *aio) : HttpHandle(aio) {}

    virtual void run() override {
        json req = getReqJson();
        HTTP_VALID_CHECK(req.contains("user"), 2000, "Invalid login request! missing user");
        setResData(req);
    }
};

}  // namespace hku