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


void StockManager::init(const string& filename) {
    try {
        m_iniconfig = shared_ptr<IniParser>(new IniParser);
        if (!m_iniconfig) {
            HKU_FATAL("Can't new IniParser! [StockManager::init]");
            exit(1);
        }

        m_iniconfig->read(filename);

    } catch (std::invalid_argument& e) {
        HKU_FATAL("Reading configure error!\n" << e.what());
        exit(1);
    } catch (std::logic_error& e) {
        HKU_FATAL("Reading configure error!\n" << e.what());
        exit(1);
    } catch(...) {
        HKU_WARN("Reading configure error! Don't know  error!");
        exit(1);
    }

    if (m_iniconfig->hasSection("logger")) {
        init_logger(m_iniconfig->get("logger", "properties", ""));
    } else {
        init_logger("");
    }

    if (!m_iniconfig->hasSection("baseinfo")) {
        HKU_FATAL("Missing configure of baseinfo!");
        exit(1);
    }

    BaseInfoDriverPtr base_info = DataDriverFactory::getBaseInfoDriver(m_iniconfig);
    HKU_TRACE("Loading market information...");
    if (!base_info->loadMarketInfo(m_marketInfoDict)) {
        HKU_FATAL("Can't load Market Information.");
        exit(1);
    }

    HKU_TRACE("Loading stock type information...");
    if (!base_info->loadStockTypeInfo(m_stockTypeInfo)) {
        HKU_FATAL("Can't load StockType Information.");
        exit(1);
    }

    HKU_TRACE("Loading stock information...");
    if (!base_info->loadStock()) {
        HKU_FATAL("Can't load Stock");
        exit(1);
    }

    HKU_TRACE("Loading KData...");
    boost::chrono::system_clock::time_point start_time = boost::chrono::system_clock::now();

    KDataDriverPtr kdata_driver = DataDriverFactory::getKDataDriver(m_iniconfig);

    bool preload_day = m_iniconfig->getBool("preload", "day", "false");
    bool preload_week = m_iniconfig->getBool("preload", "week", "false");
    bool preload_month = m_iniconfig->getBool("preload", "month", "false");
    bool preload_quarter = m_iniconfig->getBool("preload", "quater", "false");
    bool preload_halfyear = m_iniconfig->getBool("preload", "halfyear", "false");
    bool preload_year = m_iniconfig->getBool("preload", "year", "false");

    for(auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
        iter->second.setKDataDriver(kdata_driver);

        if (preload_day) iter->second.loadKDataToBuffer(KQuery::DAY);
        if (preload_week) iter->second.loadKDataToBuffer(KQuery::WEEK);
        if (preload_month) iter->second.loadKDataToBuffer(KQuery::MONTH);
        if (preload_quarter) iter->second.loadKDataToBuffer(KQuery::QUARTER);
        if (preload_halfyear) iter->second.loadKDataToBuffer(KQuery::HALFYEAR);
        if (preload_year) iter->second.loadKDataToBuffer(KQuery::YEAR);
    }

    boost::chrono::duration<double> sec = boost::chrono::system_clock::now() - start_time;
    HKU_TRACE(sec << " Loaded Data.");
}



string StockManager::tmpdir() const {
    return m_iniconfig->get("tmpdir", "tmpdir", ".");
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


bool StockManager::addStock(const Stock& stock) {
    auto iter = m_stockDict.find(stock.market_code());
    if(iter != m_stockDict.end()) {
        HKU_ERROR("The stock had exist! [StockManager::addStock]");
        return false;
    }

    m_stockDict[stock.market_code()] = stock;
    return true;
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
    Block result;
    auto block_driver = DataDriverFactory::getBlockDriver(m_iniconfig);
    if (!block_driver) {
        return result;
    }
    return block_driver->getBlock(category, name);
}

BlockList StockManager::getBlockList(const string& category) {
    BlockList result;
    auto block_driver = DataDriverFactory::getBlockDriver(m_iniconfig);
    if (!block_driver) {
        return result;
    }
    return block_driver->getBlockList(category);
}

BlockList StockManager::getBlockList() {
    BlockList result;
    auto block_driver = DataDriverFactory::getBlockDriver(m_iniconfig);
    if (!block_driver) {
        return result;
    }
    return block_driver->getBlockList();
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

} /* namespace */
