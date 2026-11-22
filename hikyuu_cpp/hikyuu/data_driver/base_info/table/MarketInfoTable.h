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

#include <limits>

#include "../../../MarketInfo.h"
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
      m_closeTime2(0),
      m_tzOffsetMin(480),
      m_dstRule(0),
      m_dataLag(0) {}

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
        return MarketInfo::hhmmToTimeDelta(m_openTime1);
    }

    TimeDelta closeTime1() const {
        return MarketInfo::hhmmToTimeDelta(m_closeTime1);
    }

    TimeDelta openTime2() const {
        return MarketInfo::hhmmToTimeDelta(m_openTime2);
    }

    TimeDelta closeTime2() const {
        return MarketInfo::hhmmToTimeDelta(m_closeTime2);
    }

    int tzOffsetMin() const {
        // A NULL column reads as Null<int64_t>, whose int cast would silently become 0 (UTC)
        HKU_CHECK(m_tzOffsetMin >= MarketInfo::MIN_TZ_OFFSET_MIN &&
                    m_tzOffsetMin <= MarketInfo::MAX_TZ_OFFSET_MIN,
                  "Invalid tz_offset_min: {}!", m_tzOffsetMin);
        return static_cast<int>(m_tzOffsetMin);
    }

    int dstRule() const {
        HKU_CHECK(m_dstRule >= 0 && m_dstRule <= 2, "Invalid dst_rule: {}!", m_dstRule);
        return static_cast<int>(m_dstRule);
    }

    int dataLag() const {
        // The upper bound only keeps the value inside an int, the field means trading days
        HKU_CHECK(m_dataLag >= 0 && m_dataLag <= std::numeric_limits<int>::max(),
                  "Invalid data_lag: {}!", m_dataLag);
        return static_cast<int>(m_dataLag);
    }

public:
    static const char* getInsertSQL() {
        return "insert into `market` "
               "(`marketid`, `market`, `name`, `description`, `code`, `lastDate`,"
               " `openTime1`, `closeTime1`, `openTime2`, `closeTime2`,"
               " `tz_offset_min`, `dst_rule`, `data_lag`) "
               "values (?,?,?,?,?,?,?,?,?,?,?,?,?)";
    }

    static const char* getUpdateSQL() {
        return "update `market` set `market`=?, `name`=?, `description`=?, "
               "`code`=?, `lastDate`=?, `openTime1`=?, `closeTime1`=?, "
               "`openTime2`=?, `closeTime2`=?, `tz_offset_min`=?, `dst_rule`=?, "
               "`data_lag`=? where `marketid`=?";
    }

    static const char* getSelectSQL() {
        return "select `marketid`,`market`,`name`, `description`, `code`, `lastDate`, "
               "`openTime1`, `closeTime1`, `openTime2`, `closeTime2`, "
               "`tz_offset_min`, `dst_rule`, `data_lag` from `market`";
    }

    void save(const SQLStatementPtr& st) const {
        st->bind(0, m_marketid, m_market, m_name, m_description, m_code, m_lastDate, m_openTime1,
                 m_closeTime1, m_openTime2, m_closeTime2, m_tzOffsetMin, m_dstRule, m_dataLag);
    }

    void update(const SQLStatementPtr& st) const {
        st->bind(0, m_market, m_name, m_description, m_code, m_lastDate, m_openTime1, m_closeTime1,
                 m_openTime2, m_closeTime2, m_tzOffsetMin, m_dstRule, m_dataLag, m_marketid);
    }

    void load(const SQLStatementPtr& st) {
        st->getColumn(0, m_marketid, m_market, m_name, m_description, m_code, m_lastDate,
                      m_openTime1, m_closeTime1, m_openTime2, m_closeTime2, m_tzOffsetMin,
                      m_dstRule, m_dataLag);
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
    int64_t m_tzOffsetMin;
    int64_t m_dstRule;
    int64_t m_dataLag;
};

}  // namespace hku

#endif /* HIKYUU_DATA_DRIVER_BASE_INFO_TABLE_MARKETINFOTABLE_H */