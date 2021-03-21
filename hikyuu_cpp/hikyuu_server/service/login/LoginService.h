/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-21
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpService.h"
#include "LoginHandle.h"

namespace hku {

class LoginService : public HttpService {
public:
    LoginService() : HttpService() {}

    virtual void regHandle() override {
        POST<LoginHandle>("login");
    }
};

}  // namespace hku