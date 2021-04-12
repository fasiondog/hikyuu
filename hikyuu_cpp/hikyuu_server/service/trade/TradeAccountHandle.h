/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-11
 *     Author: fasiondog
 */

#pragma once

#include "../RestHandle.h"
#include "model/TradeAccountModel.h"

namespace hku {

class AddTradeAccountHandle : public RestHandle {
    REST_HANDLE_IMP(AddTradeAccountHandle)

    virtual void run() override {
        json req = getReqJson();
        HTTP_VALID_CHECK(req.contains("name"), HttpErrorCode::MISS_PARAMETER,
                         R"(Missing param "name")");
        HTTP_VALID_CHECK(req.contains("type"), HttpErrorCode::MISS_PARAMETER,
                         R"(Missing param "type")");
        TradeAccountModel account;
        account.setName(req["name"].get<std::string>());
        account.setType(req["type"].get<std::string>());
        LOG_INFO("{}", req);
        setResData(R"([{"result": true, "account": "12345"}])");
    }
};

class GetTradeAccountHandle : public RestHandle {
    REST_HANDLE_IMP(GetTradeAccountHandle)

    virtual void run() override {}
};

class ModTradeAccountHandle : public RestHandle {
    REST_HANDLE_IMP(ModTradeAccountHandle)

    virtual void run() override {}
};

class DelTradeAccountHandle : public RestHandle {
    REST_HANDLE_IMP(DelTradeAccountHandle)

    virtual void run() override {}
};

}  // namespace hku