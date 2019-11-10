/*
 * MarketInfoTable.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_MARKETINFOTABLE_H
#define HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_MARKETINFOTABLE_H

#include "../../../utilities/db_connect/SQLStatementBase.h"

namespace hku {

class MarketInfoTable {
public:
    MarketInfoTable() : m_marketid(0), m_lastDate(0) {}

    uint64 id() const {
        return m_marketid;
    }

    const string& market() const {
        return m_market;
    }

    const string& name() const {
        return m_name;
    }

    const string& description() const {
        return m_description;
    }

    const string& code() const {
        return m_code;
    }

    int64 lastDate() const {
        return m_lastDate;
    }

public:
    static const char* getInsertSQL() {
        return "insert into `Market` "
               "(`marketid`, `market`, `name`, `description`, `code`, `lastDate`) "
               "values (?,?,?,?,?,?)";
    }

    static const char* getUpdateSQL() {
        return "update `Market` set `market`=?, `name`=?, `description`=?, "
               "`code`=?, `lastDate`=? where `marketid`=?";
    }

    static const char* getSelectSQL() {
        return "select `marketid`,`market`,`name`, `description`, `code`, `lastDate` from `Market`";
    }

    void save(const SQLStatementPtr& st) const {
        st->bind(0, m_marketid, m_market, m_name, m_description, m_code, m_lastDate);
    }

    void update(const SQLStatementPtr& st) const {
        st->bind(0, m_market, m_name, m_description, m_code, m_lastDate, m_marketid);
    }

    void load(const SQLStatementPtr& st) {
        st->getColumn(0, m_marketid, m_market, m_name, m_description, m_code, m_lastDate);
    }

private:
    uint64 m_marketid;
    string m_market;
    string m_name;
    string m_description;
    string m_code;
    int64 m_lastDate;
};

}  // namespace hku

#endif /* HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_MARKETINFOTABLE_H */