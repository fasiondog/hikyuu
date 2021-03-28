/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-21
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpService.h"
#include "LoginHandle.h"
#include "LogoutHandle.h"

namespace hku {

class AccountService : public HttpService {
    HTTP_SERVICE_IMP(AccountService)

    virtual void regHandle() override {
        POST<LoginHandle>("login");
        POST<LogoutHandle>("logout");
    }
};

}  // namespace hku