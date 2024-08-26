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

#include "hikyuu/utilities/ini_parser/IniParser.h"
#include "hikyuu/utilities/thread/ThreadPool.h"
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

    // 防止重复 init
    if (m_thread_id != std::thread::id()) {
        return;
    }
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
    loadHistoryFinanceField();

    // 获取板块驱动
    m_blockDriver = DataDriverFactory::getBlockDriver(blockParam);

    // 获取K线数据驱动并预加载指定的数据
    HKU_INFO("Loading KData...");
    std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();

    auto driver = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
    HKU_CHECK(driver, "driver is null!");
    if (m_kdataDriverParam != driver->getPrototype()->getParameter()) {
        m_kdataDriverParam = driver->getPrototype()->getParameter();
    }
    setKDataDriver(driver);

    // 加载 block，须在 stock 的 kdatadriver 被设置之后调用
    m_blockDriver->load();

    // 加载 K 线至缓存
    loadAllKData();

    // 加载历史财务信息
    loadHistoryFinance();

    initInnerTask();

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
    const auto& ktypes = KQuery::getAllKType();
    vector<string> low_ktypes;
    low_ktypes.reserve(ktypes.size());
    for (const auto& ktype : ktypes) {
        auto& back = low_ktypes.emplace_back(ktype);
        to_lower(back);
        HKU_INFO_IF(m_preloadParam.tryGet<bool>(back, false), "Preloading all {} kdata to buffer!",
                    back);
    }

    auto driver = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
    if (!driver->getPrototype()->canParallelLoad()) {
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
            for (size_t i = 0, len = ktypes.size(); i < len; i++) {
                const auto& low_ktype = low_ktypes[i];
                if (m_preloadParam.tryGet<bool>(low_ktype, false)) {
                    iter->second.loadKDataToBuffer(ktypes[i]);
                }
            }
        }

    } else {
        // 异步并行加载
        auto* tg = getGlobalTaskGroup();
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
            for (size_t i = 0, len = ktypes.size(); i < len; i++) {
                const auto& low_ktype = low_ktypes[i];
                if (m_preloadParam.tryGet<bool>(low_ktype, false)) {
                    tg->submit(
                      [=, ktype = ktypes[i]]() mutable { iter->second.loadKDataToBuffer(ktype); });
                }
            }
        }
    }
}

void StockManager::reload() {
    HKU_IF_RETURN(m_initializing, void());
    m_initializing = true;

    loadAllHolidays();
    loadAllMarketInfos();
    loadAllStockTypeInfo();
    loadAllStocks();
    loadAllStockWeights();
    loadAllZhBond10();
    loadHistoryFinanceField();

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

    loadHistoryFinance();
    m_initializing = false;
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

BlockList StockManager::getStockBelongs(const Stock& stk, const string& category) {
    return m_blockDriver ? m_blockDriver->getStockBelongs(stk, category) : BlockList();
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
    result.loadKDataToBuffer(KQuery::DAY);
    result.loadKDataToBuffer(KQuery::MIN);
    return addStock(result) ? result : Null<Stock>();
}

void StockManager::removeTempCsvStock(const string& code) {
    removeStock(fmt::format("TMP{}", code));
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

void StockManager::removeStock(const string& market_code) {
    string n_market_code(market_code);
    to_upper(n_market_code);
    std::lock_guard<std::mutex> lock(*m_stockDict_mutex);
    auto iter = m_stockDict.find(n_market_code);
    if (iter != m_stockDict.end()) {
        m_stockDict.erase(iter);
    }
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
                stock.m_data->m_history_finance_ready = false;
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
    if (m_context.isAll()) {
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
    } else {
        std::lock_guard<std::mutex> lock1(*m_stockDict_mutex);
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
            Stock& stock = iter->second;
            auto sw_list = m_baseInfoDriver->getStockWeightList(
              stock.market(), stock.code(), m_context.startDatetime(), Null<Datetime>());
            {
                std::lock_guard<std::mutex> lock2(stock.m_data->m_weight_mutex);
                stock.m_data->m_weightList = std::move(sw_list);
            }
        }
    }
}

void StockManager::loadAllZhBond10() {
    m_zh_bond10 = m_baseInfoDriver->getAllZhBond10();
}

void StockManager::loadHistoryFinanceField() {
    auto fields = m_baseInfoDriver->getHistoryFinanceField();
    for (const auto& field : fields) {
        m_field_ix_to_name[field.first - 1] = field.second;
        m_field_name_to_ix[field.second] = field.first - 1;
    }
}

vector<std::pair<size_t, string>> StockManager::getHistoryFinanceAllFields() const {
    vector<std::pair<size_t, string>> ret;
    for (auto iter = m_field_ix_to_name.begin(); iter != m_field_ix_to_name.end(); ++iter) {
        ret.emplace_back(iter->first, iter->second);
    }
    std::sort(ret.begin(), ret.end(),
              [](const std::pair<size_t, string>& a, const std::pair<size_t, string>& b) {
                  return a.first < b.first;
              });
    return ret;
}

void StockManager::loadHistoryFinance() {
    auto* tg = getGlobalTaskGroup();
    std::lock_guard<std::mutex> lock1(*m_stockDict_mutex);
    for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
        tg->submit([=]() { iter->second.getHistoryFinance(); });
    }
}

}  // namespace hku
