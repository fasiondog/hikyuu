/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-21
 *     Author: fasiondog
 */

#pragma once

#include "../RestHandle.h"

namespace hku {

class StatusHandle : public RestHandle {
    REST_HANDLE_IMP(StatusHandle)

    virtual void run() override {
        res["status"] = "running";
    }
};

}  // namespace hku