/*
 * sqlite3callback.h
 *
 *  Created on: 2010-11-2
 *      Author: fasiondog
 */

#ifndef SQLITE3CALLBACK_H_
#define SQLITE3CALLBACK_H_

#include <string>
#include <vector>
#include <list>

/******************************************************************************
 *           以下为Sqlite3回调函数，及其使用的相关数据结构
 *****************************************************************************/
int callback_get_marketid(void *marketid, int nCol, char **azVals, char **azCols);
int callback_get_market_stock(void *code, int nCol, char **azVals, char **azCols);
int callback_get_codeprefix(void *codeTree, int nCol, char **azVals, char **azCols);
int callback_get_stockid(void *stockid, int nCol, char **azVals, char **azCols);
int callback_get_datemap_of_stockweight(void *output, int nCol, char **azVals, char **azCols);
int callback_get_stock_type(void *stktype, int nCol, char **azVals, char **azCols);


struct SimpleStockRecord {
    unsigned int stockid;
    unsigned int marketid;
    std::string code;
};

struct Output_callback_import_stock_name {
    std::vector<SimpleStockRecord> sh_stock_list;
    std::vector<SimpleStockRecord> sz_stock_list;
};

int callback_import_stock_name(void *output, int nCol, char **azVals, char **azCols);

struct StockInfo {
    unsigned int valid;
    unsigned int startDate;
    unsigned int endDate;

};
int callback_get_stock_info(void *output, int nCol, char **azVals, char **azCols);

struct StockCode {
    unsigned int stockid;
    std::string code;
};
int callback_get_stockcode_list(void *output, int nCol, char **azVals, char **azCols);

struct StockInfoByMarket {
    unsigned int stockid;
    std::string code;
    std::string name;
    unsigned int valid;
};
int callback_get_stock_info_by_market(void *output, int nCol, char **azVals, char **azCols);


#endif /* SQLITE3CALLBACK_H_ */
