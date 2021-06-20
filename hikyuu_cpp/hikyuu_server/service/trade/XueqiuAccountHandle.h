/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-06-21
 *     Author: fasiondog
 */

#pragma once

#include "db/db.h"
#include "service/RestHandle.h"
#include "service/user/model/UserModel.h"
#include "model/TradeAccountModel.h"
#include "model/XueqiuAccountModel.h"

namespace hku {

class QueryXueqiuAccountHandle : public RestHandle {
    REST_HANDLE_IMP(QueryXueqiuAccountHandle)
    virtual void run() override {
        auto userid = getCurrentUserId();

        std::vector<XueqiuAccountModel> xq_accounts;
        auto con = DB::getConnect();
    }
};

class AddXueqiuAccountHandle : public RestHandle {
    REST_HANDLE_IMP(AddXueqiuAccountHandle)
    virtual void run() override {}
};

class RemoveXueqiuAccountHandle : public RestHandle {
    REST_HANDLE_IMP(RemoveXueqiuAccountHandle)
    virtual void run() override {}
};

class ModifyXueqiuAccountHandle : public RestHandle {
    REST_HANDLE_IMP(ModifyXueqiuAccountHandle)
    virtual void run() override {}
};

}  // namespace hku