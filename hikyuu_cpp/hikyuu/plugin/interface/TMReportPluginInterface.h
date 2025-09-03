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
#include "hikyuu/trade_manage/PositionExtInfo.h"
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

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
     * @param ktype k线类型
     */
    virtual Performance getExtPerformance(const TMPtr& tm, const Datetime& datetime,
                                          const KQuery::KType& ktype) = 0;

    /**
     * @brief 获取指定截止时间前各月的收益百分比
     * @param tm
     * @param datetime
     * @return std::vector<std::pair<Datetime, double>>
     */
    virtual std::vector<std::pair<Datetime, double>> getProfitPercentMonthly(
      const TMPtr& tm, const Datetime& datetime) = 0;

    /**
     * @brief 获取指定截止时间前各年的收益百分比
     * @param tm
     * @param datetime
     * @return std::vector<std::pair<Datetime, double>>
     */
    virtual std::vector<std::pair<Datetime, double>> getProfitPercentYearly(
      const TMPtr& tm, const Datetime& datetime) = 0;
};

}  // namespace hku
