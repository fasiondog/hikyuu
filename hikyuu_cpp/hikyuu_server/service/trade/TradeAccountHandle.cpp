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
    check_missing("name");
    check_missing("type");
    TradeAccountModel account;
    std::string name = req["name"].get<std::string>();
    std::string td_type = req["type"].get<std::string>();
    HTTP_CHECK(TradeService::isValidEumValue(TradeAccountModel::getTableName(), "type", td_type),
               TradeErrorCode::TD_ACCOUNT_INVALD_TYPE, "Invalid trade account type: {}", td_type);
    account.setName(name);
    account.setType(td_type);
    account.setTdId(TradeService::newTdId());
    auto con = TradeService::getDBConnect();
    {
        TransAction trans(con);
        HTTP_CHECK(!TradeAccountModel::isExistName(con, name),
                   TradeErrorCode::TD_ACCOUNT_REPETITION, "Name repetition");
        con->save(account, false);
    }
    setResData(account.json());
}

void GetTradeAccountHandle::run() {}

void ModTradeAccountHandle::run() {}

void DelTradeAccountHandle::run() {}

}  // namespace hku