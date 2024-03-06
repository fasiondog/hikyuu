/*
 * SQLiteKDataDriver.cpp
 *
 *   Created on: 2023年09月14日
 *       Author: yangrq1018
 */

#include <fmt/format.h>
#include <boost/algorithm/string.hpp>
#include "SQLiteKDataDriver.h"
#include "../mysql/KRecordTable.h"

namespace hku {

inline bool isBaseKType(const KQuery::KType& ktype) {
    return (ktype == KQuery::DAY || ktype == KQuery::MIN || ktype == KQuery::MIN5);
}

inline KQuery::KType getBaseKType(const KQuery::KType& ktype) {
    KQuery::KType base_ktype;
    if (ktype == KQuery::WEEK || ktype == KQuery::MONTH || ktype == KQuery::QUARTER ||
        ktype == KQuery::HALFYEAR || ktype == KQuery::YEAR) {
        base_ktype = KQuery::DAY;
    } else if (ktype == KQuery::MIN15 || ktype == KQuery::MIN30 || ktype == KQuery::MIN60 ||
               ktype == KQuery::HOUR2 || ktype == KQuery::HOUR4 || ktype == KQuery::HOUR6 ||
               ktype == KQuery::HOUR12) {
        base_ktype = KQuery::MIN5;
    } else if (ktype == KQuery::MIN3) {
        base_ktype = KQuery::MIN;
    } else {
        HKU_ERROR("Unable to convert ktype {} to a base ktype", ktype);
    }
    return base_ktype;
}

SQLiteKDataDriver::SQLiteKDataDriver() : KDataDriver("sqlite3") {}

SQLiteKDataDriver::~SQLiteKDataDriver() {}

bool SQLiteKDataDriver::_init() {
    HKU_ASSERT_M(m_sqlite_connection_map.empty(), "Maybe repeat initialization!");
    // read param from config
    StringList keys = m_params.getNameList();
    string db_filename;
    m_ifConvert = tryGetParam<bool>("convert", false);
    HKU_DEBUG("SQLiteKDataDriver: m_ifConvert set to {}", m_ifConvert);

    for (auto iter = keys.begin(); iter != keys.end(); ++iter) {
        size_t pos = iter->find("_");
        if (pos == string::npos || pos == 0 || pos == iter->size() - 1)
            continue;

        string exchange = iter->substr(0, pos);
        string ktype = iter->substr(pos + 1);
        to_upper(exchange);
        to_upper(ktype);

        try {
            db_filename = getParam<string>(*iter);
            Parameter connect_param;
            connect_param.set<string>("db", db_filename);
            SQLiteConnectPtr conn(new SQLiteConnect(connect_param));

            if (ktype == KQuery::getKTypeName(KQuery::DAY)) {
                m_sqlite_connection_map[exchange + "_DAY"] = conn;
                if (m_ifConvert) {
                    m_sqlite_connection_map[exchange + "_WEEK"] = conn;
                    m_sqlite_connection_map[exchange + "_MONTH"] = conn;
                    m_sqlite_connection_map[exchange + "_QUARTER"] = conn;
                    m_sqlite_connection_map[exchange + "_HALFYEAR"] = conn;
                    m_sqlite_connection_map[exchange + "_YEAR"] = conn;
                }
            } else if (ktype == KQuery::getKTypeName(KQuery::MIN)) {
                m_sqlite_connection_map[exchange + "_MIN"] = conn;
            } else if (ktype == KQuery::getKTypeName(KQuery::MIN5)) {
                m_sqlite_connection_map[exchange + "_MIN5"] = conn;
                if (m_ifConvert) {
                    m_sqlite_connection_map[exchange + "_MIN15"] = conn;
                    m_sqlite_connection_map[exchange + "_MIN30"] = conn;
                    m_sqlite_connection_map[exchange + "_MIN60"] = conn;
                    m_sqlite_connection_map[exchange + "_HOUR2"] = conn;
                }
            }
        } catch (...) {
            HKU_ERROR("Can't open sqlite file: {}", db_filename);
        }
    }

    return true;
}

string SQLiteKDataDriver::_getTableName(const string&, const string& code, KQuery::KType) {
    string table = fmt::format("`{}`", code);
    to_lower(table);
    return table;
}

KRecordList SQLiteKDataDriver::getKRecordList(const string& market, const string& code,
                                              const KQuery& query) {
    KRecordList result;
    KQuery::KType ktype = query.kType();
    if (query.queryType() == KQuery::INDEX) {
        if (!isBaseKType(ktype)) {
            KQuery::KType base_ktype = getBaseKType(ktype);
            int64_t start, end, num;
            start = query.start();
            end = query.end();
            num = end - start;
            int32_t multiplier = KQuery::getKTypeInMin(ktype) / KQuery::getKTypeInMin(base_ktype);
            end = getCount(market, code, base_ktype);
            start = end - num * multiplier;
            HKU_ERROR_IF(start < 0, "Invalid start index: {}", start);
            result = _getKRecordList(market, code, ktype, start, end);
        } else {
            result = _getKRecordList(market, code, ktype, query.start(), query.end());
        }
    } else {
        result = _getKRecordList(market, code, ktype, query.startDatetime(), query.endDatetime());
    }
    if (isBaseKType(ktype))
        return result;
    HKU_ERROR_IF_RETURN(!m_ifConvert, KRecordList(), "KData: unsupported ktype {}", ktype);
    KQuery::KType base_ktype = getBaseKType(ktype);
    return convertToNewInterval(result, base_ktype, ktype);
}

KRecordList SQLiteKDataDriver::_getKRecordList(const string& market, const string& code,
                                               const KQuery::KType& kType, size_t start_ix,
                                               size_t end_ix) {
    KRecordList result;
    HKU_IF_RETURN(start_ix >= end_ix, result);
    string key(format("{}_{}", market, kType));
    SQLiteConnectPtr connection = m_sqlite_connection_map[key];
    HKU_IF_RETURN(!connection, result);

    try {
        KRecordTable r(market, code, kType);
        SQLStatementPtr st = connection->getStatement(fmt::format(
          "{} order by date limit {}, {}", r.getSelectSQLNoDB(), start_ix, end_ix - start_ix));

        st->exec();
        while (st->moveNext()) {
            KRecordTable record;
            try {
                record.load(st);
                KRecord k;
                k.datetime = record.date();
                k.openPrice = record.open();
                k.highPrice = record.high();
                k.lowPrice = record.low();
                k.closePrice = record.close();
                k.transAmount = record.amount();
                k.transCount = record.count();
                result.push_back(k);
            } catch (...) {
                HKU_ERROR("Failed get record: {}", record.str());
            }
        }
    } catch (...) {
        // 表可能不存在
        HKU_ERROR("Failed to get record by index: {}", key);
    }
    return result;
}
KRecordList SQLiteKDataDriver::_getKRecordList(const string& market, const string& code,
                                               const KQuery::KType& kType, Datetime start_date,
                                               Datetime end_date) {
    KRecordList result;
    HKU_IF_RETURN(start_date >= end_date, result);

    string key(format("{}_{}", market, kType));
    SQLiteConnectPtr connection = m_sqlite_connection_map[key];
    HKU_IF_RETURN(!connection, result);

    try {
        KRecordTable r(market, code, kType);
        SQLStatementPtr st = connection->getStatement(
          fmt::format("{} where date >= {} and date < {} order by date", r.getSelectSQLNoDB(),
                      start_date.number(), end_date.number()));
        st->exec();
        while (st->moveNext()) {
            KRecordTable record;
            try {
                record.load(st);
                KRecord k;
                k.datetime = record.date();
                k.openPrice = record.open();
                k.highPrice = record.high();
                k.lowPrice = record.low();
                k.closePrice = record.close();
                k.transAmount = record.amount();
                k.transCount = record.count();
                result.push_back(k);
            } catch (...) {
                HKU_ERROR("Failed get record: {}", record.str());
            }
        }
    } catch (...) {
        // 表可能不存在
        HKU_ERROR("Failed to get record by date: {}", key);
    }
    return result;
}

size_t SQLiteKDataDriver::getCount(const string& market, const string& code,
                                   const KQuery::KType& kType) {
    string key(format("{}_{}", market, kType));
    SQLiteConnectPtr connection = m_sqlite_connection_map[key];
    HKU_IF_RETURN(!connection, 0);

    size_t result = 0;
    result = connection->queryInt(
      fmt::format("select count(1) from {}", _getTableName(market, code, kType)), 0);

    if (isBaseKType(kType))
        return result;
    HKU_ERROR_IF_RETURN(!m_ifConvert, 0, "KData: unsupported ktype {}", kType);
    auto old_intervals_per_new_candle =
      KQuery::getKTypeInMin(kType) / KQuery::getKTypeInMin(getBaseKType(kType));
    return result / old_intervals_per_new_candle;
}

bool SQLiteKDataDriver::getIndexRangeByDate(const string& market, const string& code,
                                            const KQuery& query, size_t& out_start,
                                            size_t& out_end) {
    out_start = 0;
    out_end = 0;
    HKU_ERROR_IF_RETURN(query.queryType() != KQuery::DATE, false, "queryType must be KQuery::DATE");
    HKU_IF_RETURN(
      query.startDatetime() >= query.endDatetime() || query.startDatetime() > (Datetime::max)(),
      false);
    string key(format("{}_{}", market, query.kType()));
    SQLiteConnectPtr connection = m_sqlite_connection_map[key];
    HKU_IF_RETURN(!connection, false);

    string tablename = _getTableName(market, code, query.kType());
    try {
        out_start = connection->queryInt(fmt::format("select count(1) from {} where date<{}",
                                                     tablename, query.startDatetime().number()),
                                         0);
        out_end = connection->queryInt(fmt::format("select count(1) from {} where date<{}",
                                                   tablename, query.endDatetime().number()),
                                       0);
    } catch (...) {
        // 表可能不存在, 不打印异常信息
        out_start = 0;
        out_end = 0;
        return false;
    }

    return true;
}

KRecordList SQLiteKDataDriver::convertToNewInterval(const KRecordList& candles,
                                                    const KQuery::KType& from_ktype,
                                                    const KQuery::KType& to_ktype) {
    int32_t old_intervals_per_new_candle =
      KQuery::getKTypeInMin(to_ktype) / KQuery::getKTypeInMin(from_ktype);
    KRecordList result(candles.size() / old_intervals_per_new_candle);
    if (result.size() == 0)
        return result;

    int32_t target = 0;
    for (size_t x = 0, total = candles.size(); x < total; ++x) {
        if (candles[x].openPrice != 0 && candles[x].highPrice != 0 && candles[x].lowPrice != 0 &&
            candles[x].closePrice != 0) {
            if (result[target].datetime.isNull())
                result[target].datetime = candles[x].datetime;
            if (result[target].openPrice == 0)
                result[target].openPrice = candles[x].openPrice;
            if (candles[x].highPrice > result[target].highPrice)
                result[target].highPrice = candles[x].highPrice;
            if ((result[target].lowPrice == 0) || (candles[x].lowPrice < result[target].lowPrice))
                result[target].lowPrice = candles[x].lowPrice;

            result[target].transCount += candles[x].transCount;
            result[target].transAmount += candles[x].transAmount;
            result[target].closePrice = candles[x].closePrice;
        }

        if ((x + 1) % old_intervals_per_new_candle == 0) {
            if ((total - x) < old_intervals_per_new_candle + 1)
                break;
            target++;
        }
    }

    return result;
}

}  // namespace hku