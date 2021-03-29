/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-29
 *     Author: fasiondog
 */

#pragma once

#include "../RestHandle.h"

namespace hku {

/**
 * 资金账户信息
 */
class WalletHandle : public RestHandle {
    REST_HANDLE_IMP(WalletHandle)

    virtual void run() override {
        setResData(fmt::format(
          R"([{{"currency":"CNY", "balance":"100000","available":"100000", "hold":"0"}}])"));
    }
};

}  // namespace hku