/*
 * SQLiteKDataDriver.cpp
 *
 *   Created on: 2023-09-14
 *       Author: yangrq1018
 */

#include <algorithm>
#include <utility>

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

// Session phase end times (HHMM) of the A-share market, consistent with the data import pipeline.
// Minutes bars are timestamped at the interval end, so a bar is grouped into the first boundary
// that is not earlier than the bar time.
const int* getMinuteEndBoundaries(const KQuery::KType& ktype, size_t& count) {
    static const int min15_ends[] = {945,  1000, 1015, 1030, 1045, 1100, 1115, 1130,
                                     1315, 1330, 1345, 1400, 1415, 1430, 1445, 1500};
    static const int min30_ends[] = {1000, 1030, 1100, 1130, 1330, 1400, 1430, 1500};
    static const int min60_ends[] = {1030, 1130, 1400, 1500};
    static const int hour2_ends[] = {1130, 1500};
    if (ktype == KQuery::MIN15) {
        count = sizeof(min15_ends) / sizeof(int);
        return min15_ends;
    }
    if (ktype == KQuery::MIN30) {
        count = sizeof(min30_ends) / sizeof(int);
        return min30_ends;
    }
    if (ktype == KQuery::MIN60) {
        count = sizeof(min60_ends) / sizeof(int);
        return min60_ends;
    }
    if (ktype == KQuery::HOUR2) {
        count = sizeof(hour2_ends) / sizeof(int);
        return hour2_ends;
    }
    count = 0;
    return nullptr;
}

// Start of the natural calendar phase (week/month/...) containing the given daily bar.
bool getDailyPhaseStart(const KQuery::KType& ktype, const Datetime& d, Datetime& out) {
    if (ktype == KQuery::WEEK)
        out = d.startOfWeek();
    else if (ktype == KQuery::MONTH)
        out = d.startOfMonth();
    else if (ktype == KQuery::QUARTER)
        out = d.startOfQuarter();
    else if (ktype == KQuery::HALFYEAR)
        out = d.startOfHalfyear();
    else if (ktype == KQuery::YEAR)
        out = d.startOfYear();
    else
        return false;
    return true;
}

SQLiteKDataDriver::SQLiteKDataDriver() : KDataDriver("sqlite3") {}

SQLiteKDataDriver::~SQLiteKDataDriver() {}

bool SQLiteKDataDriver::_init() {
    HKU_CHECK(m_sqlite_connection_map.empty(), "Maybe repeat initialization!");
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
    KQuery::KType ktype = query.kType();
    if (isBaseKType(ktype)) {
        if (query.queryType() == KQuery::INDEX) {
            return _getKRecordList(market, code, ktype, query.start(), query.end());
        }
        return _getKRecordList(market, code, ktype, query.startDatetime(), query.endDatetime());
    }

    HKU_ERROR_IF_RETURN(!m_ifConvert, KRecordList(), "KData: unsupported ktype {}", ktype);

    // Non-base ktypes are calendar/session buckets aggregated from the base ktype; the bucket
    // boundaries cannot be expressed as a linear index mapping, so convert the full base series
    // first and then slice/filter the result.
    KQuery::KType base_ktype = getBaseKType(ktype);
    size_t base_count = getCount(market, code, base_ktype);
    KRecordList converted = convertToNewInterval(
      _getKRecordList(market, code, base_ktype, 0, base_count), base_ktype, ktype);

    if (query.queryType() == KQuery::INDEX) {
        int64_t total = static_cast<int64_t>(converted.size());
        int64_t start_ix = query.start();
        if (start_ix < 0) {
            start_ix += total;
            if (start_ix < 0)
                start_ix = 0;
        }
        int64_t end_ix;
        if (query.end() == Null<int64_t>()) {
            end_ix = total;
        } else {
            end_ix = query.end();
            if (end_ix < 0) {
                end_ix += total;
                if (end_ix < 0)
                    end_ix = 0;
            }
        }
        if (start_ix > end_ix)
            start_ix = end_ix;
        if (end_ix > total)
            end_ix = total;
        if (start_ix > total)
            start_ix = total;
        if (start_ix >= end_ix)
            return KRecordList();
        return KRecordList(converted.begin() + start_ix, converted.begin() + end_ix);
    }

    KRecordList result;
    Datetime start_date = query.startDatetime();
    Datetime end_date = query.endDatetime();
    for (const auto& record : converted) {
        if (record.datetime >= start_date && record.datetime < end_date)
            result.push_back(record);
    }
    return result;
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
        // The table may not exist
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
        // The table may not exist
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
    KQuery::KType base_ktype = getBaseKType(kType);
    return convertToNewInterval(_getKRecordList(market, code, base_ktype, 0, result), base_ktype,
                                kType)
      .size();
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
        // The table may not exist, the exception information is not printed
        out_start = 0;
        out_end = 0;
        return false;
    }

    return true;
}

