/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-21
 *     Author: fasiondog
 */

#pragma once

#include "../../http/HttpHandle.h"

namespace hku {

class LogoutHandle : public HttpHandle {
    HTTP_HANDLE_IMP(LogoutHandle)

    virtual void run() override {
        setResHeader("Content-Type", "application/json; charset=UTF-8");
        setResData(R"({"result": true})");
    }
};

}  // namespace hku