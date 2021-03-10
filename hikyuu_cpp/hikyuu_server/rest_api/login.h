/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-06
 *     Author: fasiondog
 */

#pragma once

#include "HKUHandle.h"

namespace hku {

class LoginHandle : public HKUHandle {
public:
    LoginHandle(nng_aio *aio) : HKUHandle(aio) {}

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