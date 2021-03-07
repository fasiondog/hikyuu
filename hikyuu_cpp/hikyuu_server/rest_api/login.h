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
        std::string content = getReqData();
        if (content.empty()) {
            setResStatus(NNG_HTTP_STATUS_BAD_REQUEST);
            return;
        }
        setResData(content);
    }
};

}  // namespace hku