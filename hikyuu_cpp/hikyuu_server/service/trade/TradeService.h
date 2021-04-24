/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-29
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpService.h"
#include "WalletHandle.h"
#include "TradeAccountHandle.h"

namespace hku {

class TradeService : public HttpService {
    CLASS_LOGGER(TradeService)

public:
    TradeService() = delete;
    TradeService(const char* url) : HttpService(url) {}

    virtual void regHandle() override {
        GET<WalletHandle>("wallet");

        POST<AddTradeAccountHandle>("account");
        GET<GetTradeAccountHandle>("account");
        PUT<ModTradeAccountHandle>("account");
        DEL<DelTradeAccountHandle>("account");
    }
};

}  // namespace hku