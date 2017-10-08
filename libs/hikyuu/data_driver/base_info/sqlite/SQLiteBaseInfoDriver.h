/*
 * SQLiteBaseInfoDriver.h
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#ifndef SQLITEBASEINFODRIVER_H_
#define SQLITEBASEINFODRIVER_H_

#include <sqlite3.h>
#include "../../../Log.h"
#include "../../BaseInfoDriver.h"

namespace hku {

class SQLiteBaseInfoDriver: public BaseInfoDriver {
public:
    SQLiteBaseInfoDriver(): BaseInfoDriver("sqlite3") {}
    virtual ~SQLiteBaseInfoDriver() { }

    virtual bool _init();
    virtual bool _loadMarketInfo();
    virtual bool _loadStockTypeInfo();
    virtual bool _loadStock();

private:
    bool _getStockWeightList(hku_uint32, StockWeightList&);

private:
    static int _getMarketTableCallBack(void *out, int nCol,
                                           char **azVals, char **azCols);

    static int _getStockTypeInfoTableCallBack(void *out, int nCol,
                                           char **azVals, char **azCols);

    static int _getStockWeightCallBack(void *out, int nCol,
                                           char **azVals, char **azCols);

    static int _getStockTableCallBack(void *out, int nCol,
                                          char **azVals, char **azCols);

private:
    //股票基本信息数据库实例
    shared_ptr<sqlite3> m_db;

};

} /* namespace hku */
#endif /* SQLITEBASEINFODRIVER_H_ */
