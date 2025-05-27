/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-27
 *      Author: fasiondog
 */

#pragma once

#include <vector>
#include "hikyuu/KQuery.h"
#include "hikyuu/trade_manage/TradeManagerBase.h"
#include "hikyuu/trade_manage/Performance.h"
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

// 持仓扩展信息（只适合一买一卖的情况，一买多卖的情况，部分统计不准确仅供参考)
struct PositionExtInfo final {
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

class TMReportPluginInterface : public PluginBase {
public:
    TMReportPluginInterface() = default;
    virtual ~TMReportPluginInterface() = default;

    // 获取账户截止至指定时刻的最大回撤百分比（包含该时刻）（仅根据收盘价计算）
    virtual price_t getMaxPullBack(const TMPtr& tm, const Datetime& date,
                                   const KQuery::KType& ktype) = 0;

    /**
     * @brief 获取账户历史持仓扩展详情
     * @param tm 指定账户
     * @param ktype k线类型
     * @param trade_mode 交易模式，影响部分统计项: 0-收盘时交易, 1-下一开盘时交易
     * @return std::vector<PositionExtInfo>
     */
    virtual std::vector<PositionExtInfo> getHistoryPositionExtInfoList(const TMPtr& tm,
                                                                       const KQuery::KType& ktype,
                                                                       int trade_mode) = 0;

    /**
     * @brief 获取账户最后交易时刻后持仓详情
     * @param tm 指定账户
     * @param current_time 当前时刻（需大于等于最后交易时刻）
     * @param ktype k线类型
     * @param trade_mode 交易模式，影响部分统计项: 0-收盘时交易, 1-下一开盘时交易
     * @return std::vector<PositionExtInfo>
     */
    virtual std::vector<PositionExtInfo> getPositionExtInfoList(const TMPtr& tm,
                                                                const Datetime& current_time,
                                                                const KQuery::KType& ktype,
                                                                int trade_mode) = 0;

    /**
     * 统计截至某一时刻的系统绩效, datetime必须大于等于lastDatetime，
     * 以便用于计算当前市值
     * @param tm 指定的交易管理实例
     * @param datetime 统计截止时刻
     */
    virtual Performance getExtPerformance(const TMPtr& tm, const Datetime& datetime,
                                          const KQuery::KType& ktype) = 0;
};

}  // namespace hku
