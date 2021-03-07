/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#pragma once

#include "../http/HttpHandle.h"

namespace hku {

class HKUHandle : public HttpHandle {
    CLASS_LOGGER(HKUHandle)

public:
    HKUHandle(nng_aio *aio) : HttpHandle(aio) {}

    virtual void before_run() override;

    void error(int errcode) override;

    virtual void private_error(int errcode) {}
};

}  // namespace hku