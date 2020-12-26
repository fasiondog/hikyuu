/*
 * StockManager.cpp
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "GlobalInitializer.h"
#include <chrono>
#include <fmt/format.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "utilities/IniParser.h"
#include "utilities/util.h"
#include "StockManager.h"
#include "GlobalTaskGroup.h"
#include "data_driver/kdata/cvs/KDataTempCsvDriver.h"
#include "data_driver/base_info/sqlite/SQLiteBaseInfoDriver.h"
#include "data_driver/base_info/mysql/MySQLBaseInfoDriver.h"
#include "data_driver/block_info/qianlong/QLBlockInfoDriver.h"
#include "data_driver/kdata/hdf5/H5KDataDriver.h"
#include "data_driver/kdata/tdx/TdxKDataDriver.h"
#include "data_driver/kdata/mysql/MySQLKDataDriver.h"

namespace hku {

StockManager* StockManager::m_sm = nullptr;

void StockManager::quit() {
    // releaseThreadPool();
    if (m_sm) {
        delete m_sm;
        m_sm = nullptr;
    }
}

StockManager::StockManager() {}
StockManager::~StockManager() {
    // releaseThreadPool();
    fmt::print("Quit Hikyuu system!\n\n");
}

StockManager& StockManager::instance() {
    if (!m_sm) {
        m_sm = new StockManager();
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
    param.set<bool>("ticks", false);
    param.set<int>("day_max", 100000);
    param.set<int>("week_max", 100000);
    param.set<int>("month_max", 100000);
    param.set<int>("quarter_max", 100000);
    param.set<int>("halfyear_max", 100000);
    param.set<int>("year_max", 100000);
    param.set<int>("min_max", 5120);
    param.set<int>("min5_max", 5120);
    param.set<int>("min15_max", 5120);
    param.set<int>("min30_max", 5120);
    param.set<int>("min60_max", 5120);
    param.set<int>("ticks_max", 5120);
    return param;
}

Parameter default_other_param() {
    Parameter param;
    param.set<string>("tmpdir", ".");
    param.set<string>("logger", "");
    return param;
}

void StockManager::init(const Parameter& baseInfoParam, const Parameter& blockParam,
                        const Parameter& kdataParam, const Parameter& preloadParam,
                        const Parameter& hikyuuParam) {
    m_baseInfoDriverParam = baseInfoParam;
    m_blockDriverParam = blockParam;
    m_kdataDriverParam = kdataParam;
    m_preloadParam = preloadParam;
    m_hikyuuParam = hikyuuParam;

    // 创建内部线程池
    // 不能同过 GlobalInitializer 初始化全局线程池
    // 原因是 std::thread 无法在 dllmain 中创建使用，会造成死锁
    // initThreadPool();

    // 获取路径信息
    m_tmpdir = hikyuuParam.tryGet<string>("tmpdir", ".");
    m_datadir = hikyuuParam.tryGet<string>("datadir", ".");

    m_stockDict.clear();
    m_marketInfoDict.clear();
    m_stockTypeInfo.clear();

    string funcname(" [StockManager::init]");

    //加载证券基本信息
    m_baseInfoDriver = DataDriverFactory::getBaseInfoDriver(baseInfoParam);
    HKU_CHECK(m_baseInfoDriver, "Failed get base info driver!");
    m_baseInfoDriver->loadBaseInfo();

    //获取板块驱动
    m_blockDriver = DataDriverFactory::getBlockDriver(blockParam);

    //获取K线数据驱动并预加载指定的数据
    HKU_INFO("Loading KData...");
    std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();

    KDataDriverPtr kdata_driver = DataDriverFactory::getKDataDriver(m_kdataDriverParam);
    setKDataDriver(kdata_driver);

    // add special Market, for temp csv file
    m_marketInfoDict["TMP"] =
      MarketInfo("TMP", "Temp Csv file", "temp load from csv file", "000001", Null<Datetime>(),
                 TimeDelta(0), TimeDelta(0), TimeDelta(0), TimeDelta(0));

    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start_time;
    HKU_INFO("{:<.2f}s Loaded Data.", sec.count());
}

void StockManager::setKDataDriver(const KDataDriverPtr& driver) {
    HKU_ERROR_IF_RETURN(!driver, void(), "kdata driver is null!");

    if (m_kdataDriverParam != driver->getParameter()) {
        m_kdataDriverParam = driver->getParameter();
    }

    bool preload_day = m_preloadParam.tryGet<bool>("day", false);
    HKU_INFO_IF(preload_day, "Preloading all day kdata to buffer!");

    bool preload_week = m_preloadParam.tryGet<bool>("week", false);
    HKU_INFO_IF(preload_week, "Preloading all week kdata to buffer!");

    bool preload_month = m_preloadParam.tryGet<bool>("month", false);
    HKU_INFO_IF(preload_week, "Preloading all month kdata to buffer!");

    bool preload_quarter = m_preloadParam.tryGet<bool>("quarter", false);
    HKU_INFO_IF(preload_quarter, "Preloading all quarter kdata to buffer!");

    bool preload_halfyear = m_preloadParam.tryGet<bool>("halfyear", false);
    HKU_INFO_IF(preload_halfyear, "Preloading all halfyear kdata to buffer!");

    bool preload_year = m_preloadParam.tryGet<bool>("year", false);
    HKU_INFO_IF(preload_year, "Preloading all year kdata to buffer!");

    bool preload_min = m_preloadParam.tryGet<bool>("min", false);
    HKU_INFO_IF(preload_min, "Preloading all 1 min kdata to buffer!");

    bool preload_min5 = m_preloadParam.tryGet<bool>("min5", false);
    HKU_INFO_IF(preload_min5, "Preloading all 5 min kdata to buffer!");

    bool preload_min15 = m_preloadParam.tryGet<bool>("min15", false);
    HKU_INFO_IF(preload_min15, "Preloading all 15 min kdata to buffer!");

    bool preload_min30 = m_preloadParam.tryGet<bool>("min30", false);
    HKU_INFO_IF(preload_min30, "Preloading all 30 min kdata to buffer!");

    bool preload_min60 = m_preloadParam.tryGet<bool>("min60", false);
    HKU_INFO_IF(preload_min60, "Preloading all 60 min kdata to buffer!");

    for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
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

string StockManager::datadir() const {
    return m_datadir;
}

Stock StockManager::getStock(const string& querystr) const {
    Stock result;
    string query_str = querystr;
    to_upper(query_str);
    auto iter = m_stockDict.find(query_str);
    return (iter != m_stockDict.end()) ? iter->second : result;
}

MarketInfo StockManager::getMarketInfo(const string& market) const {
    string market_tmp = market;
    to_upper(market_tmp);
    auto iter = m_marketInfoDict.find(market_tmp);
    return (iter != m_marketInfoDict.end()) ? iter->second : Null<MarketInfo>();
}

StockTypeInfo StockManager::getStockTypeInfo(uint32_t type) const {
    auto iter = m_stockTypeInfo.find(type);
    return (iter != m_stockTypeInfo.end()) ? iter->second : Null<StockTypeInfo>();
}

MarketList StockManager::getAllMarket() const {
    MarketList result;
    auto iter = m_marketInfoDict.begin();
    for (; iter != m_marketInfoDict.end(); ++iter) {
        result.push_back(iter->first);
    }
    return result;
}

Block StockManager::getBlock(const string& category, const string& name) {
    return m_blockDriver ? m_blockDriver->getBlock(category, name) : Block();
}

BlockList StockManager::getBlockList(const string& category) {
    return m_blockDriver ? m_blockDriver->getBlockList(category) : BlockList();
}

BlockList StockManager::getBlockList() {
    return m_blockDriver ? m_blockDriver->getBlockList() : BlockList();
}

DatetimeList StockManager::getTradingCalendar(const KQuery& query, const string& market) {
    auto marketinfo = getMarketInfo(market);
    return getStock(fmt::format("{}{}", marketinfo.market(), marketinfo.code()))
      .getDatetimeList(query);
}

Stock StockManager::addTempCsvStock(const string& code, const string& day_filename,
                                    const string& min_filename, price_t tick, price_t tickValue,
                                    int precision, size_t minTradeNumber, size_t maxTradeNumber) {
    string new_code(code);
    to_upper(new_code);
    Stock result("TMP", new_code, day_filename, STOCKTYPE_TMP, true, Datetime(199901010000),
                 Null<Datetime>(), tick, tickValue, precision, minTradeNumber, maxTradeNumber);

    KDataTempCsvDriver* p = new KDataTempCsvDriver(day_filename, min_filename);
    result.setKDataDriver(KDataDriverPtr(p));
    const auto& preload_param = getPreloadParameter();
    if (preload_param.tryGet<bool>("day", true)) {
        result.loadKDataToBuffer(KQuery::DAY);
    }
    if (preload_param.tryGet<bool>("min", false)) {
        result.loadKDataToBuffer(KQuery::MIN);
    }
    return loadStock(result) ? result : Null<Stock>();
}

void StockManager::removeTempCsvStock(const string& code) {
    string query_str = "TMP" + code;
    to_upper(query_str);
    auto iter = m_stockDict.find(query_str);
    if (iter != m_stockDict.end()) {
        m_stockDict.erase(iter);
    }
}

bool StockManager::loadStock(const Stock& stock) {
    string market_code(stock.market_code());
    to_upper(market_code);
    HKU_ERROR_IF_RETURN(m_stockDict.find(market_code) != m_stockDict.end(), false,
                        "The stock had exist! {}", market_code);
    m_stockDict[market_code] = stock;
    return true;
}

bool StockManager::loadMarketInfo(const MarketInfo& marketInfo) {
    string market = marketInfo.market();
    to_upper(market);
    HKU_ERROR_IF_RETURN(m_marketInfoDict.find(market) != m_marketInfoDict.end(), false,
                        "The marketInfo had exist! {}", market);
    m_marketInfoDict[market] = marketInfo;
    return true;
}

bool StockManager::loadStockTypeInfo(const StockTypeInfo& stkTypeInfo) {
    HKU_ERROR_IF_RETURN(m_stockTypeInfo.find(stkTypeInfo.type()) != m_stockTypeInfo.end(), false,
                        "The stockTypeInfo had exist! {}", stkTypeInfo.type());
    m_stockTypeInfo[stkTypeInfo.type()] = stkTypeInfo;
    return true;
}

}  // namespace hku
