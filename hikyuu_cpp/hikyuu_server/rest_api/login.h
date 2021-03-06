/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-06
 *     Author: fasiondog
 */

#pragma once

#include "../http/HttpHandle.h"

namespace hku {

class LoginHandle : public HttpHandle {
public:
    LoginHandle(nng_aio *aio) : HttpHandle(aio) {}

    virtual void run() override {
        std::string content = getRequestData();
        if (content.empty()) {
            setResponseStatus(NNG_HTTP_STATUS_NO_CONTENT);
            return;
        }
        setResponseData(content);
    }
};

}  // namespace hku