/*
 * MySQLBaseInfoDriver.cpp
 *
 *  Created on: 2014年8月27日
 *      Author: fasiondog
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "MySQLBaseInfoDriver.h"

#include "../../../utilities/util.h"
#include "../../../StockManager.h"
#include "../../../Log.h"

namespace hku {

class MySQLCloser{
public:
    void operator()(MYSQL *db){
        if(db){
            mysql_close(db);
            //HKU_TRACE("Closed MySQL database!");
        }
    }
};


MySQLBaseInfoDriver::~MySQLBaseInfoDriver() {

}


bool MySQLBaseInfoDriver::_init() {
    string func_name(" [MySQLBaseInfoDriver::MySQLBaseInfoDriver]");

    shared_ptr<MYSQL> mysql(new MYSQL, MySQLCloser());
    if (!mysql){
        HKU_ERROR("Can't create MYSQL instance!" << func_name );
        return false;
    }

    string host("127.0.0.1");
    if (haveParam("host")) {
        host = getParam<string>("host");
    } else {
        setParam<string>("host", host);
    }

    string usr("root");
    if (haveParam("usr")) {
        usr = getParam<string>("usr");
    } else {
        setParam<string>("usr", usr);
    }

    string pwd;
    if (haveParam("pwd")) {
        pwd = getParam<string>("pwd");
    } else {
        setParam<string>("pwd", pwd);
    }

    string database("stk_base");
    if (haveParam("db")) {
        database = getParam<string>("db");
    } else {
        setParam<string>("db", database);
    }

    string port_str("3306");
    if (haveParam("port")) {
        port_str = getParam<string>("port");
    } else {
        setParam<string>("port", port_str);
    }

    unsigned int port;
    try {
        port = boost::lexical_cast<unsigned int>(port_str);
    } catch(...) {
        port = 3306;
    }

    HKU_TRACE("MYSQL host: " << host);
    HKU_TRACE("MYSQL port: " << port);
    HKU_TRACE("MYSQL database: " << database);

    if (!mysql_init(mysql.get())) {
        HKU_FATAL("Initial MySQL handle error!" << func_name);
        return false;
    }

    if (!mysql_real_connect(mysql.get(), host.c_str(), usr.c_str(),
            pwd.c_str(), database.c_str(), port, NULL, 0) ) {
        HKU_FATAL("Failed to connect to database!" << func_name);
        return false;
    }

    if (mysql_set_character_set(mysql.get(), "utf8")) {
        HKU_ERROR("mysql_set_character_set error!" << func_name);
        return false;
    }

    m_mysql = mysql;
    return true;
}


bool MySQLBaseInfoDriver::_loadMarketInfo() {
    string func_name(" [MySQLBaseInfoDriver::loadMarketInfo]");
    if (!m_mysql) {
        HKU_ERROR("Null m_mysql!" << func_name);
        return false;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    int res = mysql_query(m_mysql.get(), "select market,name,"
            "description,code,lastDate from market");
    if(res) {
        HKU_ERROR("mysql_query error! error no: " << res << func_name);
        return false;
    }

    result = mysql_store_result(m_mysql.get());
    if (!result) {
        HKU_ERROR("mysql_store_result error!" << func_name);
        return false;
    }

    StockManager& sm = StockManager::instance();
    while(row = mysql_fetch_row(result)) {
        string market(row[0]);
        boost::to_upper(market);
        Datetime last_date;
        try {
            hku_int64 d = (boost::lexical_cast<hku_uint64>(row[4])*10000);
            last_date = Datetime(d);
        } catch(...) {
            last_date = Null<Datetime>();
        }

        try {
            MarketInfo marketInfo(market, HKU_STR(row[1]), HKU_STR(row[2]),
                                     row[3], last_date);
            sm.addMarketInfo(marketInfo);
        } catch(...) {
            HKU_ERROR("Can't get MarketInfo " << market << func_name);
            continue;
        }
    }

    mysql_free_result(result);
    return true;
}


bool MySQLBaseInfoDriver::_loadStockTypeInfo() {
    string func_name(" [MySQLBaseInfoDriver::loadStockTypeInfo]");
    if (!m_mysql) {
        HKU_ERROR("Null m_mysql!" << func_name);
        return false;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    int res = mysql_query(m_mysql.get(), "select sti.type, sti.description, "
            "sti.tick, sti.tickValue, sti.precision, sti.minTradeNumber, "
            "sti.maxTradeNumber from stocktypeinfo as sti");
    if(res) {
        HKU_ERROR("mysql_query error! error no: " << res << func_name);
        return false;
    }

    result = mysql_store_result(m_mysql.get());
    if (!result) {
        HKU_ERROR("mysql_store_result error!" << func_name);
        return false;
    }

    StockManager& sm = StockManager::instance();
    while(row = mysql_fetch_row(result)) {
        hku_uint32 type = boost::lexical_cast<hku_uint32>(row[0]);
        try {
            StockTypeInfo stkTypeInfo(type, HKU_STR(row[1]),
                    boost::lexical_cast<price_t>(row[2]),
                    boost::lexical_cast<price_t>(row[3]),
                    boost::lexical_cast<int>(row[4]),
                    boost::lexical_cast<size_t>(row[5]),
                    boost::lexical_cast<size_t>(row[6]));
            sm.addStockTypeInfo(stkTypeInfo);
        } catch(...) {
            HKU_ERROR("Can't get StockTypeInfo " << type << func_name);
            continue;
        }
    }

    mysql_free_result(result);
    return true;
}


bool MySQLBaseInfoDriver::
_getStockWeightList(hku_uint64 stockid, StockWeightList& out) {
    string func_name(" [MySQLBaseInfoDriver::_getStockWeightList]");
    if (!m_mysql) {
        HKU_ERROR("Null m_mysql!" << func_name);
        return false;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    std::stringstream buf (std::stringstream::out);
    buf<<"select id, date, countAsGift, countForSell, priceForSell, bonus,\
          countOfIncreasement, totalCount, freeCount from stkweight \
          where stockid=" << stockid << " order by date";
    int res = mysql_query(m_mysql.get(), buf.str().c_str());
    if(res) {
        HKU_ERROR("mysql_query error! error no: " << res << func_name);
        return false;
    }

    result = mysql_store_result(m_mysql.get());
    if (!result) {
        HKU_ERROR("mysql_store_result error!" << func_name);
        return false;
    }

    my_ulonglong total = mysql_num_rows(result);
    if (total != 0)
        out.reserve(total);

    int id = 0;
    while(row = mysql_fetch_row(result)) {
        id = boost::lexical_cast<int>(row[0]);
        Datetime date;
        try {
            date = Datetime(row[1]);
            if (date >= (Datetime::max)()) {
                HKU_WARN("Invalid date!" << id << func_name);
                continue;
            }
        } catch (...) {
            continue;
        }

        try {
            out.push_back(StockWeight(date,
                    boost::lexical_cast<price_t>(row[2]),
                    boost::lexical_cast<price_t>(row[3]),
                    boost::lexical_cast<price_t>(row[4]),
                    boost::lexical_cast<price_t>(row[5]),
                    boost::lexical_cast<price_t>(row[6]),
                    boost::lexical_cast<price_t>(row[7]),
                    boost::lexical_cast<price_t>(row[8])));
        } catch(...) {
            HKU_WARN("Can't get weight record (id)" << id << func_name);
            continue;
        }
    }

    mysql_free_result(result);
    return true;
}


bool MySQLBaseInfoDriver::_loadStock() {
    string func_name(" [MySQLBaseInfoDriver::_loadStock]");
    if (!m_mysql) {
        HKU_ERROR("Null m_mysql!" << func_name);
        return false;
    }

    MYSQL_RES *result;
    MYSQL_ROW row;

    std::stringstream buf (std::stringstream::out);
    buf << "select stock.stockid, market.market, stock.code, stock.name, "
           "stock.type, stock.valid, stock.startDate, stock.endDate, "
           "stocktypeinfo.tick, stocktypeinfo.tickValue, stocktypeinfo.precision, "
           "stocktypeinfo.minTradeNumber, stocktypeinfo.maxTradeNumber "
           "from (stock left outer join stocktypeinfo on "
           "(stock.type = stocktypeinfo.type)) "
           "left outer join market on stock.marketid = market.marketid";
    int res = mysql_query(m_mysql.get(), buf.str().c_str());
    if(res) {
        HKU_ERROR("mysql_query error! error no: " << res << func_name);
        return false;
    }

    result = mysql_store_result(m_mysql.get());
    if (!result) {
        HKU_ERROR("mysql_store_result error!" << func_name);
        return false;
    }

    StockManager& sm = StockManager::instance();
    while(row = mysql_fetch_row(result)) {
        hku_uint64 stockid = boost::lexical_cast<hku_uint64>(row[0]);
        string market(row[1]);
        boost::to_upper(market);

        Datetime start_date;
        try {
            start_date = Datetime(row[6]);
            if (start_date >= (Datetime::max)())
                start_date = Null<Datetime>();
        } catch (...) {
            start_date = Null<Datetime>();
        }

        Datetime end_date;
        try {
            end_date = Datetime(row[7]);
            if (end_date >= (Datetime::max)())
                end_date = Null<Datetime>();
        } catch(...) {
            end_date = Null<Datetime>();
        }

        if (start_date >= end_date) {
            start_date = Null<Datetime>();
            end_date = Null<Datetime>();
        }

        try {
            Stock stock(market,
                    row[2], //code
                    HKU_STR(row[3]), //name
                    boost::lexical_cast<hku_uint32>(row[4]), //type
                    boost::lexical_cast<bool>(row[5]), //valid;
                    start_date, //startDate
                    end_date, //endDate
                    boost::lexical_cast<price_t>(row[8]), //tick
                    boost::lexical_cast<price_t>(row[9]), //tickValue
                    boost::lexical_cast<int>(row[10]), //precision
                    boost::lexical_cast<size_t>(row[11]),//minTradeNumber
                    boost::lexical_cast<size_t>(row[12])//maxTradeNumber
                    );
            StockWeightList weight;
            _getStockWeightList(stockid, weight);
            stock.setWeightList(weight);
            sm.addStock(stock);

        } catch(...) {
            HKU_ERROR("Can't get MarketInfo " << market << func_name);
            continue;
        }
    }

    mysql_free_result(result);
    return true;
}

} /* namespace hku */
