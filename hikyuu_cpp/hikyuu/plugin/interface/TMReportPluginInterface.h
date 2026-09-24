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

    // Get the maximum drawdown percentage of the account up to the given moment (inclusive), it is
    // calculated from the close price only
    virtual price_t getMaxPullBack(const TMPtr& tm, const Datetime& date,
                                   const KQuery::KType& ktype) = 0;

    /**
     * @brief Get the extended detail of the historical positions of the account
     * @param tm the given account
     * @param ktype K-line type
     * @param trade_mode trade mode, it affects some statistics items: 0-trade at the close, 1-trade
     *                   at the next open
     * @return std::vector<PositionExtInfo>
     */
    virtual std::vector<PositionExtInfo> getHistoryPositionExtInfoList(const TMPtr& tm,
                                                                       const KQuery::KType& ktype,
                                                                       int trade_mode) = 0;

    /**
     * @brief Get the detail of the positions after the last trade moment of the account
     * @param tm the given account
     * @param current_time the current moment (it should be greater than or equal to the last trade
     *                     moment)
     * @param ktype K-line type
     * @param trade_mode trade mode, it affects some statistics items: 0-trade at the close, 1-trade
     *                   at the next open
     * @return std::vector<PositionExtInfo>
     */
    virtual std::vector<PositionExtInfo> getPositionExtInfoList(const TMPtr& tm,
                                                                const Datetime& current_time,
                                                                const KQuery::KType& ktype,
                                                                int trade_mode) = 0;

    /**
     * @brief Get the position detail of the given security of the account at the given moment
     * @param tm account
     * @param stock the held stock
     * @param current_time the current moment (it should be greater than or equal to the last trade
     *                     moment)
     * @param ktype K-line type
     * @param trade_mode trade mode, it affects some statistics items: 0-trade at the close, 1-trade
     *                   at the next open
     * @return PositionExtInfo
     */
    virtual PositionExtInfo getPositionExtInfo(const TMPtr& tm, const Stock& stock,
                                               const Datetime& current_time,
                                               const KQuery::KType& ktype, int trade_mode) = 0;

    /**
     * Count the system performance up to a certain moment; datetime must be greater than or equal
     * to lastDatetime so that it can be used to calculate the current market value
     * @param tm the given trade management instance
     * @param datetime the statistics end moment
     */
    virtual Performance getExtPerformance(const TMPtr& tm, const Datetime& datetime,
                                          const KQuery::KType& ktype) = 0;

    /**
     * @brief Get the monthly return percentages before the given end time
     * @param tm
     * @param datetime
     * @return std::vector<std::pair<Datetime, double>>
     */
    virtual std::vector<std::pair<Datetime, double>> getProfitPercentMonthly(
      const TMPtr& tm, const Datetime& datetime) = 0;

    /**
     * @brief Get the yearly return percentages before the given end time
     * @param tm
     * @param datetime
     * @return std::vector<std::pair<Datetime, double>>
     */
    virtual std::vector<std::pair<Datetime, double>> getProfitPercentYearly(
      const TMPtr& tm, const Datetime& datetime) = 0;
};

}  // namespace hku
