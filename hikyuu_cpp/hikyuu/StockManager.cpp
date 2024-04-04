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
#include "utilities/thread/ThreadPool.h"
#include "StockManager.h"
#include "global/GlobalTaskGroup.h"
#include "global/schedule/inner_tasks.h"
#include "data_driver/kdata/cvs/KDataTempCsvDriver.h"

namespace hku {

StockManager* StockManager::m_sm = nullptr;

void StockManager::quit() {
    if (m_sm) {
        delete m_sm;
        m_sm = nullptr;
    }
}

StockManager::StockManager() : m_initializing(false) {
    m_stockDict_mutex = new std::mutex;
    m_marketInfoDict_mutex = new std::mutex;
    m_stockTypeInfo_mutex = new std::mutex;
    m_holidays_mutex = new std::mutex;
}

StockManager::~StockManager() {
    delete m_stockDict_mutex;
    delete m_marketInfoDict_mutex;
    delete m_stockTypeInfo_mutex;
    delete m_holidays_mutex;
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
    param.set<bool>("hour2", false);
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
    param.set<int>("hour2_max", 5120);
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
                        const Parameter& hikyuuParam, const StrategyContext& context) {
    HKU_WARN_IF_RETURN(m_initializing, void(),
                       "The last initialization has not finished. Please try again later!");
    m_initializing = true;
    m_thread_id = std::this_thread::get_id();
    m_baseInfoDriverParam = baseInfoParam;
    m_blockDriverParam = blockParam;
    m_kdataDriverParam = kdataParam;
    m_preloadParam = preloadParam;
    m_hikyuuParam = hikyuuParam;
    m_context = context;

    // 获取路径信息
    m_tmpdir = hikyuuParam.tryGet<string>("tmpdir", ".");
    m_datadir = hikyuuParam.tryGet<string>("datadir", ".");

    m_stockDict.clear();
    m_marketInfoDict.clear();
    m_stockTypeInfo.clear();

    // 加载证券基本信息
    m_baseInfoDriver = DataDriverFactory::getBaseInfoDriver(baseInfoParam);
    HKU_CHECK(m_baseInfoDriver, "Failed get base info driver!");

    loadAllHolidays();
    loadAllMarketInfos();
    loadAllStockTypeInfo();
    loadAllStocks();
    loadAllStockWeights();
    loadAllZhBond10();

    // 获取板块驱动
    m_blockDriver = DataDriverFactory::getBlockDriver(blockParam);

    // 获取K线数据驱动并预加载指定的数据
    HKU_INFO("Loading KData...");
    std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();

    setKDataDriver(DataDriverFactory::getKDataDriverPool(m_kdataDriverParam));

    // 加载 block，须在 stock 的 kdatadriver 被设置之后调用
    m_blockDriver->load();

    // 加载 K 线至缓存
    loadAllKData();

    // add special Market, for temp csv file
    m_marketInfoDict["TMP"] =
      MarketInfo("TMP", "Temp Csv file", "temp load from csv file", "000001", Null<Datetime>(),
                 TimeDelta(0), TimeDelta(0), TimeDelta(0), TimeDelta(0));

    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start_time;
    HKU_INFO("{:<.2f}s Loaded Data.", sec.count());
    m_initializing = false;
}

void StockManager::setKDataDriver(const KDataDriverConnectPoolPtr& driver) {
    for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
        if (iter->second.market() == "TMP")
            continue;
        iter->second.setKDataDriver(driver);
    }
}

