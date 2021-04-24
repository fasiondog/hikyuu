/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-11
 *     Author: fasiondog
 */

#pragma once

#include <iostream>
#include "../RestHandle.h"
#include "model/TradeAccountModel.h"

namespace hku {

class AddTradeAccountHandle : public RestHandle {
    REST_HANDLE_IMP(AddTradeAccountHandle)
    virtual void run() override {
        check_missing_param("name");
        check_missing_param("type");
        TradeAccountModel account;
        std::string name = req["name"].get<std::string>();
        std::string td_type = req["type"].get<std::string>();
        check_enum_field<TradeAccountModel>("type", td_type);
        account.setName(name);
        account.setType(td_type);
        account.setTdId(DB::getNewTdId());
        auto con = DB::getConnect();
        {
            TransAction trans(con);
            HTTP_CHECK(!TradeAccountModel::isExistName(con, name),
                       TradeErrorCode::TD_ACCOUNT_REPETITION, "Name repetition");
            con->save(account, false);
        }
        setResData(account.json());
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