/*
 * MySQLBaseInfoDriver.h
 *
 *  Created on: 2014年8月27日
 *      Author: fasiondog
 */

#pragma once
#ifndef MYSQLBASEINFODRIVER_H_
#define MYSQLBASEINFODRIVER_H_

#include "../../../utilities/ConnectPool.h"
#include "../../../utilities/db_connect/mysql/MySQLConnect.h"
#include "../../BaseInfoDriver.h"

#if defined(_MSC_VER)
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

namespace hku {

class MySQLBaseInfoDriver : public BaseInfoDriver {
public:
    MySQLBaseInfoDriver();
    virtual ~MySQLBaseInfoDriver();

    virtual bool _init() override;
    virtual vector<StockInfo> getAllStockInfo() override;
    virtual vector<MarketInfo> getAllMarketInfo() override;
    virtual vector<StockTypeInfo> getAllStockTypeInfo() override;

    virtual Parameter getFinanceInfo(const string& market, const string& code) override;
    virtual StockWeightList getStockWeightList(const string& market, const string& code,
                                               Datetime start, Datetime end) override;
    virtual unordered_map<string, StockWeightList> getAllStockWeightList() override;
    virtual MarketInfo getMarketInfo(const string& market) override;
    virtual StockTypeInfo getStockTypeInfo(uint32_t type) override;
    virtual StockInfo getStockInfo(string market, const string& code) override;
    virtual std::unordered_set<Datetime> getAllHolidays() override;
    virtual ZhBond10List getAllZhBond10() override;

private:
    ConnectPool<MySQLConnect>* m_pool;
};

} /* namespace hku */

#endif /* MYSQLBASEINFODRIVER_H_ */
