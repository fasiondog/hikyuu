/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-29
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpService.h"
#include "WalletHandle.h"

namespace hku {

class TradeService : public HttpService {
    HTTP_SERVICE_IMP(TradeService)

    virtual void regHandle() override {
        GET<WalletHandle>("wallet");
    }
};

}  // namespace hku