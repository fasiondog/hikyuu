/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include "HKUHandle.h"

namespace hku {

class HelloHandle : public HKUHandle {
public:
    HelloHandle(nng_aio *aio) : HKUHandle(aio) {}

    virtual void run() override {
        setResData("hello");
    }
};

}  // namespace hku