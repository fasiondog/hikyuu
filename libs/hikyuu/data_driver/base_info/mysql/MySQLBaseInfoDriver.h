/*
 * MySQLBaseInfoDriver.h
 *
 *  Created on: 2014年8月27日
 *      Author: fasiondog
 */

#ifndef MYSQLBASEINFODRIVER_H_
#define MYSQLBASEINFODRIVER_H_

#include "../../BaseInfoDriver.h"

#if defined(BOOST_WINDOWS)
    #include <mysql.h>
#else
    #include <mysql/mysql.h>
#endif

namespace hku {

class MySQLBaseInfoDriver: public BaseInfoDriver {
public:
    MySQLBaseInfoDriver(): BaseInfoDriver("mysql") {}
    virtual ~MySQLBaseInfoDriver();

    virtual bool _init();
    virtual bool _loadMarketInfo();
    virtual bool _loadStockTypeInfo();
    virtual bool _loadStock();

private:
    bool _getStockWeightList(hku_uint64, StockWeightList&);

private:
    shared_ptr<MYSQL> m_mysql;
};

} /* namespace hku */

#endif /* MYSQLBASEINFODRIVER_H_ */
