/*
 * StockTable.h
 *
 *  Copyright (c) 2019 fasiondog
 * 
 *  Created on: 2019-8-14
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_STOCKTABLE_H
#define HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_STOCKTABLE_H

#include "../../db_connect/SQLStatementBase.h"

namespace hku {

class StockTable {
public:
    StockTable(): stockid(0), marketid(0), type(0), valid(0), startDate(0), endDate(0) {}

public:
    static const char * getInsertSQL() {
        return "insert into `Stock` "
               "(`marketid`, `code`, `name`, `type`, `valid`, `startDate`, `endDate`) "
               "values (?,?,?,?,?,?,?)";
    }

    static const char * getUpdateSQL() {
        return "update `Stock` set `marketid`=?, `code`=?, `name`=?, `type`=?, "
               "`valid`=?, `startDate`=?, `endDate`=?, `lastDate`=? where `stockid`=?";
    }

    static const char * getSelectSQL() {
        return "select `stockid`,`marketid`, `code`, `name`, `type`, `valid`, "
               "`startDate`, `endDate` from `Stock`";
    }

    void save(const SQLStatementPtr& st) const {
        st->bind(0, marketid, code, name, type, valid, startDate, endDate);
    }

    void update(const SQLStatementPtr& st) const {
        st->bind(0, marketid, code, name, type, valid, startDate, endDate, stockid);
    }

    void load(const SQLStatementPtr& st) {
        st->getColumn(0, stockid, marketid, code, name, type, valid, startDate, endDate);
    }

public:
    uint64 stockid;
    uint64 marketid;
    string code;
    string name;
    uint32 type;
    uint32 valid;
    uint64 startDate;
    uint64 endDate;
};

} /* namespace */

#endif /* HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_STOCKTABLE_H */