/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include "../http/HttpHandle.h"

namespace hku {

class HelloHandle : public HttpHandle {
public:
    HelloHandle(nng_aio *aio) : HttpHandle(aio) {}

    virtual void run() override {
        setResponseData("hello");
    }
};

}  // namespace hku