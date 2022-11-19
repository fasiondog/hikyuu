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
    MarketInfoTable()
    : m_marketid(0),
      m_lastDate(0),
      m_openTime1(0),
      m_closeTime1(0),
      m_openTime2(0),
      m_closeTime2(0) {}

    uint64_t id() const {
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

    Datetime lastDate() const {
        HKU_CHECK(m_lastDate <= 99999999, "Invalid lastDate: {}!", m_lastDate);
        return Datetime(m_lastDate * 10000LL);
    }

    TimeDelta openTime1() const {
        return _transTimeDelta(m_openTime1);
    }

    TimeDelta closeTime1() const {
        return _transTimeDelta(m_closeTime1);
    }

    TimeDelta openTime2() const {
        return _transTimeDelta(m_openTime2);
    }

    TimeDelta closeTime2() const {
        return _transTimeDelta(m_closeTime2);
    }

private:
    TimeDelta _transTimeDelta(uint64_t time) const {
        int64_t hours = time / 100;
        HKU_CHECK(hours >= 0 && hours <= 23, "Invalid time: {}!", time);
        int64_t mins = time - hours * 100;
        HKU_CHECK(mins >= 0 && mins <= 59, "Invalid time: {}!", time);
        return TimeDelta(0, hours, mins);
    }

public:
    static const char* getInsertSQL() {
        return "insert into `market` "
               "(`marketid`, `market`, `name`, `description`, `code`, `lastDate`,"
               " `openTime1`, `closeTime1`, `openTime2`, `closeTime2`) "
               "values (?,?,?,?,?,?,?,?,?,?)";
    }

    static const char* getUpdateSQL() {
        return "update `market` set `market`=?, `name`=?, `description`=?, "
               "`code`=?, `lastDate`=?, `openTime1`=?, `closeTime1`=?, "
               "`openTime2=`=?, `closeTime2`=? where `marketid`=?";
    }

    static const char* getSelectSQL() {
        return "select `marketid`,`market`,`name`, `description`, `code`, `lastDate`, "
               "`openTime1`, `closeTime1`, `openTime2`, `closeTime2` from `market`";
    }

    void save(const SQLStatementPtr& st) const {
        st->bind(0, m_marketid, m_market, m_name, m_description, m_code, m_lastDate, m_openTime1,
                 m_closeTime1, m_openTime2, m_closeTime2);
    }

    void update(const SQLStatementPtr& st) const {
        st->bind(0, m_market, m_name, m_description, m_code, m_lastDate, m_openTime1, m_closeTime1,
                 m_openTime2, m_closeTime2, m_marketid);
    }

    void load(const SQLStatementPtr& st) {
        st->getColumn(0, m_marketid, m_market, m_name, m_description, m_code, m_lastDate,
                      m_openTime1, m_closeTime1, m_openTime2, m_closeTime2);
    }

private:
    uint64_t m_marketid;
    string m_market;
    string m_name;
    string m_description;
    string m_code;
    uint64_t m_lastDate;
    uint64_t m_openTime1;
    uint64_t m_closeTime1;
    uint64_t m_openTime2;
    uint64_t m_closeTime2;
};

}  // namespace hku

#endif /* HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_MARKETINFOTABLE_H */