void StockManager::loadAllKData() {
    auto driver = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
    HKU_ERROR_IF_RETURN(!driver, void(), "kdata driver is null!");

    if (m_kdataDriverParam != driver->getPrototype()->getParameter()) {
        m_kdataDriverParam = driver->getPrototype()->getParameter();
    }

    bool preload_day = m_preloadParam.tryGet<bool>("day", false);
    HKU_INFO_IF(preload_day, "Preloading all day kdata to buffer!");

    bool preload_week = m_preloadParam.tryGet<bool>("week", false);
    HKU_INFO_IF(preload_week, "Preloading all week kdata to buffer!");

    bool preload_month = m_preloadParam.tryGet<bool>("month", false);
    HKU_INFO_IF(preload_month, "Preloading all month kdata to buffer!");

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

    bool preload_hour2 = m_preloadParam.tryGet<bool>("hour2", false);
    HKU_INFO_IF(preload_hour2, "Preloading all 120 min kdata to buffer!");

    if (!driver->getPrototype()->canParallelLoad()) {
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
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
            if (preload_hour2)
                iter->second.loadKDataToBuffer(KQuery::HOUR2);
        }

    } else {
        // 异步并行加载
        auto& tg = *getGlobalTaskGroup();
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
            if (preload_day)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::DAY); });
            if (preload_week)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::WEEK); });
            if (preload_month)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::MONTH); });
            if (preload_quarter)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::QUARTER); });
            if (preload_halfyear)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::HALFYEAR); });
            if (preload_year)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::YEAR); });
            if (preload_min)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::MIN); });
            if (preload_min5)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::MIN5); });
            if (preload_min15)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::MIN15); });
            if (preload_min30)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::MIN30); });
            if (preload_min60)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::MIN60); });
            if (preload_hour2)
                tg.submit([=]() mutable { iter->second.loadKDataToBuffer(KQuery::HOUR2); });
        }
    }

    initInnerTask();
}

void StockManager::reload() {
    loadAllHolidays();

    loadAllMarketInfos();
    loadAllStockTypeInfo();
    loadAllStocks();
    loadAllStockWeights();
    loadAllZhBond10();

    m_blockDriver->load();

    HKU_INFO("start reload kdata to buffer");
    std::vector<Stock> can_not_parallel_stk_list;  // 记录不支持并行加载的Stock
    {
        auto* tg = getGlobalTaskGroup();
        std::lock_guard<std::mutex> lock(*m_stockDict_mutex);
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
            auto driver = iter->second.getKDataDirver();
            if (!driver->getPrototype()->canParallelLoad()) {
                can_not_parallel_stk_list.push_back(iter->second);
                continue;
            }

            auto& ktype_list = KQuery::getAllKType();
            for (auto& ktype : ktype_list) {
                if (iter->second.isBuffer(ktype)) {
                    tg->submit([=]() mutable {
                        Stock& stk = iter->second;
                        stk.loadKDataToBuffer(ktype);
                    });
                }
            }
        }
    }

    for (auto& stk : can_not_parallel_stk_list) {
        const auto& ktype_list = KQuery::getAllKType();
        for (const auto& ktype : ktype_list) {
            if (stk.isBuffer(ktype)) {
                stk.loadKDataToBuffer(ktype);
            }
        }
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
    std::lock_guard<std::mutex> lock(*m_stockDict_mutex);
    auto iter = m_stockDict.find(query_str);
    return (iter != m_stockDict.end()) ? iter->second : result;
}

StockList StockManager::getStockList(std::function<bool(const Stock&)>&& filter) const {
    StockList ret;
    ret.reserve(m_stockDict.size());
    std::lock_guard<std::mutex> lock(*m_stockDict_mutex);
    auto iter = m_stockDict.begin();
    if (filter) {
        for (; iter != m_stockDict.end(); ++iter) {
            if (filter(iter->second)) {
                ret.emplace_back(iter->second);
            }
        }
    } else {
        for (; iter != m_stockDict.end(); ++iter) {
            ret.emplace_back(iter->second);
        }
    }
    return ret;
}

MarketInfo StockManager::getMarketInfo(const string& market) const {
    MarketInfo result;
    string market_tmp = market;
    to_upper(market_tmp);

    std::lock_guard<std::mutex> lock(*m_marketInfoDict_mutex);
    auto iter = m_marketInfoDict.find(market_tmp);
    if (iter != m_marketInfoDict.end()) {
        result = iter->second;
    } else {
        result = m_baseInfoDriver->getMarketInfo(market_tmp);
        if (result != Null<MarketInfo>()) {
            m_marketInfoDict[market_tmp] = result;
        }
    }
    return result;
}

