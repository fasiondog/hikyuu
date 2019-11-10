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

#include "../../../utilities/db_connect/DBConnect.h"
#include "../../BaseInfoDriver.h"

namespace hku {

class SQLiteBaseInfoDriver : public BaseInfoDriver {
public:
    SQLiteBaseInfoDriver();
    virtual ~SQLiteBaseInfoDriver();

    virtual bool _init() override;
    virtual bool _loadMarketInfo() override;
    virtual bool _loadStockTypeInfo() override;
    virtual bool _loadStock() override;
    virtual Parameter getFinanceInfo(const string& market, const string& code) override;

private:
    StockWeightList _getStockWeightList(uint64 stockid);

private:
    //股票基本信息数据库实例
    DBConnectPool* m_pool;
};

} /* namespace hku */
#endif /* HIKYUU_DATA_DRIVER_BASE_INFO_SQLITE_SQLITEBASEINFODRIVER_H */
