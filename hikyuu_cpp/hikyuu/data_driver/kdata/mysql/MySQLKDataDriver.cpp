/*
 * MySQLKDataDriverImp.cpp
 *
 *  Created on: 2014年9月3日
 *      Author: fasiondog
 */

#include <boost/lexical_cast.hpp>
#include "../../../Log.h"
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
    string table = fmt::format("`{}_{}`.`{}`", market, KQuery::getKTypeName(ktype), code);
    to_lower(table);
    return table;
}

KRecordList MySQLKDataDriver::getKRecordList(const string& market, const string& code,
                                             const KQuery& query) {
    KRecordList result;
    if (query.queryType() == KQuery::INDEX) {
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
        KRecordTable r(market, code, kType);
        SQLStatementPtr st = m_connect->getStatement(fmt::format(
          "{} order by date limit {}, {}", r.getSelectSQL(), start_ix, end_ix - start_ix));

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
    }
    return result;
}

KRecordList MySQLKDataDriver::_getKRecordList(const string& market, const string& code,
                                              const KQuery::KType& ktype, Datetime start_date,
                                              Datetime end_date) {
    KRecordList result;
    HKU_IF_RETURN(start_date >= end_date, result);

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
                k.transAmount = record.amount();
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
        out_start = m_connect->queryInt(fmt::format("select count(1) from {} where date<{}",
                                                    tablename, query.startDatetime().number()),
                                        0);
        out_end = m_connect->queryInt(fmt::format("select count(1) from {} where date<{}",
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

        if (startix < endix) {
            SQLStatementPtr st = m_connect->getStatement(
              fmt::format("select `date`, `price`, `vol` from {} order by date limit {}, {}", table,
                          startix, endix - startix));

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
                result.emplace_back(Datetime(date), price, vol,
                                    static_cast<TransRecord::DIRECT>(direct));
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

        if (startix < endix) {
            SQLStatementPtr st = m_connect->getStatement(fmt::format(
              "select `date`, `price`, `vol`, `buyorsell` from {} order by date limit {}, {}",
              table, startix, endix - startix));

            st->exec();
            while (st->moveNext()) {
                try {
                    uint64_t date = 0;
                    double price = 0.0, vol = 0.0;
                    int direct = 0;
                    st->getColumn(0, date, price, vol);
                    result.emplace_back(Datetime(date), price, vol,
                                        static_cast<TransRecord::DIRECT>(direct));
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
