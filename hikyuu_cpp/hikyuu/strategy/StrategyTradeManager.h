/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-23
 *     Author: fasiondog
 */

#pragma once

#include "../trade_manage/TradeManagerBase.h"

namespace hku {

class HKU_API StrategyTradeManager : public TradeManagerBase {
public:
    StrategyTradeManager() = default;
    virtual ~StrategyTradeManager() = default;
};

}  // namespace hku