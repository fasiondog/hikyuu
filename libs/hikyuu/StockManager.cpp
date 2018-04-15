/*
 * StockManager.cpp
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/chrono.hpp>

#include "utilities/util.h"
#include "StockManager.h"
#include "data_driver/DataDriverFactory.h"
#include "data_driver/KDataTempCsvDriver.h"
#include "data_driver/base_info/sqlite/SQLiteBaseInfoDriver.h"
#include "data_driver/base_info/mysql/MySQLBaseInfoDriver.h"
#include "data_driver/block_info/qianlong/QLBlockInfoDriver.h"
#include "data_driver/kdata/hdf5/H5KDataDriver.h"
#include "data_driver/kdata/tdx/TdxKDataDriver.h"
#include "data_driver/kdata/mysql/MySQLKDataDriver.h"

namespace hku {

shared_ptr<StockManager> StockManager::m_sm;

StockManager::~StockManager() {
    HKU_TRACE("Quit Hikyuu system!\n");
}


StockManager& StockManager::instance() {
    if( !m_sm ) {
        m_sm = shared_ptr<StockManager>(new StockManager());
    }
    return (*m_sm);
}


Parameter default_preload_param() {
    Parameter param;
    param.set<bool>("day", true);
    param.set<bool>("week", false);
    param.set<bool>("month", false);
    param.set<bool>("quarter", false);
    param.set<bool>("halfyear", false);
    param.set<bool>("year", false);
    param.set<bool>("min", false);
    param.set<bool>("min5", false);
    param.set<bool>("min15", false);
    param.set<bool>("min30", false);
    param.set<bool>("min60", false);
    return param;
}

Parameter default_other_param() {
    Parameter param;
    param.set<string>("tmpdir", ".");
    param.set<string>("logger","");
    return param;
}

void StockManager::init(
        const Parameter& baseInfoParam,
        const Parameter& blockParam,
        const Parameter& kdataParam,
        const Parameter& preloadParam,
        const Parameter& hikyuuParam) {

    m_baseInfoDriverParam = baseInfoParam;
    m_blockDriverParam = blockParam;
    m_kdataDriverParam = kdataParam;
    m_preloadParam = preloadParam;
    m_hikyuuParam = hikyuuParam;

    m_stockDict.clear();
    m_marketInfoDict.clear();
    m_stockTypeInfo.clear();

    string funcname(" [StockManager::init]");

    //初始化注册默认支持的数据驱动
    DataDriverFactory::regBaseInfoDriver(BaseInfoDriverPtr(new SQLiteBaseInfoDriver));
    DataDriverFactory::regBaseInfoDriver(BaseInfoDriverPtr(new MySQLBaseInfoDriver));

    DataDriverFactory::regBlockDriver(BlockInfoDriverPtr(new QLBlockInfoDriver));
    DataDriverFactory::regKDataDriver(KDataDriverPtr(new TdxKDataDriver));
    DataDriverFactory::regKDataDriver(KDataDriverPtr(new H5KDataDriver));
    DataDriverFactory::regKDataDriver(KDataDriverPtr(new H5KDataDriver));
    DataDriverFactory::regKDataDriver(KDataDriverPtr(new MySQLKDataDriver));

    //获取临时路径信息
    try {
        m_tmpdir = hikyuuParam.get<string>("tmpdir");
    } catch (...) {
        m_tmpdir = "";
    }

    //获取log4cplus配置文件信息
    try {
        string logger = hikyuuParam.get<string>("logger");
        init_logger(logger);
    } catch(...) {
        init_logger("");
    }

    //加载证券基本信息
    BaseInfoDriverPtr base_info = DataDriverFactory::getBaseInfoDriver(baseInfoParam);
    base_info->loadBaseInfo();

    //获取板块驱动
    m_blockDriver = DataDriverFactory::getBlockDriver(blockParam);

    //获取K线数据驱动并预加载指定的数据
    HKU_TRACE("Loading KData...");
    boost::chrono::system_clock::time_point start_time = boost::chrono::system_clock::now();

    KDataDriverPtr kdata_driver = DataDriverFactory::getKDataDriver(kdataParam);

    setKDataDriver(kdata_driver);

    //add special Market, for temp csv file
    m_marketInfoDict["TMP"] = MarketInfo("TMP", "Temp Csv file",
                                         "temp load from csv file",
                                         "000001", Null<Datetime>());

    boost::chrono::duration<double> sec = boost::chrono::system_clock::now() - start_time;
    HKU_TRACE(sec << " Loaded Data.");
}


void StockManager::setKDataDriver(const KDataDriverPtr& driver) {
    string funcname(" [StockManager::setKDataDriver]");
    if (!driver) {
        HKU_ERROR("kdata driver is null!" << funcname);
        return;
    }

    if (m_kdataDriverParam == driver->getParameter()) {

    } else {
        m_kdataDriverParam = driver->getParameter();
    }

    bool preload_day = false;
    try {
        preload_day = m_preloadParam.get<bool>("day");
        if (preload_day)
            HKU_TRACE("Preload all day kdata to buffer!" << funcname);
    } catch(...) {
        preload_day = false;
    }

    bool preload_week = false;
    try {
        preload_week = m_preloadParam.get<bool>("week");
        if (preload_week)
            HKU_TRACE("Preload all week kdata to buffer!" << funcname);
    } catch(...) {
        preload_week = false;
    }

    bool preload_month = false;
    try {
        preload_month = m_preloadParam.get<bool>("month");
        if (preload_week)
            HKU_TRACE("Preload all month kdata to buffer!" << funcname);
    } catch(...) {
        preload_month = false;
    }

    bool preload_quarter = false;
    try {
        preload_quarter = m_preloadParam.get<bool>("quarter");
        if (preload_quarter)
            HKU_TRACE("Preload all quarter kdata to buffer!" << funcname);
    } catch(...) {
        preload_quarter = false;
    }

    bool preload_halfyear = false;
    try {
        preload_halfyear = m_preloadParam.get<bool>("halfyear");
        if (preload_halfyear)
            HKU_TRACE("Preload all halfyear kdata to buffer!" << funcname);
    } catch(...) {
        preload_halfyear = false;
    }

    bool preload_year = false;
    try {
        preload_year = m_preloadParam.get<bool>("year");
        if (preload_year)
            HKU_TRACE("Preload all year kdata to buffer!" << funcname);
    } catch(...) {
        preload_year = false;
    }

    bool preload_min = false;
    try {
        preload_min = m_preloadParam.get<bool>("min");
        if (preload_min)
            HKU_TRACE("Preload all 1 min kdata to buffer!" << funcname);
    } catch(...) {
        preload_min = false;
    }

    bool preload_min5 = false;
    try {
        preload_min5 = m_preloadParam.get<bool>("min5");
        if (preload_min5)
            HKU_TRACE("Preload all 5 min kdata to buffer!" << funcname);
    } catch(...) {
        preload_min5 = false;
    }

    bool preload_min15 = false;
    try {
        preload_min15 = m_preloadParam.get<bool>("min15");
        if (preload_min15)
            HKU_TRACE("Preload all 15 min kdata to buffer!" << funcname);
    } catch(...) {
        preload_min15 = false;
    }

    bool preload_min30 = false;
    try {
        preload_min30 = m_preloadParam.get<bool>("min30");
        if (preload_min30)
            HKU_TRACE("Preload all 30 min kdata to buffer!" << funcname);
    } catch(...) {
        preload_min30 = false;
    }

    bool preload_min60 = false;
    try {
        preload_min60 = m_preloadParam.get<bool>("min60");
        if (preload_min60)
            HKU_TRACE("Preload all 60 min kdata to buffer!" << funcname);
    } catch(...) {
        preload_min60 = false;
    }

    for(auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
        if (iter->second.market() == "TMP")
            continue;

        iter->second.setKDataDriver(driver);

        if (preload_day)
            iter->second.loadKDataToBuffer(KQuery::DAY);

        if (preload_week)
            iter->second.loadKDataToBuffer(KQuery::WEEK);

        if (preload_month)
            iter->second.loadKDataToBuffer(KQuery::MONTH);

        if (preload_quarter)
            iter->second.loadKDataToBuffer(KQuery::QUARTER);

        if (preload_halfyear)
            iter->second.loadKDataToBuffer(KQuery::HALFYEAR);

        if (preload_year)
            iter->second.loadKDataToBuffer(KQuery::YEAR);

        if (preload_min)
            iter->second.loadKDataToBuffer(KQuery::MIN);

        if (preload_min5)
            iter->second.loadKDataToBuffer(KQuery::MIN5);

        if (preload_min15)
            iter->second.loadKDataToBuffer(KQuery::MIN15);

        if (preload_min30)
            iter->second.loadKDataToBuffer(KQuery::MIN30);

        if (preload_min60)
            iter->second.loadKDataToBuffer(KQuery::MIN60);
    }
}


string StockManager::tmpdir() const {
    return m_tmpdir;
}


Stock StockManager::getStock(const string& querystr) const {
    Stock result;
    string query_str = querystr;
    boost::to_upper(query_str);
    auto iter = m_stockDict.find(query_str);
    if(iter != m_stockDict.end()) {
        return iter->second;
    }
    return result;
}


MarketInfo StockManager::getMarketInfo(const string& market) const {
    string market_tmp = market;
    boost::to_upper(market_tmp);
    auto iter = m_marketInfoDict.find(market_tmp);
    if(iter != m_marketInfoDict.end()) {
        return iter->second;
    }
    return Null<MarketInfo>();
}


StockTypeInfo StockManager::getStockTypeInfo(hku_uint32 type) const {
    auto iter = m_stockTypeInfo.find(type);
    if( iter != m_stockTypeInfo.end() ){
        return iter->second;
    }
    return Null<StockTypeInfo>();
}


MarketList StockManager::getAllMarket() const {
    MarketList result;
    auto iter = m_marketInfoDict.begin();
    for(; iter != m_marketInfoDict.end(); ++iter) {
        result.push_back(iter->first);
    }
    return result;
}


Block StockManager::getBlock(const string& category, const string& name) {
    return m_blockDriver ? m_blockDriver->getBlock(category, name): Block();
}

BlockList StockManager::getBlockList(const string& category) {
    return m_blockDriver ? m_blockDriver->getBlockList(category) : BlockList();
}

BlockList StockManager::getBlockList() {
    return m_blockDriver ? m_blockDriver->getBlockList() : BlockList();
}

DatetimeList StockManager::
getTradingCalendar(const KQuery& query, const string& market) {
    Stock stock = getStock("SH000001");
    size_t start_ix = 0, end_ix = 0;
    DatetimeList result;
    if (stock.getIndexRange(query, start_ix, end_ix)) {
        result = stock.getDatetimeList(start_ix, end_ix, query.kType());
    }
    return result;
}

Stock StockManager::addTempCsvStock(
        const string& code,
        const string& day_filename,
        const string& min_filename,
        price_t tick,
        price_t tickValue,
        int precision,
        size_t minTradeNumber,
        size_t maxTradeNumber) {
    string new_code(code);
    boost::to_upper(new_code);
    Stock result("TMP", new_code, day_filename, STOCKTYPE_TMP, true,
            Datetime(199901010000), Null<Datetime>(),
            tick, tickValue, precision, minTradeNumber, maxTradeNumber);

    KDataTempCsvDriver *p = new KDataTempCsvDriver(day_filename, min_filename);
    result.setKDataDriver(KDataDriverPtr(p));
    result.loadKDataToBuffer(KQuery::DAY);
    result.loadKDataToBuffer(KQuery::MIN);

    if (!addStock(result)){
        //加入失败，返回Null<Stock>
        return Null<Stock>();
    }

    return result;
}

void StockManager::removeTempCsvStock(const string& code) {
    string query_str = "TMP" + code;
    boost::to_upper(query_str);
    auto iter = m_stockDict.find(query_str);
    if(iter != m_stockDict.end()) {
        m_stockDict.erase(iter);
    }
}


bool StockManager::addStock(const Stock& stock) {
    string market_code(stock.market_code());
    boost::to_upper(market_code);
    if(m_stockDict.find(market_code) != m_stockDict.end()) {
        HKU_ERROR("The stock had exist! "
                << market_code
                << " [StockManager::addStock]");
        return false;
    }

    m_stockDict[market_code] = stock;
    return true;
}


bool StockManager::addMarketInfo(const MarketInfo& marketInfo) {
    string market = marketInfo.market();
    boost::to_upper(market);
    if (m_marketInfoDict.find(market) != m_marketInfoDict.end()) {
        HKU_ERROR("The marketInfo had exist! "
                << market
                << " [StockManager::addMarketInfo]");
        return false;
    }

    m_marketInfoDict[market] = marketInfo;
    return true;
}

bool StockManager::addStockTypeInfo(const StockTypeInfo& stkTypeInfo) {
    if (m_stockTypeInfo.find(stkTypeInfo.type()) != m_stockTypeInfo.end()) {
        HKU_ERROR("The stockTypeInfo had exist! "
                << stkTypeInfo.type()
                << " [StockManager::addStockTypeInfo]");
        return false;
    }

    m_stockTypeInfo[stkTypeInfo.type()] = stkTypeInfo;
    return true;
}

} /* namespace */
