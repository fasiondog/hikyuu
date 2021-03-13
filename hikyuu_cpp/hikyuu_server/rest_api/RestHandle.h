/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#pragma once

#include "../http/HttpHandle.h"

namespace hku {

class RestHandle : public HttpHandle {
    CLASS_LOGGER(RestHandle)

public:
    RestHandle(nng_aio *aio);
    virtual void before_run() override;
};

}  // namespace hku