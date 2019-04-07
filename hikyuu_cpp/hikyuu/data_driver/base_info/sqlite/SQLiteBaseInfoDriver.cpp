/*
 * SQLiteBaseInfoDriver.cpp
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "SQLiteBaseInfoDriver.h"
#include "../../../utilities/util.h"
#include "../../../StockManager.h"
#include "../../../Log.h"

namespace hku {

class Sqlite3Closer{
public:
    void operator()(sqlite3 *db){
        if(db){
            sqlite3_close(db);
            //std::cout << "Closed Sqlite3 database!" << std::endl;
            //Cannot use log output when exiting!
            //HKU_TRACE("Closed Sqlite3 database!");
        }
    }
};


bool SQLiteBaseInfoDriver::_init() {
    string dbname;
    try {
        dbname = getParam<string>("db");
        HKU_TRACE("SQLITE3: " << dbname);
    } catch(...) {
        HKU_ERROR("Can't get Sqlite3 filename!"
                " [SQLiteBaseInfoDriver::SQLiteBaseInfoDriver]");
        return false;
    }

    sqlite3 *db;
    int rc=sqlite3_open_v2(dbname.c_str(), &db,
                             SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX,0);
    if( rc ){
        HKU_ERROR("Can't open database: " << sqlite3_errmsg(db) << "("
                << dbname << ") [SQLiteBaseInfoDriver::SQLiteBaseInfoDriver]");
        sqlite3_close(db);
        return false;
    }

    m_db = shared_ptr<sqlite3>(db, Sqlite3Closer());

    return true;
}


bool SQLiteBaseInfoDriver::_loadMarketInfo() {
    if (!m_db) {
        return false;
    }

    list<MarketInfo> out;
    char *zErrMsg=0;
    int rc = sqlite3_exec(m_db.get(), "select market,name,"
                            "description,code,lastdate from market",
                            _getMarketTableCallBack, &out, &zErrMsg);
    if( rc != SQLITE_OK ){
        HKU_ERROR("SQL error: " << zErrMsg
                << " [SQLiteBaseInfoDriver::loadMarketInfo]");
        sqlite3_free(zErrMsg);
        return false;
    }

    StockManager& sm = StockManager::instance();
    for (auto iter = out.begin(); iter != out.end(); ++iter) {
        sm.addMarketInfo(*iter);
    }

    return true;
}

//select market,name,description,code,lastdate from market
int SQLiteBaseInfoDriver::_getMarketTableCallBack(void *out,
        int nCol, char **azVals, char **azCols) {
    assert(nCol==5);
    int result = 0;
    hku_uint64 d;
    string market(azVals[0]);
    boost::to_upper(market);
    try{
        d = (boost::lexical_cast<hku_uint64>(azVals[4])*10000);
        Datetime datetime;
        try {
            datetime = Datetime(d);
        } catch (std::out_of_range& e) {
            datetime = Null<Datetime>();
            HKU_WARN("LastDate of market(" << market << ") is invalide!"
                    " Assigned to Null<Dateteim>! " << e.what() <<
                    " [SQLiteBaseInfoDriver::_getMarketTableCallBack]");
        }

        MarketInfo marketInfo(market, HKU_STR(azVals[1]),
                HKU_STR(azVals[2]), azVals[3], datetime);
        ((list<MarketInfo> *)out)->push_back(marketInfo);
        result = 0;

    }catch(boost::bad_lexical_cast& e){
        HKU_ERROR("Can't get the information of market(" << market
                << ") " << e.what() << " [SQLiteBaseInfoDriver::_getMarketTableCallBack]");
        result = 1;

    }catch(...){
        HKU_ERROR("Some error! [SQLiteBaseInfoDriver::_getMarketTableCallBack]");
        result = 1;
    }
    return result;
}

bool SQLiteBaseInfoDriver::_loadStockTypeInfo() {
    if (!m_db) {
        return false;
    }

    list<StockTypeInfo> out;
    char *zErrMsg=0;
    int rc = sqlite3_exec(m_db.get(), "select type, description, tick, "
                          "tickValue, precision, minTradeNumber, "
                          "maxTradeNumber from StockTypeInfo",
                        _getStockTypeInfoTableCallBack, &out, &zErrMsg);
    if( rc!=SQLITE_OK ){
        HKU_ERROR("SQL error: " << zErrMsg
                << " [SQLiteBaseInfoDriver::loadStockTypeInfo]");
        sqlite3_free(zErrMsg);
        return false;
    }

    StockManager& sm = StockManager::instance();
    for (auto iter = out.begin(); iter != out.end(); ++iter) {
        sm.addStockTypeInfo(*iter);
    }

    return true;
}

//select type,description,tick,precision,minTradeNumber,maxTradeNumber
//from stockTypeInfo
int SQLiteBaseInfoDriver::_getStockTypeInfoTableCallBack(
            void *out, int nCol, char **azVals, char **azCols){
    assert(nCol == 7);
    int result = 0;
    try{
        hku_uint32 type = boost::lexical_cast<hku_uint32>(azVals[0]);
        StockTypeInfo stockTypeInfo(
                type, HKU_STR(azVals[1]),
                boost::lexical_cast<price_t>(azVals[2]),
                boost::lexical_cast<price_t>(azVals[3]),
                boost::lexical_cast<int>(azVals[4]),
                boost::lexical_cast<size_t>(azVals[5]),
                boost::lexical_cast<size_t>(azVals[6]));
        ((list<StockTypeInfo> *)out)->push_back(stockTypeInfo);
        result = 0;
    }catch(boost::bad_lexical_cast& e){
        HKU_ERROR("bad_lexical_cast! " << e.what()
                << " [SQLiteBaseInfoDriver::_getStockTypeInfoTableCallBack]");
        result = 1;
    }catch(...){
        HKU_ERROR("Some error! [SQLiteBaseInfoDriver::_getStockTypeInfoTableCallBack]");
        result = 1;
    }
    return result;
}

bool SQLiteBaseInfoDriver::_getStockWeightList(hku_uint32 id,
        StockWeightList& out) {
    if (!m_db) {
        return false;
    }

    char *zErrMsg=0;
    std::stringstream buf (std::stringstream::out);
    buf<<"select id, date, countAsGift, countForSell, priceForSell, bonus,\
          countOfIncreasement, totalCount, freeCount from stkWeight \
          where stockid=" << id << " order by date";
    int rc = sqlite3_exec(m_db.get(), buf.str().c_str(),
                           _getStockWeightCallBack, &out, &zErrMsg);
    if( rc!=SQLITE_OK ){
        HKU_ERROR("SQL error: " << zErrMsg
                << " [SQLiteBaseInfoDriver::getStockWeightList]");
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}

//select id, date, countAsGift, countForSell, priceForSell,
//bobus, countOfIncreasement, totalCount, freeCount
//from stkweight where stockid=? order by date
int SQLiteBaseInfoDriver::_getStockWeightCallBack(
        void *out, int nCol, char **azVals, char **azCols){
    assert(nCol == 9);
    int failure = 0;
    int id = 0;
    try{
        id = boost::lexical_cast<int>(azVals[0]);
        hku_uint64 datetime;
        datetime = boost::lexical_cast<hku_uint64>(azVals[1]) * 10000;
        StockWeight weight(Datetime(datetime),
                boost::lexical_cast<price_t>(azVals[2]) * 0.0001,
                boost::lexical_cast<price_t>(azVals[3]) * 0.0001,
                boost::lexical_cast<price_t>(azVals[4]) * 0.001,
                boost::lexical_cast<price_t>(azVals[5]) * 0.001,
                boost::lexical_cast<price_t>(azVals[6]) * 0.0001,
                boost::lexical_cast<price_t>(azVals[7]),
                boost::lexical_cast<price_t>(azVals[8]));
        ((StockWeightList *)out)->push_back(weight);
        failure = 0;
    }catch(std::out_of_range& e) {
        HKU_ERROR("Date of id(" << id << ") is invalid! " << e.what()
                << " [SQLiteBaseInfoDriver::_getStockWeightCallBack]");
        //不返回失败，仅抛弃该条记录

    }catch(boost::bad_lexical_cast& e){
        HKU_ERROR("id(" << id << ") bad_lexical_cast! " << e.what()
                << " [SQLiteBaseInfoDriver::_getStockWeightCallBack] ");
        failure = 1;

    }catch(...){
        HKU_ERROR("Some error! [SQLiteBaseInfoDriver::_getStockWeightCallBack]");
        failure = 1;
    }

    return failure;
}

struct StockTable {
    hku_uint32 id;
    string market;
    string code;
    string name;
    hku_uint32 type;
    bool valid;
    Datetime startDate;
    Datetime endDate;
};

bool SQLiteBaseInfoDriver::_loadStock() {
    if (!m_db) {
        return false;
    }

    list<StockTable> stock_table;
    char *zErrMsg=0;
    int rc = sqlite3_exec(m_db.get(), "select stock.stockid,market.market,\
            Stock.code,Stock.name,Stock.type,Stock.valid,Stock.startDate, \
            Stock.endDate from Stock inner join Market \
            on Stock.marketid = Market.marketid",
            _getStockTableCallBack, &stock_table, &zErrMsg);
    if( rc != SQLITE_OK ){
        HKU_ERROR("SQL error: " << zErrMsg
                << " [SQLiteBaseInfoDriver::loadStock]");
        sqlite3_free(zErrMsg);
        return false;
    }

    Stock stock;
    StockTypeInfo stockTypeInfo;
    StockTypeInfo null_stockTypeInfo;
    StockManager& sm = StockManager::instance();
    list<StockTable>::iterator iter = stock_table.begin();
    for(; iter != stock_table.end(); ++iter) {
        stockTypeInfo = sm.getStockTypeInfo(iter->type);
        if(stockTypeInfo != null_stockTypeInfo) {
            stock = Stock(iter->market,
                          iter->code,
                          iter->name,
                          iter->type,
                          iter->valid,
                          iter->startDate,
                          iter->endDate,
                          stockTypeInfo.tick(),
                          stockTypeInfo.tickValue(),
                          stockTypeInfo.precision(),
                          stockTypeInfo.minTradeNumber(),
                          stockTypeInfo.maxTradeNumber());
        } else {
            stock = Stock(iter->market,
                          iter->code,
                          iter->name,
                          iter->type,
                          iter->valid,
                          iter->startDate,
                          iter->endDate);
        }

        if(sm.addStock(stock)){
            StockWeightList weightList;
            _getStockWeightList(iter->id, weightList);
            stock.setWeightList(weightList);
        }
    }

    return true;
}

//select stock.stockid,market.market,stock.code,stock.name,stock.type,
//stock.valid,stock.startDate,stock.endDate from stock
//inner join market on stock.marketid = market.marketid
int SQLiteBaseInfoDriver::_getStockTableCallBack(
        void *out, int nCol, char **azVals, char **azCols) {
    assert(nCol == 8);
    int result = 0;
    StockTable stockRecord;
    try {
        stockRecord.id = boost::lexical_cast<hku_uint32>(azVals[0]);
        stockRecord.market = string(azVals[1]);
        stockRecord.code = string(azVals[2]);
        stockRecord.name = string(HKU_STR(azVals[3]));
        stockRecord.type = boost::lexical_cast<hku_uint32>(azVals[4]);
        hku_uint32 temp_valid = boost::lexical_cast<hku_uint32>(azVals[5]);
        stockRecord.valid = temp_valid > 0 ? true : false;
        Datetime datetime;
        hku_uint64 startDate, endDate;
        startDate = boost::lexical_cast<hku_uint64>(azVals[6])*10000;
        endDate = boost::lexical_cast<hku_uint64>(azVals[7])*10000;
        if(startDate > endDate || startDate == 0 || endDate == 0) {
            //日期非法，置为Null<Datetime>
            stockRecord.startDate = Null<Datetime>();
            stockRecord.endDate = Null<Datetime>();
        } else {
            stockRecord.startDate = (startDate == 999999990000LL)
                                  ? Null<Datetime>()
                                  : Datetime(startDate);
            stockRecord.endDate = (endDate == 999999990000LL)
                                ? Null<Datetime>()
                                : Datetime(endDate);
        }
        ((list<StockTable> *)out)->push_back(stockRecord);
        result = 0;

    } catch(std::out_of_range& e) {
        HKU_WARN("Date of stock(" << stockRecord.id << ") is invalid! "
                << e.what() << " [SQLiteBaseInfoDriver::_getStockTableCallBack]");
        //不返回失败，仅抛弃该记录
        result = 0;

    } catch(boost::bad_lexical_cast& e) {
        HKU_ERROR("bad_lexical_cast! " << e.what()
                << " [SQLiteBaseInfoDriver::_getStockTableCallBack]");
        result = 1;

    } catch(...) {
        HKU_ERROR("Some error! [SQLiteBaseInfoDriver::_getStockTableCallBack]");
        result = 1;
    }

    return result;
}


Parameter SQLiteBaseInfoDriver
::getFinanceInfo(const string& market, const string& code) {
    Parameter result;
    if (!m_db) {
        return result;
    }
    
    std::stringstream buf;
    buf << "select m.market, s.code, f.updated_date, f.ipo_date, f.province,"
        << "f.industry, f.zongguben, f.liutongguben, f.guojiagu, f.faqirenfarengu,"
        << "f.farengu, f.bgu, f.hgu, f.zhigonggu, f.zongzichan, f.liudongzichan,"
        << "f.gudingzichan, f.wuxingzichan, f.gudongrenshu, f.liudongfuzhai,"
        << "f.changqifuzhai, f.zibengongjijin, f.jingzichan, f.zhuyingshouru,"
        << "f.zhuyinglirun, f.yingshouzhangkuan, f.yingyelirun, f.touzishouyu,"
        << "f.jingyingxianjinliu, f.zongxianjinliu, f.cunhuo, f.lirunzonghe,"
        << "f.shuihoulirun, f.jinglirun, f.weifenpeilirun, f.meigujingzichan,"
        << "f.baoliu2 from stkfinance f, stock s, market m "
        << "where m.market='" << market << "'"
        << " and s.code = '" << code << "'"
        << " and s.marketid = m.marketid"
        << " and f.stockid = s.stockid"
        << " order by updated_date DESC limit 1";
    //std::cout << buf.str() << std::endl;
    char *zErrMsg=0;
    int rc = sqlite3_exec(m_db.get(), buf.str().c_str(),
                    _getFinanceTableCallBack, &result, &zErrMsg);
    if( rc != SQLITE_OK ){
        HKU_ERROR("SQL error: " << zErrMsg
                << " [SQLiteBaseInfoDriver::getFinanceInfo]");
        sqlite3_free(zErrMsg);
        return result;
    }

    return result;
}

int SQLiteBaseInfoDriver
::_getFinanceTableCallBack(void *out, int nCol,
                           char **azVals, char **azCols) {
    assert(nCol == 37);
    int result = 0;
    Parameter *p = (Parameter *)out;
    try {
        p->set<string>("market", string(azVals[0]));
        p->set<string>("code", string(azVals[1]));
        p->set<int>("updated_date", atof(azVals[2]));
        p->set<int>("ipo_date", atof(azVals[3]));
        p->set<price_t>("province", atof(azVals[4]));
        p->set<price_t>("industry", atof(azVals[5]));
        p->set<price_t>("zongguben", atof(azVals[6]));
        p->set<price_t>("liutongguben", atof(azVals[7]));
        p->set<price_t>("guojiagu", atof(azVals[8]));
        p->set<price_t>("faqirenfarengu", atof(azVals[9]));
        p->set<price_t>("farengu", atof(azVals[10]));
        p->set<price_t>("bgu", atof(azVals[11]));
        p->set<price_t>("hgu", atof(azVals[12]));
        p->set<price_t>("zhigonggu", atof(azVals[13]));
        p->set<price_t>("zongzichan", atof(azVals[14]));
        p->set<price_t>("liudongzichan", atof(azVals[15]));
        p->set<price_t>("gudingzichan", atof(azVals[16]));
        p->set<price_t>("wuxingzichan", atof(azVals[17]));
        p->set<price_t>("gudongrenshu", atof(azVals[18]));
        p->set<price_t>("liudongfuzhai", atof(azVals[19]));
        p->set<price_t>("changqifuzhai", atof(azVals[20]));
        p->set<price_t>("zibengongjijin", atof(azVals[21]));
        p->set<price_t>("jingzichan", atof(azVals[22]));
        p->set<price_t>("zhuyingshouru", atof(azVals[23]));
        p->set<price_t>("zhuyinglirun", atof(azVals[24]));
        p->set<price_t>("yingshouzhangkuan", atof(azVals[25]));
        p->set<price_t>("yingyelirun", atof(azVals[26]));
        p->set<price_t>("touzishouyi", atof(azVals[27]));
        p->set<price_t>("jingyingxianjinliu", atof(azVals[28]));
        p->set<price_t>("zongxianjinliu", atof(azVals[29]));
        p->set<price_t>("cunhuo", atof(azVals[30]));
        p->set<price_t>("lirunzonghe", atof(azVals[31]));
        p->set<price_t>("shuihoulirun", atof(azVals[32]));
        p->set<price_t>("jinglirun", atof(azVals[33]));
        p->set<price_t>("weifenpeilirun", atof(azVals[34]));
        p->set<price_t>("meigujingzichan", atof(azVals[35]));
        //p->set<price_t>("baoliu2", atof(azVals[36]));
        result = 0;

    } catch(...) {
        HKU_ERROR("Some error! [SQLiteBaseInfoDriver::_getFinanceTableCallBack]");
        result = 1;
    }
    return result;
}


} /* namespace hku */
