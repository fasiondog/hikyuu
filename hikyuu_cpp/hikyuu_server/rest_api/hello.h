/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include "RestHandle.h"

namespace hku {

class HelloHandle : public RestHandle {
public:
    HelloHandle(nng_aio* aio) : RestHandle(aio) {}

    virtual void run() override {
        std::string req = getReqData();
        try {
            setResData("hello");
        } catch (std::exception& e) {
            CLS_INFO("req: {}", req);
            throw e;
        }
    }
};

}  // namespace hku