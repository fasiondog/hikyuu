/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include "HttpMicro.h"

namespace hku {

class HttpTask {
public:
    HttpTask(nng_aio *aio);

    void run() {
        HKU_INFO("HttpTask");
    }

    void operator()();

protected:
    nng_aio *m_http_aio{nullptr};
};

}  // namespace hku
