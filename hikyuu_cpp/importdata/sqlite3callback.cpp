/*
 * sqlite3callback.cpp
 *
 *  Created on: 2010-11-2
 *      Author: fasiondog
 */

#include <cassert>
#include <iostream>
#include <map>
#include <sqlite3.h>
#include <boost/lexical_cast.hpp>
#include "CodeTree.h"
#include "sqlite3callback.h"

/******************************************************************************
 *           以下为Sqlite3回调函数，及其使用的相关数据结构
 *****************************************************************************/

//select marketid from market where market='SH'
int callback_get_marketid(void *marketid, int nCol, char **azVals, char **azCols) {
    assert(nCol==1);
    int result = 0;
    try{
        (*(unsigned int *)marketid) = boost::lexical_cast<unsigned int>(azVals[0]);
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        std::cerr << "[callback_get_marketid] bad_lexical_cast!" << std::endl
                  << e.what() << std::endl;
        result = 1;
    }catch(...){
        std::cerr << "[callback_get_marketid] Some error!" << std::endl;
        result = 1;
    }
    return result;
}

//select code from market where market=?
int callback_get_market_stock(void *code, int nCol, char **azVals, char **azCols) {
    assert(nCol==1);
    int result = 0;
    try{
        (*(std::string *)code) = boost::lexical_cast<std::string>(azVals[0]);
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        std::cerr << "[callback_get_market_stock] bad_lexical_cast!" << std::endl
                  << e.what() << std::endl;
        result = 1;
    }catch(...){
        std::cerr << "[callback_get_market_stock] Some error!" << std::endl;
        result = 1;
    }
    return result;
}

//select codepre, type from CodeRuleType where marketid=?
int callback_get_codeprefix(void *codeTree, int nCol, char **azVals, char **azCols) {
    assert(nCol==2);
    int result = 0;
    try{
        std::string codeprefix(azVals[0]);
        unsigned int stocktype = boost::lexical_cast<unsigned int>(azVals[1]);
        (*(CodeTreePtr *)codeTree)->addCode(codeprefix, stocktype);
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        std::cerr << "[callback_get_codeprefix] bad_lexical_cast!" << std::endl
                  << e.what() << std::endl;
        result = 1;
    }catch(...){
        std::cerr << "[callback_get_codeprefix] Some error!" << std::endl;
        result = 1;
    }
    return result;
}

//select stockid from stock where marketid=? and code='???'
int callback_get_stockid(void *stockid, int nCol, char **azVals, char **azCols) {
    assert(nCol==1);
    int result = 0;
    try{
        (*(unsigned int *)stockid) = boost::lexical_cast<unsigned int>(azVals[0]);
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        std::cerr << "[callback_get_stockid] bad_lexical_cast!" << std::endl
                  << e.what() << std::endl;
        result = 1;
    }catch(...){
        std::cerr << "[callback_get_stockid] Some error!" << std::endl;
        result = 1;
    }
    return result;
}

//select type from stock where marketid=? and code='???'
int callback_get_stock_type(void *stktype, int nCol, char **azVals, char **azCols) {
    assert(nCol==1);
    int result = 0;
    try{
        (*(unsigned int *)stktype) = boost::lexical_cast<unsigned int>(azVals[0]);
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        std::cerr << "[callback_get_stock_type] bad_lexical_cast!" << std::endl
                  << e.what() << std::endl;
        result = 1;
    }catch(...){
        std::cerr << "[callback_get_stock_type] Some error!" << std::endl;
        result = 1;
    }
    return result;
}

//select stockid, marketid, code from stock
int callback_import_stock_name(void *output, int nCol, char **azVals, char **azCols) {
    assert(nCol == 3);
    int result = 0;
    try{
        SimpleStockRecord record;
        record.stockid = boost::lexical_cast<unsigned int>(azVals[0]);
        record.marketid = boost::lexical_cast<unsigned int>(azVals[1]);
        record.code = std::string(azVals[2]);
        Output_callback_import_stock_name *p = (Output_callback_import_stock_name *)output;
        if (record.marketid == 1) {
            p->sh_stock_list.push_back(record);
        } else if (record.marketid == 2) {
            p->sz_stock_list.push_back(record);
        } else {
            ; // do nothin;
        }
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        std::cerr << "[callback_import_stock_name] bad_lexical_cast!" << std::endl
                  << e.what() << std::endl;
        result = 1;
    }catch(...){
        std::cerr << "[callback_import_stock_name] Some error!" << std::endl;
        result = 1;
    }
    return result;
}

//select date from stkweight where stockid=%s
int callback_get_datemap_of_stockweight(void *output, int nCol, char **azVals, char **azCols) {
    assert(nCol==1);
    int result = 0;
    try{
        unsigned int date = boost::lexical_cast<unsigned int>(azVals[0]);
        (*(std::map<unsigned int, int> *)output)[date] = 0;
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        std::cerr << "[callback_get_datemap_of_stockweight] bad_lexical_cast!" << std::endl
                  << e.what() << std::endl;
        result = 1;
    }catch(...){
        std::cerr << "[callback_get_datemap_of_stockweight] Some error!" << std::endl;
        result = 1;
    }
    return result;
}

//select valid, startDate, endDate from stock where stockid=?
int callback_get_stock_info(void *output, int nCol, char **azVals, char **azCols) {
    assert(nCol==3);
    int result = 0;
    StockInfo info;
    try{
        info.valid = boost::lexical_cast<unsigned int>(azVals[0]);
        info.startDate = boost::lexical_cast<unsigned int>(azVals[1]);
        info.endDate = boost::lexical_cast<unsigned int>(azVals[2]);
        (*(StockInfo *)output) = info;
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        std::cerr << "[callback_get_stock_info] bad_lexical_cast!" << std::endl
                  << e.what() << std::endl;
        result = 1;
    }catch(...){
        std::cerr << "[callback_get_stock_info] Some error!" << std::endl;
        result = 1;
    }
    return result;
}

//select stockid, code from stock where marketid=?
int callback_get_stockcode_list(void *output, int nCol, char **azVals, char **azCols) {
    assert(nCol==2);
    int result = 0;
    try{
        StockCode stock_code;
        stock_code.stockid = boost::lexical_cast<unsigned int>(azVals[0]);
        stock_code.code = std::string(azVals[1]);
        std::list<StockCode> *p = (std::list<StockCode> *)output;
        p->push_back(stock_code);
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        std::cerr << "[callback_get_stockcode_list] bad_lexical_cast!" << std::endl
                  << e.what() << std::endl;
        result = 1;
    }catch(...){
        std::cerr << "[callback_get_stockcode_list] Some error!" << std::endl;
        result = 1;
    }
    return result;
}

//select stockid, code, name, valid from stock where marketid = ?"
int callback_get_stock_info_by_market(void *output, int nCol, char **azVals, char **azCols) {
    assert(nCol==3);
    int result = 0;
    StockInfoByMarket info;
    try{
        info.stockid = boost::lexical_cast<unsigned int>(azVals[0]);
        info.code = std::string(azVals[1]);
        info.name = std::string(azVals[2]);
        info.valid = boost::lexical_cast<unsigned int>(azVals[3]);
        std::list<StockInfoByMarket> *p = (std::list<StockInfoByMarket> *)output;
        p->push_back(info);
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        std::cerr << "[callback_get_stock_info_by_market] bad_lexical_cast!" << std::endl
                  << e.what() << std::endl;
        result = 1;
    }catch(...){
        std::cerr << "[callback_get_stock_info_by_market] Some error!" << std::endl;
        result = 1;
    }
    return result;
}