KRecordList SQLiteKDataDriver::convertToNewInterval(const KRecordList& candles,
                                                    const KQuery::KType& from_ktype,
                                                    const KQuery::KType& to_ktype) {
    KRecordList result;
    HKU_IF_RETURN(candles.empty(), result);

    bool is_daily = (from_ktype == KQuery::DAY);

    size_t boundary_count = 0;
    const int* boundaries = nullptr;
    if (!is_daily) {
        boundaries = getMinuteEndBoundaries(to_ktype, boundary_count);
        HKU_ERROR_IF_RETURN(!boundaries, result, "KData: unsupported converted ktype {}", to_ktype);
    }

    result.reserve(candles.size());
    KRecord current;
    uint64_t current_key = 0;
    bool has_current = false;
    Datetime current_last_bar;

    for (const KRecord& bar : candles) {
        uint64_t key = 0;
        Datetime bar_timestamp;
        if (is_daily) {
            // Group by the natural calendar phase; phases without trading bars (holidays) are
            // simply absent, no trading calendar is needed.
            Datetime phase;
            if (!getDailyPhaseStart(to_ktype, bar.datetime, phase))
                return KRecordList();
            key = phase.ymd();
        } else {
            int hhmm = static_cast<int>(bar.datetime.hour() * 100 + bar.datetime.minute());
            const int* end = boundaries + boundary_count;
            const int* iter = std::lower_bound(boundaries, end, hhmm);
            if (iter == end) {
                // Bar outside the session boundaries, ignore it.
                continue;
            }
            key = bar.datetime.ymd() * 10000 + static_cast<uint64_t>(*iter);
            bar_timestamp = Datetime(bar.datetime.year(), bar.datetime.month(), bar.datetime.day(),
                                     *iter / 100, *iter % 100);
        }

        if (!has_current || key != current_key) {
            if (has_current) {
                // A fully suspended (all-zero) daily phase has no priced bar: fall back to the
                // last bar's timestamp so the bucket datetime is never left as a Null timestamp
                if (is_daily && current.datetime.isNull())
                    current.datetime = current_last_bar;
                result.push_back(std::move(current));
            }
            current = KRecord();
            current_key = key;
            has_current = true;
            current_last_bar = Datetime();
            if (!is_daily)
                current.datetime = bar_timestamp;
        }

        if (is_daily)
            current_last_bar = bar.datetime;

        current.transCount += bar.transCount;
        current.transAmount += bar.transAmount;
        if (bar.openPrice != 0 && bar.highPrice != 0 && bar.lowPrice != 0 && bar.closePrice != 0) {
            if (current.openPrice == 0)
                current.openPrice = bar.openPrice;
            if (bar.highPrice > current.highPrice)
                current.highPrice = bar.highPrice;
            if (current.lowPrice == 0 || bar.lowPrice < current.lowPrice)
                current.lowPrice = bar.lowPrice;
            current.closePrice = bar.closePrice;
            // Bars are timestamped at the interval end: the last trading day for daily-level
            // phases, the fixed session boundary for minute-level intervals.
            if (is_daily)
                current.datetime = bar.datetime;
        }
    }

    if (has_current) {
        if (is_daily && current.datetime.isNull())
            current.datetime = current_last_bar;
        result.push_back(std::move(current));
    }
    return result;
}

}  // namespace hku