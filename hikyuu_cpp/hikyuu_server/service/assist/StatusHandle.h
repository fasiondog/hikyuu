/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-21
 *     Author: fasiondog
 */

#pragma once

#include "../RestHandle.h"
#include "bcrypt/BCrypt.hpp"

namespace hku {

class StatusHandle : public RestHandle {
    REST_HANDLE_IMP(StatusHandle)

    virtual void run() override {
        res["status"] = "running";
        auto salt = BCrypt::gensalt();
        auto hashed = BCrypt::hashpw("123456", salt);
        HKU_INFO("hashed: {}", hashed);
        HKU_INFO("checked: {}", BCrypt::checkpw("admin", hashed));
        HKU_INFO("checked: {}", BCrypt::checkpw("123456", hashed));
    }
};

}  // namespace hku