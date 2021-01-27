/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-27
 *     Author: fasiondog
 */

#pragma once

#include "../../../utilities/db_connect/SQLStatementBase.h"

namespace hku {

class StockInfoTable {
public:
    StockInfoTable()
    : m_type(Null<uint32_t>()),
      m_valid(0),
      m_startDate(0),
      m_endDate(0),
      m_precision(1),
      m_tick(0.0),
      m_tickValue(0.0),
      m_minTradeNumber(0.0),
      m_maxTradeNumber(0.0) {}

    static const char* getSelectSQL() {
        return "select c.market, a.code, a.name, a.type, a.valid, a.startDate, a.endDate, b.tick, "
               "b.tickValue, b.precision, b.minTradeNumber, b.maxTradeNumber from stock a, "
               "StockTypeInfo b, market c where a.type = b.id and a.marketid = c.marketid";
    }

    void load(const SQLStatementPtr& st) {
        st->getColumn(0, m_market, m_code, m_name, m_type, m_valid, m_startDate, m_endDate, m_tick,
                      m_tickValue, m_precision, m_minTradeNumber, m_maxTradeNumber);
    }

    const string& market() const {
        return m_market;
    }

    const string& code() const {
        return m_code;
    }

    const string& name() const {
        return m_name;
    }

    uint32_t type() const {
        return m_type;
    }

    bool valid() const {
        return m_valid;
    }

    Datetime startDate() const {
        try {
            return Datetime(m_startDate * 10000LL);
        } catch (...) {
        }
        return Null<Datetime>();
    }

    Datetime endDate() const {
        try {
            return Datetime(m_endDate * 10000LL);
        } catch (...) {
        }
        return Null<Datetime>();
    }

    uint32_t precision() const {
        return m_precision;
    }

    double tick() const {
        return m_tick;
    }

    double tickValude() const {
        return m_tickValue;
    }

    double minTradeNumber() const {
        return m_minTradeNumber;
    }

    double maxTradeNumber() const {
        return m_maxTradeNumber;
    }

private:
    string m_market;
    string m_code;
    string m_name;
    uint32_t m_type;
    uint32_t m_valid;
    uint64_t m_startDate;
    uint64_t m_endDate;
    uint32_t m_precision;
    double m_tick;
    double m_tickValue;
    double m_minTradeNumber;
    double m_maxTradeNumber;
};

}  // namespace hku