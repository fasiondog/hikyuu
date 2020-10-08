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

MySQLKDataDriver::MySQLKDataDriver() : KDataDriver("mysql"), m_pool(nullptr) {}

MySQLKDataDriver::~MySQLKDataDriver() {
    if (m_pool) {
        delete m_pool;
    }
}

bool MySQLKDataDriver::_init() {
    HKU_ASSERT_M(m_pool == nullptr, "Maybe repeat initialization!");
    Parameter connect_param;
    connect_param.set<string>("db", "");  //数据库名称须在SQL语句中明确指定
    connect_param.set<string>("host", getParamFromOther<string>(m_params, "host", "127.0.0.1"));
    connect_param.set<string>("usr", getParamFromOther<string>(m_params, "usr", "root"));
    connect_param.set<string>("pwd", getParamFromOther<string>(m_params, "pwd", ""));
    string port_str = getParamFromOther<string>(m_params, "port", "3306");
    unsigned int port = boost::lexical_cast<unsigned int>(port_str);
    m_pool = new ConnectPool<MySQLConnect>(connect_param);
    return true;
}

string MySQLKDataDriver ::_getTableName(const string& market, const string& code,
                                        KQuery::KType ktype) {
    string table = fmt::format("`{}`_`{}`.`{}`", market, KQuery::getKTypeName(ktype), code);
    to_lower(table);
    return std::move(table);
}

KRecordList MySQLKDataDriver::getKRecordList(const string& market, const string& code,
                                             const KQuery& query) {
    KRecordList result;
    if (query.queryType() == KQuery::INDEX) {
        result = _getKRecordList(market, code, query.kType(), query.start(), query.end());
    } else {
        HKU_INFO("Query by date are not supported!");
    }
    return result;
}

KRecordList MySQLKDataDriver::_getKRecordList(const string& market, const string& code,
                                              KQuery::KType kType, size_t start_ix, size_t end_ix) {
    KRecordList result;
    if (!m_pool) {
        HKU_ERROR("The connection pool is not initialized.");
        return result;
    };

    if (start_ix >= end_ix) {
        HKU_ERROR("start_ix({}) >= endix({})", start_ix, end_ix);
        return result;
    }

    auto con = m_pool->getConnect();
    if (!con) {
        HKU_ERROR("The acquisition connection failed.");
        return result;
    };

    KRecordTable r(market, code, kType);
    SQLStatementPtr st = con->getStatement(
      fmt::format("{} order by date limit {}, {}", r.getSelectSQL(), start_ix, end_ix - start_ix));

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
    return result;
}

size_t MySQLKDataDriver::getCount(const string& market, const string& code, KQuery::KType kType) {
    size_t result = 0;
    if (!m_pool) {
        HKU_ERROR("The connection pool is not initialized.");
        return result;
    }

    auto con = m_pool->getConnect();
    if (!con) {
        HKU_ERROR("The acquisition connection failed.");
        return result;
    };

    try {
        result =
          con->queryInt(fmt::format("select count(1) from {}", _getTableName(market, code, kType)));
    } catch (std::exception& e) {
        HKU_ERROR(e.what());
        result = 0;
    } catch (...) {
        HKU_ERROR("Unknow error!");
        result = 0;
    }

    return result;
}

bool MySQLKDataDriver::getIndexRangeByDate(const string& market, const string& code,
                                           const KQuery& query, size_t& out_start,
                                           size_t& out_end) {
    out_start = 0;
    out_end = 0;
    if (query.queryType() != KQuery::DATE) {
        HKU_ERROR("queryType must be KQuery::DATE");
        return false;
    }

    if (query.startDatetime() >= query.endDatetime() || query.startDatetime() > (Datetime::max)()) {
        return false;
    }

    if (!m_pool) {
        HKU_ERROR("The connection pool is not initialized.");
        return false;
    }

    auto con = m_pool->getConnect();
    if (!con) {
        HKU_ERROR("The acquisition connection failed.");
        return false;
    };

    string tablename = _getTableName(market, code, query.kType());
    try {
        out_start = con->queryInt(fmt::format("select count(1) from {} where date<{}", tablename,
                                              query.startDatetime().number()));
        out_end = con->queryInt(fmt::format("select count(1) from {} where date<{}", tablename,
                                            query.endDatetime().number()));
    } catch (std::exception& e) {
        HKU_ERROR(e.what());
        out_start = 0;
        out_end = 0;
        return false;
    } catch (...) {
        HKU_ERROR("Unknow error!");
        out_start = 0;
        out_end = 0;
        return false;
    }

    return true;
}

} /* namespace hku */
