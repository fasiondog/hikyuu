/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-12
 *     Author: fasiondog
 */

#include "common/uuid.h"
#include "TradeAccountHandle.h"
#include "TradeService.h"

namespace hku {

void AddTradeAccountHandle::run() {
    json req = getReqJson();
    HTTP_VALID_CHECK(req.contains("name"), HttpErrorCode::MISS_PARAMETER,
                     R"(Missing param "name")");
    HTTP_VALID_CHECK(req.contains("type"), HttpErrorCode::MISS_PARAMETER,
                     R"(Missing param "type")");
    TradeAccountModel account;
    std::string name = req["name"].get<std::string>();
    account.setName(name);
    account.setType(req["type"].get<std::string>());
    account.setAccount(UUID());
    auto con = TradeService::getDBConnect();
    {
        TransAction trans(con);
        HTTP_VALID_CHECK(!TradeAccountModel::isExistName(con, name),
                         TradeErrorCode::TD_ACCOUNT_REPETITION, "Name repetition");
        con->save(account, false);
    }
    json res;
    to_json(res, account);
    setResData(res);
}

void GetTradeAccountHandle::run() {}

void ModTradeAccountHandle::run() {}

void DelTradeAccountHandle::run() {}

}  // namespace hku