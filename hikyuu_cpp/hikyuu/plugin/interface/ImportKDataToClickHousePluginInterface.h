/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-08
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/KRecord.h"
#include "hikyuu/KQuery.h"
#include "hikyuu/TimeLineRecord.h"
#include "hikyuu/TransRecord.h"
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class ImportKDataToClickHousePluginInterface : public PluginBase {
public:
    ImportKDataToClickHousePluginInterface() = default;
    virtual ~ImportKDataToClickHousePluginInterface() = default;

    /**
     * @brief Set the ClickHouse connection and the baseinfo database name
     * @param host ClickHouse host
     * @param port ClickHouse port
     * @param user user name
     * @param pwd password
     * @param baseinfo_db the baseinfo database name (hku_base by default); addMarket / addStockType
     *                    write into this database
     */
    virtual bool setConfig(const string& host, int port, const string& user, const string& pwd,
                           const string& baseinfo_db) = 0;

    virtual Datetime getLastDatetime(const string& market, const string& code,
                                     const KQuery::KType& ktype) = 0;

    virtual void addKRecordList(const string& market, const string& code,
                                const KRecordList& krecords, const KQuery::KType& ktype) = 0;

    virtual void addTimeLineList(const string& market, const string& code,
                                 const TimeLineList& timeline) = 0;

    virtual void addTransList(const string& market, const string& code,
                              const TransRecordList& translist) = 0;

    virtual void updateIndex(const string& market, const string& code,
                             const KQuery::KType& ktype) = 0;

    virtual void remove(const string& market, const string& code, const KQuery::KType& ktype,
                        Datetime start) = 0;

    /**
     * @brief Register a new market into the baseinfo database (idempotent: it is skipped and true
     * is returned if it already exists)
     * @param market market abbreviation (converted to uppercase automatically)
     * @param name market name
     * @param description description (it is recommended to note the time zone)
     * @param index_code the representative index code of the market (getMarketStock / the trading
     *                   calendar depend on {market}{index_code})
     * @param open1 morning opening time HHMM
     * @param close1 morning closing time HHMM
     * @param open2 afternoon opening time HHMM
     * @param close2 afternoon closing time HHMM
     * @return true success or it already exists; false failure
     * @note It takes effect only after hikyuu_init() is restarted following the registration; the
     *       representative index K-lines must be imported separately
     */
    virtual bool addMarket(const string& market, const string& name, const string& description,
                           const string& index_code, uint64_t open1, uint64_t close1,
                           uint64_t open2, uint64_t close2) = 0;

    /**
     * @brief Register a security type into the baseinfo database (idempotent: it is skipped and
     * true is returned if it already exists)
     * @param type_id the type value; id == type is stipulated; only 10 (CRYPTO reuse) or >= 12
     *                (custom) is allowed
     * @param description type description
     * @param precision price precision (decimal places)
     * @param tick minimum tick size
     * @param tick_value value of every tick
     * @param min_trade minimum trade volume per order
     * @param max_trade maximum trade volume per order
     * @return true success or it already exists; false failure
     * @note It takes effect only after hikyuu_init() is restarted following the registration
     */
    virtual bool addStockType(uint32_t type_id, const string& description, uint32_t precision,
                              double tick, double tick_value, double min_trade,
                              double max_trade) = 0;
};

}  // namespace hku
