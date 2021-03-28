/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-06
 *     Author: fasiondog
 */

#pragma once

#include "../../http/HttpHandle.h"

namespace hku {

class LoginHandle : public HttpHandle {
    HTTP_HANDLE_IMP(LoginHandle)

    virtual void run() override {
        json req = getReqJson();
        HTTP_VALID_CHECK(req.contains("user"), 2000, "Invalid login request! missing user");
        setResHeader("Content-Type", "application/json; charset=UTF-8");
        setResData(
          R"({"hku_token":"7c98806c0711cf996d602890e0ab9119d9a86afe04296ba69a16f0d9d76be755"})");
    }
};

}  // namespace hku