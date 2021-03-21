/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-21
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpService.h"
#include "StatusHandle.h"
#include "LogLevelHandle.h"

namespace hku {

class AssistService : public HttpService {
    HTTP_SERVICE_IMP(AssistService)

    virtual void regHandle() override {
        GET<StatusHandle>("status");
        POST<LogLevelHandle>("log_level");
    }
};

}  // namespace hku