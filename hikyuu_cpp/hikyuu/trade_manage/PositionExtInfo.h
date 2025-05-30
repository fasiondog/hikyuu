/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-30
 *      Author: fasiondog
 */

#pragma once

#include "PositionRecord.h"

namespace hku {
// 持仓扩展信息（只适合一买一卖的情况，一买多卖的情况，部分统计不准确仅供参考)
struct HKU_API PositionExtInfo final {
    PositionRecord position;
    price_t maxHighPrice{0.};       // 期间最高价最大值
    price_t minLowPrice{0.0};       // 期间最低价最小值
    price_t maxClosePrice{0.};      // 期间收盘价最高值
    price_t minClosePrice{0.0};     // 期间收盘价最低值
    price_t currentClosePrice{0.};  // 当前收盘价
    price_t maxPullBack1{0.};       // 最大回撤百分比1(仅使用最大收盘价和最低收盘价计算)(负数)
    price_t maxPullBack2{0.};   // 最大回撤百分比2(使用期间最高价最大值和最低价最小值计算)（负数）
    price_t currentProfit{0.};  // 当前浮动盈亏(不含预计卖出成本)

    /** 当前回撤百分比1(仅使用最大收盘价和当前收盘价计算) */
    price_t currentPullBack1() const {
        price_t ret = (maxClosePrice - currentClosePrice) / maxClosePrice;
        return ret > 0. ? 0. : ret;
    }

    /** 当前回撤百分比2(使用期间最高价最大值和当前收盘价计算) */
    price_t currentPullBack2() const {
        price_t ret = (maxHighPrice - currentClosePrice) / maxHighPrice;
        return ret > 0. ? 0. : ret;
    }

    /** 期间最大浮盈1 (正数, 仅使用收盘价计算, 不含预计卖出成本, 多次买卖时统计不准) */
    price_t maxFloatingProfit1() const {
        price_t ret = maxClosePrice * position.number + position.sellMoney - position.buyMoney;
        return ret < 0. ? 0. : ret;
    }

    /** 期间最大浮盈2 (正数, 使用最高值最大值进行计算,不含预计卖出成本，多次买卖时统计不准) */
    price_t maxFloatingProfit2() const {
        price_t ret = maxHighPrice * position.number + position.sellMoney - position.buyMoney;
        return ret < 0. ? 0. : ret;
    }

    /** 期间最大浮亏1（负数，仅使用收盘价计算, 不含预计卖出成本，多次买卖时统计不准) */
    price_t minLossProfit1() const {
        price_t ret = minClosePrice * position.number + position.sellMoney - position.buyMoney;
        return ret > 0. ? 0. : ret;
    }

    /** 期间最大浮亏2（负数，仅期间最低价计算, 不含预计卖出成本，多次买卖时统计不准) */
    price_t minLossProfit2() const {
        price_t ret = minLowPrice * position.number + position.sellMoney - position.buyMoney;
        return ret > 0. ? 0. : ret;
    }

    PositionExtInfo() = default;
    PositionExtInfo(const PositionExtInfo&) = default;
    PositionExtInfo& operator=(const PositionExtInfo&) = default;

    PositionExtInfo(PositionExtInfo&& rhs)
    : position(std::move(rhs.position)),
      maxHighPrice(rhs.maxHighPrice),
      minLowPrice(rhs.minLowPrice),
      maxClosePrice(rhs.maxClosePrice),
      minClosePrice(rhs.minClosePrice),
      currentClosePrice(rhs.currentClosePrice),
      maxPullBack1(rhs.maxPullBack1),
      maxPullBack2(rhs.maxPullBack2),
      currentProfit(rhs.currentProfit) {}

    PositionExtInfo& operator=(PositionExtInfo&& rhs) {
        if (this != &rhs) {
            position = std::move(rhs.position);
            maxHighPrice = rhs.maxHighPrice;
            minLowPrice = rhs.minLowPrice;
            maxClosePrice = rhs.maxClosePrice;
            minClosePrice = rhs.minClosePrice;
            currentClosePrice = rhs.currentClosePrice;
            maxPullBack1 = rhs.maxPullBack1;
            maxPullBack2 = rhs.maxPullBack2;
            currentProfit = rhs.currentProfit;
        }
        return *this;
    }
};

}  // namespace hku