/*
 * SQLiteBaseInfoDriver.cpp
 *
 *  Copyright (c) 2019 fasiondog
 * 
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#include <thread>
#include "../../../StockManager.h"
#include "SQLiteBaseInfoDriver.h"
#include "../table/MarketInfoTable.h"
#include "../table/StockTypeInfoTable.h"
#include "../table/StockWeightTable.h"
#include "../table/StockTable.h"

namespace hku {

SQLiteBaseInfoDriver::SQLiteBaseInfoDriver()
: BaseInfoDriver("sqlite3"), m_pool(nullptr) {
}

SQLiteBaseInfoDriver::~SQLiteBaseInfoDriver() {
    if (m_pool) {
        delete m_pool;
    }
}

bool SQLiteBaseInfoDriver::_init() {
    string dbname;
    try {
        dbname = getParam<string>("db");
        HKU_TRACE("SQLITE3: {}", dbname);
    } catch(...) {
        HKU_ERROR("Can't get Sqlite3 filename!");
        return false;
    }

    int cpu_num = std::thread::hardware_concurrency();
    if (cpu_num > 1) {
        cpu_num--;
    }
    m_pool = new DBConnectPool(m_params, cpu_num);
    return true;
}

bool SQLiteBaseInfoDriver::_loadMarketInfo() {
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return false;
    }

    DBConnectGuard dbGuard(m_pool);
    auto con = dbGuard.getConnect();
    
    vector<MarketInfoTable> infoTables;
    try {
        con->batchLoad(infoTables);
    } catch (std::exception& e) {
        HKU_FATAL("load Market table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load Market table failed!");
        return false;
    }

    StockManager& sm = StockManager::instance();
    for (auto& info: infoTables) {
        Datetime lastDate;
        try {
            lastDate = Datetime(info.lastDate() * 10000);
        } catch (...) {
            lastDate = Null<Datetime>();
            HKU_WARN("lastDate of market({}) is invalid! ", info.market());
        }
        sm.loadMarketInfo(
            MarketInfo(
                info.market(),
                info.name(),
                info.description(),
                info.code(),
                lastDate
        ));
    }

    return true;
}

bool SQLiteBaseInfoDriver::_loadStockTypeInfo() {
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return false;
    }

    DBConnectGuard dbGuard(m_pool);
    auto con = dbGuard.getConnect();

    vector<StockTypeInfoTable> infoTables;
    try {
        con->batchLoad(infoTables);
    } catch (std::exception& e) {
        HKU_FATAL("load StockTypeInfo table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load StockTypeInfo table failed!");
        return false;
    }

    StockManager& sm = StockManager::instance();
    for (auto& info: infoTables) {
        sm.loadStockTypeInfo(
            StockTypeInfo(
                info.type(),
                info.description(), 
                info.tick(),
                info.tickValue(), 
                info.precision(),
                info.minTradeNumber(), 
                info.maxTradeNumber()
        ));
    }

    return true;
}

bool SQLiteBaseInfoDriver::_loadStock() {
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return false;
    }

    DBConnectGuard dbGuard(m_pool);
    auto con = dbGuard.getConnect();

    vector<MarketInfoTable> marketTable;
    try {
        con->batchLoad(marketTable);
    } catch (std::exception& e) {
        HKU_FATAL("load Market table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load Market table failed!");
        return false;
    }

    unordered_map<uint64, string> marketDict;
    for (auto& m: marketTable) {
        marketDict[m.id()] = m.market();
    }

    vector<StockTable> table;
    try {
        con->batchLoad(table);
    } catch (std::exception& e) {
        HKU_FATAL("load Stock table failed! {}", e.what());
        return false;
    } catch (...) {
        HKU_FATAL("load Stock table failed!");
        return false;
    }

    Stock stock;
    StockTypeInfo stockTypeInfo;
    StockTypeInfo null_stockTypeInfo;
    StockManager& sm = StockManager::instance();
    for (auto& r: table) {
        //HKU_INFO("stock({},{},{},{},{},{},{},{})", r.stockid,r.marketid,r.code,HKU_STR(r.name),r.type,r.valid,r.startDate,r.endDate);
        Datetime startDate, endDate;
        if(r.startDate > r.endDate || r.startDate == 0 || r.endDate == 0) {
            //日期非法，置为Null<Datetime>
            startDate = Null<Datetime>();
            endDate = Null<Datetime>();
        } else {
            startDate = (r.startDate == 99999999LL)
                      ? Null<Datetime>()
                      : Datetime(r.startDate*10000);
            endDate = (r.endDate == 99999999LL)
                    ? Null<Datetime>()
                    : Datetime(r.endDate*10000);
        }

        stockTypeInfo = sm.getStockTypeInfo(r.type);
        if(stockTypeInfo != null_stockTypeInfo) {
            stock = Stock(marketDict[r.marketid],
                          r.code,
                          HKU_STR(r.name),
                          r.type,
                          r.valid,
                          startDate,
                          endDate,
                          stockTypeInfo.tick(),
                          stockTypeInfo.tickValue(),
                          stockTypeInfo.precision(),
                          stockTypeInfo.minTradeNumber(),
                          stockTypeInfo.maxTradeNumber());
            
        } else {
            stock = Stock(marketDict[r.marketid],
                          r.code,
                          HKU_STR(r.name),
                          r.type,
                          r.valid,
                          startDate,
                          endDate);
        }

        if(sm.loadStock(stock)){
            StockWeightList weightList = _getStockWeightList(r.stockid);
            stock.setWeightList(weightList);
        }
    }

    return true;
}

StockWeightList SQLiteBaseInfoDriver::_getStockWeightList(uint64 stockid) {
    StockWeightList result;
    if (!m_pool) {
        HKU_ERROR("Connect pool ptr is null!");
        return result;
    }

    DBConnectGuard dbGuard(m_pool);
    auto con = dbGuard.getConnect();

    vector<StockWeightTable> table;
    try {
        con->batchLoad(table, format("stockid={}", stockid));
    } catch (std::exception& e) {
        HKU_FATAL("load StockWeight table failed! {}", e.what());
        return result;
    } catch (...) {
        HKU_FATAL("load StockWeight table failed!");
        return result;
    }

    for (auto& w: table) {
        try {
            result.push_back(
                StockWeight(
                    Datetime(w.date*10000), 
                    w.countAsGift * 0.0001,
                    w.countForSell * 0.0001, 
                    w.priceForSell * 0.001,
                    w.bonus * 0.001,
                    w.countOfIncreasement * 0.0001,
                    w.totalCount, 
                    w.freeCount
                )
            );
        } catch (std::out_of_range& e) {
            HKU_WARN("Date of id({}) is invalid! {}", w.id(), e.what());
        } catch (std::exception& e) {
            HKU_WARN("Error StockWeight Record id({}) {}", w.id(), e.what());
        } catch (...) {
            HKU_WARN("Error StockWeight Record id({})! Unknow reason!", w.id());
        }
    }

    return result;
}

Parameter SQLiteBaseInfoDriver
::getFinanceInfo(const string& market, const string& code) {
    Parameter result;
    if (!m_pool) {
        return result;
    }
    
    std::stringstream buf;
    buf << "select f.updated_date, f.ipo_date, f.province,"
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

    DBConnectGuard dbGuard(m_pool);
    auto con = dbGuard.getConnect();

    auto st = con->getStatement(buf.str());
    st->exec();
    if(!st->moveNext()) {
        return result;
    }

    int updated_date, ipo_date;
    price_t province, industry, zongguben, liutongguben, guojiagu, faqirenfarengu;
    price_t farengu, bgu, hgu, zhigonggu, zongzichan, liudongzichan, gudingzichan;
    price_t wuxingzichan, gudongrenshu, liudongfuzhai, changqifuzhai, zibengongjijin;
    price_t jingzichan, zhuyingshouru, zhuyinglirun, yingshouzhangkuan, yingyelirun;
    price_t touzishouyi, jingyingxianjinliu, zongxianjinliu, cunhuo, lirunzonghe;
    price_t shuihoulirun, jinglirun, weifenpeilirun, meigujingzichan, baoliu2;

    st->getColumn(
        0, updated_date, ipo_date,
        province, industry, zongguben, liutongguben, guojiagu, faqirenfarengu,
        farengu, bgu, hgu, zhigonggu, zongzichan, liudongzichan, gudingzichan,
        wuxingzichan, gudongrenshu, liudongfuzhai, changqifuzhai, zibengongjijin,
        jingzichan, zhuyingshouru, zhuyinglirun, yingshouzhangkuan, yingyelirun,
        touzishouyi, jingyingxianjinliu, zongxianjinliu, cunhuo, lirunzonghe,
        shuihoulirun, jinglirun, weifenpeilirun, meigujingzichan, baoliu2
    );

    result.set<string>("market", market);
    result.set<string>("code", code);
    result.set<int>("updated_date", updated_date);
    result.set<int>("ipo_date", ipo_date);
    result.set<price_t>("province", province);
    result.set<price_t>("industry", industry);
    result.set<price_t>("zongguben", zongguben);
    result.set<price_t>("liutongguben", liutongguben);
    result.set<price_t>("guojiagu", guojiagu);
    result.set<price_t>("faqirenfarengu", faqirenfarengu);
    result.set<price_t>("farengu", farengu);
    result.set<price_t>("bgu", bgu);
    result.set<price_t>("hgu", hgu);
    result.set<price_t>("zhigonggu", zhigonggu);
    result.set<price_t>("zongzichan", zongzichan);
    result.set<price_t>("liudongzichan", liudongzichan);
    result.set<price_t>("gudingzichan", gudingzichan);
    result.set<price_t>("wuxingzichan", wuxingzichan);
    result.set<price_t>("gudongrenshu", gudongrenshu);
    result.set<price_t>("liudongfuzhai", liudongfuzhai);
    result.set<price_t>("changqifuzhai", changqifuzhai);
    result.set<price_t>("zibengongjijin", zibengongjijin);
    result.set<price_t>("jingzichan", jingzichan);
    result.set<price_t>("zhuyingshouru", zhuyingshouru);
    result.set<price_t>("zhuyinglirun", zhuyinglirun);
    result.set<price_t>("yingshouzhangkuan", yingshouzhangkuan);
    result.set<price_t>("yingyelirun", yingyelirun);
    result.set<price_t>("touzishouyi", touzishouyi);
    result.set<price_t>("jingyingxianjinliu", jingyingxianjinliu);
    result.set<price_t>("zongxianjinliu", zongxianjinliu);
    result.set<price_t>("cunhuo", cunhuo);
    result.set<price_t>("lirunzonghe", lirunzonghe);
    result.set<price_t>("shuihoulirun", shuihoulirun);
    result.set<price_t>("jinglirun", jinglirun);
    result.set<price_t>("weifenpeilirun", weifenpeilirun);
    result.set<price_t>("meigujingzichan", meigujingzichan);
    result.set<price_t>("baoliu2", baoliu2);
    return result;
}

} /* namespace */