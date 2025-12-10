/*
 * MySQLKDataDriverImp.cpp
 *
 *  Created on: 2014年9月3日
 *      Author: fasiondog
 */

#include <boost/lexical_cast.hpp>
#include "hikyuu/utilities/Log.h"
#include "MySQLKDataDriver.h"
#include "KRecordTable.h"

namespace hku {

MySQLKDataDriver::MySQLKDataDriver() : KDataDriver("mysql"), m_connect(nullptr) {}

MySQLKDataDriver::~MySQLKDataDriver() {
    if (m_connect) {
        delete m_connect;
    }
}

bool MySQLKDataDriver::_init() {
    HKU_CHECK(m_connect == nullptr, "Maybe repeat initialization!");
    Parameter connect_param;
    connect_param.set<string>("db", "");  // 数据库名称须在SQL语句中明确指定
    connect_param.set<string>("host", getParamFromOther<string>(m_params, "host", "127.0.0.1"));
    connect_param.set<string>("usr", getParamFromOther<string>(m_params, "usr", "root"));
    connect_param.set<string>("pwd", getParamFromOther<string>(m_params, "pwd", ""));
    string port_str = getParamFromOther<string>(m_params, "port", "3306");
    unsigned int port = boost::lexical_cast<unsigned int>(port_str);
    connect_param.set<int>("port", port);
    m_connect = new MySQLConnect(connect_param);
    return true;
}

string MySQLKDataDriver ::_getTableName(const string& market, const string& code,
                                        const KQuery::KType& ktype) {
    string table;
    if (ktype == KQuery::TIMELINE) {
        table = fmt::format("`{}_time`.`{}`", market, code);
    } else if (ktype == KQuery::TRANS) {
        table = fmt::format("`{}_trans`.`{}`", market, code);
    } else {
        table = fmt::format("`{}_{}`.`{}`", market, KQuery::getKTypeName(ktype), code);
    }
    to_lower(table);
    return table;
}

KRecordList MySQLKDataDriver::getKRecordList(const string& market, const string& code,
                                             const KQuery& query) {
    KRecordList result;
    auto kType = query.kType();
    if (kType == KQuery::TIMELINE) {
        auto timeline = getTimeLineList(market, code, query);
        size_t total = timeline.size();
        result.resize(total);
        for (size_t i = 0; i < total; ++i) {
            result[i].datetime = timeline[i].datetime;
            result[i].closePrice = timeline[i].price;
            result[i].transCount = timeline[i].vol;
        }

    } else if (kType == KQuery::TRANS) {
        auto trans = getTransList(market, code, query);
        size_t total = trans.size();
        result.resize(total);
        for (size_t i = 0; i < total; ++i) {
            result[i].datetime = trans[i].datetime;
            result[i].closePrice = trans[i].price;
            result[i].transCount = trans[i].vol;
            result[i].openPrice = trans[i].direct;
        }

    } else if (query.queryType() == KQuery::INDEX) {
        result = _getKRecordList(market, code, query.kType(), query.start(), query.end());
    } else {
        result =
          _getKRecordList(market, code, query.kType(), query.startDatetime(), query.endDatetime());
    }
    return result;
}

KRecordList MySQLKDataDriver::_getKRecordList(const string& market, const string& code,
                                              const KQuery::KType& kType, size_t start_ix,
                                              size_t end_ix) {
    KRecordList result;
    HKU_IF_RETURN(start_ix >= end_ix, result);

    try {
        size_t total = getCount(market, code, kType);
        HKU_IF_RETURN(total == 0, result);

        size_t mid_total = total / 2;
        if (end_ix == Null<size_t>()) {
            end_ix = total;
        }
        std::string sql;
        if (start_ix < mid_total) {
            sql = fmt::format(
              "select date, open, high, low, close, amount, `count` from {} order by date limit "
              "{}, {}",
              _getTableName(market, code, kType), start_ix, end_ix - start_ix);
        } else {
            sql = fmt::format(
              "select * from (select date as date, open, high, low, close, amount, `count` from {} "
              "order by date desc limit {}) a order by date limit 0, {}",
              _getTableName(market, code, kType), total - start_ix, end_ix - start_ix);
        }

        uint64_t date;
        price_t open, high, low, close, amount, count;
        SQLStatementPtr st = m_connect->getStatement(sql);
        st->exec();
        while (st->moveNext()) {
            try {
                st->getColumn(0, date, open, high, low, close, amount, count);
                result.emplace_back(Datetime(date), open, high, low, close, amount * 0.1, count);
            } catch (...) {
                HKU_ERROR("Failed get krecord: {}{} {}", market, code, kType);
            }
        }

    } catch (...) {
        // 表可能不存在
    }
    return result;
}

KRecordList MySQLKDataDriver::_getKRecordList(const string& market, const string& code,
                                              const KQuery::KType& ktype, Datetime start_date,
                                              Datetime end_date) {
    KRecordList result;
    HKU_IF_RETURN(start_date.isNull() || (!end_date.isNull() && start_date >= end_date), result);

    try {
        KRecordTable r(market, code, ktype);
        SQLStatementPtr st = m_connect->getStatement(
          fmt::format("{} where date >= {} and date < {} order by date", r.getSelectSQL(),
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
                k.transAmount = record.amount() * 0.1;
                k.transCount = record.count();
                result.push_back(k);
            } catch (...) {
                HKU_ERROR("Failed get record: {}", record.str());
            }
        }
    } catch (...) {
        // 表可能不存在
    }
    return result;
}

size_t MySQLKDataDriver::getCount(const string& market, const string& code,
                                  const KQuery::KType& kType) {
    size_t result = 0;

    try {
        result = m_connect->queryNumber(
          fmt::format("select count(1) from {}", _getTableName(market, code, kType)), 0);
    } catch (...) {
        // 表可能不存在, 不打印异常信息
        result = 0;
    }

    return result;
}

bool MySQLKDataDriver::getIndexRangeByDate(const string& market, const string& code,
                                           const KQuery& query, size_t& out_start,
                                           size_t& out_end) {
    out_start = 0;
    out_end = 0;
    HKU_ERROR_IF_RETURN(query.queryType() != KQuery::DATE, false, "queryType must be KQuery::DATE");
    HKU_IF_RETURN(
      query.startDatetime() >= query.endDatetime() || query.startDatetime() > (Datetime::max)(),
      false);

    string tablename = _getTableName(market, code, query.kType());
    try {
        uint64_t start, end;
        if (query.kType() == KQuery::TRANS) {
            start = query.startDatetime().ymdhms();
            end = query.endDatetime().ymdhms();
        } else {
            start = query.startDatetime().ymdhm();
            end = query.endDatetime().ymdhm();
        }
        string sql = fmt::format(
          "select COUNT(CASE WHEN date<{} THEN 1 END) AS startix, COUNT(CASE WHEN date<{} THEN 1 "
          "END) AS endix from {}",
          start, end, tablename);
        SQLStatementPtr st = m_connect->getStatement(sql);
        st->exec();
        if (st->moveNext()) {
            st->getColumn(0, out_start, out_end);
        }

    } catch (...) {
        // 表可能不存在, 不打印异常信息
        out_start = 0;
        out_end = 0;
        return false;
    }

    return true;
}

TimeLineList MySQLKDataDriver::getTimeLineList(const string& market, const string& code,
                                               const KQuery& query) {
    TimeLineList result;
    HKU_ERROR_IF_RETURN(query.queryType() >= KQuery::INVALID, result, "Invalid queryType! {}{} {}",
                        market, code, query);
    if (query.queryType() == KQuery::INDEX) {
        result = _getTimeLineListByIndex(market, code, query);
    } else {
        result = _getTimeLineListByDate(market, code, query);
    }
    return result;
}

TimeLineList MySQLKDataDriver::_getTimeLineListByDate(const string& market, const string& code,
                                                      const KQuery& query) {
    string table = fmt::format("`{}_time`.`{}`", market, code);
    to_lower(table);

    TimeLineList result;
    try {
        SQLStatementPtr st = m_connect->getStatement(fmt::format(
          "select `date`, `price`, `vol` from {} where date >= {} and date < {} order by date",
          table, query.startDatetime().number(), query.endDatetime().number()));

        m_connect->transaction();
        st->exec();
        while (st->moveNext()) {
            try {
                uint64_t date = 0;
                double price = 0.0, vol = 0.0;
                st->getColumn(0, date, price, vol);
                result.emplace_back(Datetime(date), price, vol);
            } catch (const std::exception& e) {
                HKU_ERROR("Failed get {} timeline! {}", table, e.what());
            } catch (...) {
                HKU_ERROR("Failed get {} timeline!", table, code);
            }
        }

        m_connect->commit();

    } catch (const std::exception&) {
        m_connect->rollback();
    } catch (...) {
        m_connect->rollback();
    }

    return result;
}

TimeLineList MySQLKDataDriver::_getTimeLineListByIndex(const string& market, const string& code,
                                                       const KQuery& query) {
    string table = fmt::format("`{}_time`.`{}`", market, code);
    to_lower(table);

    TimeLineList result;
    try {
        m_connect->transaction();

        int64_t startix = query.start(), endix = query.end();
        int64_t total = m_connect->queryNumber(fmt::format("select count(1) from {}", table), 0LL);

        if (startix < 0) {
            startix += total;
            if (startix < 0)
                startix = 0;
        }

        if (endix < 0) {
            endix += total;
            if (endix < 0)
                endix = 0;
        }

        if (endix == Null<int64_t>()) {
            endix = total;
        }

        if (startix < endix) {
            int64_t mid_total = total / 2;
            string sql;
            if (startix < mid_total) {
                sql =
                  fmt::format("select `date`, `price`, `vol` from {} order by date limit {}, {}",
                              table, startix, endix - startix);
            } else {
                sql = fmt::format(
                  "select * from (select `date`, `price`, `vol` from {} order by date desc limit "
                  "{}) a order by date limit 0, {}",
                  table, total - startix, endix - startix);
            }
            SQLStatementPtr st = m_connect->getStatement(sql);

            st->exec();
            while (st->moveNext()) {
                try {
                    uint64_t date = 0;
                    double price = 0.0, vol = 0.0;
                    st->getColumn(0, date, price, vol);
                    result.emplace_back(Datetime(date), price, vol);
                } catch (const std::exception& e) {
                    HKU_ERROR("Failed get {} timeline! {}", table, e.what());
                } catch (...) {
                    HKU_ERROR("Failed get {} timeline!", table, code);
                }
            }
        }

        m_connect->commit();

    } catch (const std::exception&) {
        m_connect->rollback();
    } catch (...) {
        m_connect->rollback();
    }

    return result;
}

TransList MySQLKDataDriver::getTransList(const string& market, const string& code,
                                         const KQuery& query) {
    TransList result;
    HKU_ERROR_IF_RETURN(query.queryType() >= KQuery::INVALID, result, "Invalid queryType! {}{} {}",
                        market, code, query);
    if (query.queryType() == KQuery::INDEX) {
        result = _getTransListByIndex(market, code, query);
    } else {
        result = _getTransListByDate(market, code, query);
    }
    return result;
}

TransList MySQLKDataDriver::_getTransListByDate(const string& market, const string& code,
                                                const KQuery& query) {
    string table = fmt::format("`{}_trans`.`{}`", market, code);
    to_lower(table);

    TransList result;
    try {
        SQLStatementPtr st = m_connect->getStatement(
          fmt::format("select `date`, `price`, `vol`, `buyorsell` from {} where date >= {} and "
                      "date < {} order by date",
                      table, query.startDatetime().ymdhms(), query.endDatetime().ymdhms()));

        m_connect->transaction();
        st->exec();
        while (st->moveNext()) {
            try {
                uint64_t date = 0;
                double price = 0.0, vol = 0.0;
                int direct = 0;
                st->getColumn(0, date, price, vol, direct);
                result.emplace_back(Datetime(date), price, vol, direct);
            } catch (const std::exception& e) {
                HKU_ERROR("Failed get {} trans! {}", table, e.what());
            } catch (...) {
                HKU_ERROR("Failed get {} trans!", table, code);
            }
        }

        m_connect->commit();

    } catch (const std::exception&) {
        m_connect->rollback();
    } catch (...) {
        m_connect->rollback();
    }

    return result;
}

TransList MySQLKDataDriver::_getTransListByIndex(const string& market, const string& code,
                                                 const KQuery& query) {
    string table = fmt::format("`{}_trans`.`{}`", market, code);
    to_lower(table);

    TransList result;
    try {
        m_connect->transaction();

        int64_t startix = query.start(), endix = query.end();
        int64_t total = m_connect->queryNumber(fmt::format("select count(1) from {}", table), 0LL);

        if (startix < 0) {
            startix += total;
            if (startix < 0)
                startix = 0;
        }

        if (endix < 0) {
            endix += total;
            if (endix < 0)
                endix = 0;
        }

        if (endix == Null<int64_t>()) {
            endix = total;
        }

        if (startix < endix) {
            int64_t mid_total = total / 2;
            string sql;
            if (startix < mid_total) {
                sql = fmt::format(
                  "select `date`, `price`, `vol`, `buyorsell` from {} order by date limit {}, {}",
                  table, startix, endix - startix);
            } else {
                sql = fmt::format(
                  "select * from (select `date`, `price`, `vol`, `buyorsell` from {} order by date "
                  "desc limit {}) a order by date limit 0, {}",
                  table, total - startix, endix - startix);
            }
            SQLStatementPtr st = m_connect->getStatement(sql);

            st->exec();
            while (st->moveNext()) {
                try {
                    uint64_t date = 0;
                    double price = 0.0, vol = 0.0;
                    int direct = 0;
                    st->getColumn(0, date, price, vol, direct);
                    result.emplace_back(Datetime(date), price, vol, direct);
                } catch (const std::exception& e) {
                    HKU_ERROR("Failed get {} trans! {}", table, e.what());
                } catch (...) {
                    HKU_ERROR("Failed get {} trans!", table, code);
                }
            }
        }

        m_connect->commit();

    } catch (const std::exception&) {
        m_connect->rollback();
    } catch (...) {
        m_connect->rollback();
    }

    return result;
}

} /* namespace hku */
