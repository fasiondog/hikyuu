/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include "../http/HttpTask.h"

namespace hku {

class HelloTask : public HttpTask {
public:
    HelloTask(nng_aio *aio) : HttpTask(aio) {}
};

void hello_handle(nng_aio *aio) {
    HKU_INFO("hello");
}

}  // namespace hku