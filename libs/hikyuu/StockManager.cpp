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
#include "data_driver/block_info/qianlong/QLBlockInfoDriver.h"
#include "data_driver/kdata/hdf5/H5KDataDriver.h"
#include "data_driver/kdata/tdx/TdxKDataDriver.h"

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

void StockManager::init(
        const Parameter& hikyuuParam,
        const Parameter& baseInfoParam,
        const Parameter& blockParam,
        const Parameter& kdataParam,
        const Parameter& preloadParam) {

    //初始化注册默认支持的数据驱动
    DataDriverFactory::regBaseInfoDriver(BaseInfoDriverPtr(new SQLiteBaseInfoDriver));
    DataDriverFactory::regBlockDriver(BlockInfoDriverPtr(new QLBlockInfoDriver));
    DataDriverFactory::regKDataDriver(KDataDriverPtr(new TdxKDataDriver));
    DataDriverFactory::regKDataDriver(KDataDriverPtr(new H5KDataDriver));

    try {
        m_tmpdir = hikyuuParam.get<string>("tmpdir");
    } catch (...) {
        m_tmpdir = "";
    }

    try {
        string logger = hikyuuParam.get<string>("logger");
        init_logger(logger);
    } catch(...) {
        init_logger("");
    }

    BaseInfoDriverPtr base_info = DataDriverFactory::getBaseInfoDriver(baseInfoParam);
    base_info->loadBaseInfo();


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

    m_tmpdir = m_iniconfig->get("tmpdir", "tmpdir", ".");

    if (m_iniconfig->hasSection("logger")) {
        init_logger(m_iniconfig->get("logger", "properties", ""));
    } else {
        init_logger("");
    }

    if (!m_iniconfig->hasSection("baseinfo")) {
        HKU_FATAL("Missing configure of baseinfo!");
        exit(1);
    }

    IniParser::StringListPtr block_config = m_iniconfig->getOptionList("block");
    for (auto iter = block_config->begin(); iter != block_config->end(); ++iter) {
        string value = m_iniconfig->get("block", *iter);
        m_blockDriver_params.set<string>(*iter, value);
    }

    IniParser::StringListPtr baseinfo_config = m_iniconfig->getOptionList("baseinfo");
    Parameter baseinfo_params;
    for (auto iter = baseinfo_config->begin(); iter != baseinfo_config->end(); ++iter) {
        string value = m_iniconfig->get("baseinfo", *iter);
        baseinfo_params.set<string>(*iter, value);
    }

    //DataDriverFactory driver;
    //m_data_driver.regBaseInfoDriver("mysql")
    BaseInfoDriverPtr base_info = DataDriverFactory::getBaseInfoDriver(baseinfo_params);
    base_info->loadBaseInfo();

    HKU_TRACE("Loading KData...");
    boost::chrono::system_clock::time_point start_time = boost::chrono::system_clock::now();

    IniParser::StringListPtr option = m_iniconfig->getOptionList("kdata");
    Parameter params;
    for (auto iter = option->begin(); iter != option->end(); ++iter) {
        params.set<string>(*iter, m_iniconfig->get("kdata", *iter));
    }

    DataDriverFactory::regKDataDriver(KDataDriverPtr(new TdxKDataDriver));
    DataDriverFactory::regKDataDriver(KDataDriverPtr(new H5KDataDriver));
    KDataDriverPtr kdata_driver = DataDriverFactory::getKDataDriver(params);

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

    //add special Market, for temp csv file
    m_marketInfoDict["TMP"] = MarketInfo("TMP", "Temp Csv file",
                                         "temp load from csv file",
                                         "000001", Null<Datetime>());

    boost::chrono::duration<double> sec = boost::chrono::system_clock::now() - start_time;
    HKU_TRACE(sec << " Loaded Data.");
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
    Block result;
    auto block_driver = DataDriverFactory::getBlockDriver(m_blockDriver_params);
    if (!block_driver) {
        return result;
    }
    return block_driver->getBlock(category, name);
}

BlockList StockManager::getBlockList(const string& category) {
    BlockList result;
    auto block_driver = DataDriverFactory::getBlockDriver(m_blockDriver_params);
    if (!block_driver) {
        return result;
    }
    return block_driver->getBlockList(category);
}

BlockList StockManager::getBlockList() {
    BlockList result;
    auto block_driver = DataDriverFactory::getBlockDriver(m_blockDriver_params);
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
