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
    virtual void run() override;
};

class GetTradeAccountHandle : public RestHandle {
    REST_HANDLE_IMP(GetTradeAccountHandle)
    virtual void run() override;
};

class ModTradeAccountHandle : public RestHandle {
    REST_HANDLE_IMP(ModTradeAccountHandle)
    virtual void run() override;
};

class DelTradeAccountHandle : public RestHandle {
    REST_HANDLE_IMP(DelTradeAccountHandle)
    virtual void run() override;
};

}  // namespace hku