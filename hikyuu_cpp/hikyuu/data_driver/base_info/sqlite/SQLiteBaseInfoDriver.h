/*
 * SQLiteBaseInfoDriver.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_DATA_DRIVER_BASE_INFO_SQLITE_SQLITEBASEINFODRIVER_H
#define HIKYUU_DATA_DRIVER_BASE_INFO_SQLITE_SQLITEBASEINFODRIVER_H

#include "../../../utilities/db_connect/sqlite/SQLiteConnect.h"
#include "../../../utilities/ConnectPool.h"
#include "../../BaseInfoDriver.h"

namespace hku {

class SQLiteBaseInfoDriver : public BaseInfoDriver {
public:
    SQLiteBaseInfoDriver();
    virtual ~SQLiteBaseInfoDriver();

    virtual bool _init() override;
    virtual vector<MarketInfo> getAllMarketInfo() override;
    virtual vector<StockTypeInfo> getAllStockTypeInfo() override;

    virtual Parameter getFinanceInfo(const string& market, const string& code) override;
    virtual StockWeightList getStockWeightList(const string& market, const string& code,
                                               Datetime start, Datetime end) override;
    virtual unordered_map<string, StockWeightList> getAllStockWeightList() override;
    virtual MarketInfo getMarketInfo(const string& market) override;
    virtual StockTypeInfo getStockTypeInfo(uint32_t type) override;
    virtual StockInfo getStockInfo(string market, const string& code) override;
    virtual vector<StockInfo> getAllStockInfo() override;
    virtual std::unordered_set<Datetime> getAllHolidays() override;
    virtual ZhBond10List getAllZhBond10() override;

    virtual vector<std::pair<size_t, string>> getHistoryFinanceField() override;
    virtual vector<HistoryFinanceInfo> getHistoryFinance(const string& market, const string& code,
                                                         Datetime start, Datetime end) override;

private:
    // 股票基本信息数据库实例
    ConnectPool<SQLiteConnect>* m_pool;
};

} /* namespace hku */
#endif /* HIKYUU_DATA_DRIVER_BASE_INFO_SQLITE_SQLITEBASEINFODRIVER_H */
