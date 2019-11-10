/*
 * MySQLBaseInfoDriver.h
 *
 *  Created on: 2014年8月27日
 *      Author: fasiondog
 */

#pragma once
#ifndef MYSQLBASEINFODRIVER_H_
#define MYSQLBASEINFODRIVER_H_

#include "../../BaseInfoDriver.h"

#if defined(_MSC_VER)
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

namespace hku {

class MySQLBaseInfoDriver : public BaseInfoDriver {
public:
    MySQLBaseInfoDriver() : BaseInfoDriver("mysql") {}
    virtual ~MySQLBaseInfoDriver();

    virtual bool _init() override;
    virtual bool _loadMarketInfo() override;
    virtual bool _loadStockTypeInfo() override;
    virtual bool _loadStock() override;

private:
    bool _getStockWeightList(uint64, StockWeightList&);

private:
    shared_ptr<MYSQL> m_mysql;
};

} /* namespace hku */

#endif /* MYSQLBASEINFODRIVER_H_ */