StockTypeInfo StockManager::getStockTypeInfo(uint32_t type) const {
    StockTypeInfo result;
    std::lock_guard<std::mutex> lock(*m_stockTypeInfo_mutex);
    auto iter = m_stockTypeInfo.find(type);
    if (iter != m_stockTypeInfo.end()) {
        result = iter->second;
    } else {
        result = m_baseInfoDriver->getStockTypeInfo(type);
        if (result != Null<StockTypeInfo>()) {
            m_stockTypeInfo[type] = result;
        }
    }
    return result;
}

MarketList StockManager::getAllMarket() const {
    MarketList result;
    std::lock_guard<std::mutex> lock(*m_marketInfoDict_mutex);
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

const ZhBond10List& StockManager::getZhBond10() const {
    return m_zh_bond10;
}

Stock StockManager::addTempCsvStock(const string& code, const string& day_filename,
                                    const string& min_filename, price_t tick, price_t tickValue,
                                    int precision, size_t minTradeNumber, size_t maxTradeNumber) {
    string new_code(code);
    to_upper(new_code);
    Stock result("TMP", new_code, day_filename, STOCKTYPE_TMP, true, Datetime(199901010000),
                 Null<Datetime>(), tick, tickValue, precision, minTradeNumber, maxTradeNumber);

    Parameter param;
    param.set<string>("type", "TMPCSV");
    auto driver_pool = DataDriverFactory::getKDataDriverPool(param);
    auto driver = driver_pool->getPrototype();
    KDataTempCsvDriver* p = dynamic_cast<KDataTempCsvDriver*>(driver.get());
    p->setDayFileName(day_filename);
    p->setMinFileName(min_filename);
    result.setKDataDriver(driver_pool);
    const auto& preload_param = getPreloadParameter();
    if (preload_param.tryGet<bool>("day", true)) {
        result.loadKDataToBuffer(KQuery::DAY);
    }
    if (preload_param.tryGet<bool>("min", false)) {
        result.loadKDataToBuffer(KQuery::MIN);
    }
    return addStock(result) ? result : Null<Stock>();
}

void StockManager::removeTempCsvStock(const string& code) {
    string query_str = "TMP" + code;
    to_upper(query_str);
    auto iter = m_stockDict.find(query_str);
    if (iter != m_stockDict.end()) {
        m_stockDict.erase(iter);
    }
}

bool StockManager::addStock(const Stock& stock) {
    string market_code(stock.market_code());
    to_upper(market_code);
    std::lock_guard<std::mutex> lock(*m_stockDict_mutex);
    HKU_ERROR_IF_RETURN(m_stockDict.find(market_code) != m_stockDict.end(), false,
                        "The stock had exist! {}", market_code);
    m_stockDict[market_code] = stock;
    return true;
}

void StockManager::loadAllStocks() {
    HKU_INFO("Loading stock information...");
    vector<StockInfo> stockInfos;
    if (m_context.isAll()) {
        stockInfos = m_baseInfoDriver->getAllStockInfo();
    } else {
        const vector<string>& context_stock_code_list = m_context.getStockCodeList();
        auto all_market = getAllMarket();
        for (auto stkcode : context_stock_code_list) {
            to_upper(stkcode);
            bool find = false;
            for (auto& market : all_market) {
                auto pos = stkcode.find(market);
                if (pos != string::npos && market.size() <= stkcode.size()) {
                    string stk_market = stkcode.substr(pos, market.size());
                    string stk_code = stkcode.substr(market.size(), stkcode.size());
                    stockInfos.push_back(m_baseInfoDriver->getStockInfo(stk_market, stk_code));
                    find = true;
                    break;
                }
            }
            HKU_WARN_IF(!find, "Invalid stock code: {}", stkcode);
        }
    }

    std::lock_guard<std::mutex> lock(*m_stockDict_mutex);
    for (auto& info : stockInfos) {
        Datetime startDate, endDate;
        try {
            startDate = Datetime(info.startDate * 10000LL);
        } catch (...) {
            startDate = Null<Datetime>();
        }
        try {
            endDate = Datetime(info.endDate * 10000LL);
        } catch (...) {
            endDate = Null<Datetime>();
        }
        Stock _stock(info.market, info.code, info.name, info.type, info.valid, startDate, endDate,
                     info.tick, info.tickValue, info.precision, info.minTradeNumber,
                     info.maxTradeNumber);
        string market_code = _stock.market_code();
        ;
        to_upper(market_code);
        auto iter = m_stockDict.find(market_code);
        if (iter == m_stockDict.end()) {
            m_stockDict[market_code] = _stock;
        } else {
            Stock& stock = iter->second;
            if (!stock.m_data) {
                stock.m_data = shared_ptr<Stock::Data>(
                  new Stock::Data(info.market, info.code, info.name, info.type, info.valid,
                                  startDate, endDate, info.tick, info.tickValue, info.precision,
                                  info.minTradeNumber, info.maxTradeNumber));
            } else {
                stock.m_data->m_market = info.market;
                stock.m_data->m_code = info.code;
                stock.m_data->m_name = info.name;
                stock.m_data->m_type = info.type;
                stock.m_data->m_valid = info.valid;
                stock.m_data->m_startDate = startDate;
                stock.m_data->m_lastDate = endDate;
                stock.m_data->m_tick = info.tick;
                stock.m_data->m_tickValue = info.tickValue;
                stock.m_data->m_precision = info.precision;
                stock.m_data->m_minTradeNumber = info.minTradeNumber;
                stock.m_data->m_maxTradeNumber = info.maxTradeNumber;
            }
        }
    }
}

void StockManager::loadAllMarketInfos() {
    HKU_INFO("Loading market information...");
    auto marketInfos = m_baseInfoDriver->getAllMarketInfo();
    std::lock_guard<std::mutex> lock(*m_marketInfoDict_mutex);
    m_marketInfoDict.clear();
    m_marketInfoDict.reserve(marketInfos.size());
    for (auto& marketInfo : marketInfos) {
        string market = marketInfo.market();
        to_upper(market);
        m_marketInfoDict[market] = marketInfo;
    }
}

void StockManager::loadAllStockTypeInfo() {
    HKU_INFO("Loading stock type information...");
    auto stkTypeInfos = m_baseInfoDriver->getAllStockTypeInfo();
    std::lock_guard<std::mutex> lock(*m_stockTypeInfo_mutex);
    m_stockTypeInfo.clear();
    m_stockTypeInfo.reserve(stkTypeInfos.size());
    for (auto& stkTypeInfo : stkTypeInfos) {
        m_stockTypeInfo[stkTypeInfo.type()] = stkTypeInfo;
    }
}

void StockManager::loadAllHolidays() {
    auto holidays = m_baseInfoDriver->getAllHolidays();
    std::lock_guard<std::mutex> lock(*m_holidays_mutex);
    m_holidays = std::move(holidays);
}

void StockManager::loadAllStockWeights() {
    HKU_INFO("Loading stock weight...");
    auto all_stkweight_dict = m_baseInfoDriver->getAllStockWeightList();
    std::lock_guard<std::mutex> lock1(*m_stockDict_mutex);
    for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
        auto weight_iter = all_stkweight_dict.find(iter->first);
        if (weight_iter != all_stkweight_dict.end()) {
            Stock& stock = iter->second;
            std::lock_guard<std::mutex> lock2(stock.m_data->m_weight_mutex);
            stock.m_data->m_weightList.swap(weight_iter->second);
        }
    }
}

void StockManager::loadAllZhBond10() {
    m_zh_bond10 = m_baseInfoDriver->getAllZhBond10();
}

}  // namespace hku
