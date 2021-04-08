/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-29
 *     Author: fasiondog
 */

#pragma once

#include <hikyuu/utilities/IniParser.h>
#include "http/HttpService.h"
#include "WalletHandle.h"

namespace hku {

class TradeService : public HttpService {
    CLASS_LOGGER(TradeService)

public:
    TradeService() = delete;
    TradeService(const char *url) : HttpService(url) {}
    TradeService(const char *url, const std::string &config_file) : HttpService(url) {
        IniParser ini;
        ini.read(config_file);
    }

    virtual void regHandle() override {
        GET<WalletHandle>("wallet");
    }
};

}  // namespace hku