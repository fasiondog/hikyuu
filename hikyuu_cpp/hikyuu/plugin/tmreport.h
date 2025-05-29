/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-27
 *      Author: fasiondog
 */

#pragma once

#include "interface/TMReportPluginInterface.h"

namespace hku {

/**
 * @brief 获取指定时刻时账户的最大回撤百分比（负数）（仅根据收盘价计算）
 * @param tm 指定账户
 * @param date 指定日期（包含该时刻）
 * @param ktype k线类型
 * @return price_t
 */
price_t HKU_API getMaxPullBack(const TMPtr& tm, const Datetime& date,
                               const KQuery::KType& ktype = KQuery::DAY);

/**
 * @brief 获取账户最后交易时刻之后指定时间的持仓详情（未平仓记录）
 * @param tm 指定账户
 * @param current_time 当前时刻（需大于等于最后交易时刻）
 * @param ktype k线类型
 * @param trade_mode 交易模式，影响部分统计项: 0-收盘时交易, 1-下一开盘时交易
 * @return std::vector<PositionExtInfo>
 */
std::vector<PositionExtInfo> HKU_API
getPositionExtInfoList(const TMPtr& tm, const Datetime& current_time,
                       const KQuery::KType& ktype = KQuery::DAY, int trade_mode = 0);

/** @brief 获取账户历史持仓扩展详情（已平仓记录）
 * @param tm 指定账户
 * @param ktype k线类型
 * @param trade_mode 交易模式，影响部分统计项: 0-收盘时交易, 1-下一开盘时交易
 * @return std::vector<PositionExtInfo>
 */
std::vector<PositionExtInfo> HKU_API getHistoryPositionExtInfoList(
  const TMPtr& tm, const KQuery::KType& ktype = KQuery::DAY, int trade_mode = 0);

/**
 * 统计截至某一时刻的系统绩效, datetime必须大于等于lastDatetime，
 * 以便用于计算当前市值
 * @param tm 指定的交易管理实例
 * @param datetime 统计截止时刻
 */
Performance HKU_API getExtPerformance(const TMPtr& tm, const Datetime& datetime = Datetime::now(),
                                      const KQuery::KType& ktype = KQuery::DAY);

/**
 * @brief 获取指定截止时间前各月的收益百分比
 * @param tm
 * @param datetime
 * @return std::vector<std::pair<Datetime, double>>
 */
std::vector<std::pair<Datetime, double>> HKU_API getProfitPercentMonthly(const TMPtr& tm,
                                                                         const Datetime& datetime);

/**
 * @brief 获取指定截止时间前各年的收益百分比
 * @param tm
 * @param datetime
 * @return std::vector<std::pair<Datetime, double>>
 */
std::vector<std::pair<Datetime, double>> HKU_API getProfitPercentYearly(const TMPtr& tm,
                                                                        const Datetime& datetime);

}  // namespace hku