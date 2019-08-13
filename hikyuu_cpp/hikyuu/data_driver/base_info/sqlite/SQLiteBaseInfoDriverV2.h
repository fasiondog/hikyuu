/*
 * SQLiteBaseInfoDriver.h
 * 
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#pragma once
#ifndef SQLITEBASEINFODRIVERV2_H_
#define SQLITEBASEINFODRIVERV2_H_

#include "../../db_connect/DBConnect.h"
#include "../../BaseInfoDriver.h"

namespace hku {

class SQLiteBaseInfoDriverV2: public BaseInfoDriver {
public:
    SQLiteBaseInfoDriverV2();
    virtual ~SQLiteBaseInfoDriverV2();

    virtual bool _init();
    virtual bool _loadMarketInfo();
    virtual bool _loadStockTypeInfo();
    virtual bool _loadStock();

private:
    StockWeightList _getStockWeightList(uint64 stockid);

private:
    //股票基本信息数据库实例
    DBConnectPool *m_pool;
};

} /* namespace hku */
#endif /* SQLITEBASEINFODRIVER_H_ */
