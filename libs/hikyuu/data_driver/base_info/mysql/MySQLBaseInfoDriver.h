/*
 * MySQLBaseInfoDriver.h
 *
 *  Created on: 2014年8月27日
 *      Author: fasiondog
 */

#ifndef MYSQLBASEINFODRIVER_H_
#define MYSQLBASEINFODRIVER_H_

#include <mysql.h>
#include "../../BaseInfoDriver.h"

namespace hku {

class MySQLBaseInfoDriver: public BaseInfoDriver {
public:
    MySQLBaseInfoDriver(const shared_ptr<IniParser>& config);
    virtual ~MySQLBaseInfoDriver();

    virtual bool loadMarketInfo(MarketInfoMap& out);
    virtual bool loadStockTypeInfo(StockTypeInfoMap& out);
    virtual bool loadStock();

private:
    bool _getStockWeightList(hku_uint64, StockWeightList&);

private:
    shared_ptr<MYSQL> m_mysql;
};

} /* namespace hku */

#endif /* MYSQLBASEINFODRIVER_H_ */
