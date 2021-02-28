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
        HTTP_NNG_CHECK(nng_http_res_copy_data(m_nng_res, "hello", 5),
                       "Failed nng_http_res_copy_data");
    }
};

}  // namespace hku