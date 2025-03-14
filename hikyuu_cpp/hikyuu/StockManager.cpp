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

StockManager::StockManager() : m_initializing(false), m_data_ready(false) {
    m_stockDict_mutex = new std::shared_mutex;
    m_marketInfoDict_mutex = new std::shared_mutex;
    m_stockTypeInfo_mutex = new std::shared_mutex;
    m_holidays_mutex = new std::shared_mutex;
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
    HKU_CHECK(!context.empty(), "No stock code list is included in the context!");

    m_baseInfoDriverParam = baseInfoParam;
    m_blockDriverParam = blockParam;
    m_kdataDriverParam = kdataParam;
    m_preloadParam = preloadParam;
    m_hikyuuParam = hikyuuParam;
    m_context = context;

    // 获取路径信息
    m_tmpdir = hikyuuParam.tryGet<string>("tmpdir", ".");
    m_datadir = hikyuuParam.tryGet<string>("datadir", ".");

    // 加载证券基本信息
    m_baseInfoDriver = DataDriverFactory::getBaseInfoDriver(baseInfoParam);
    HKU_CHECK(m_baseInfoDriver, "Failed get base info driver!");

    // 获取板块驱动
    m_blockDriver = DataDriverFactory::getBlockDriver(blockParam);

    auto driver = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
    HKU_CHECK(driver, "driver is null!");
    if (m_kdataDriverParam != driver->getPrototype()->getParameter()) {
        m_kdataDriverParam = driver->getPrototype()->getParameter();
    }

    loadData();
    initInnerTask();

    m_initializing = false;
}

void StockManager::loadData() {
    std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
    m_data_ready = false;

    loadAllHolidays();
    loadAllMarketInfos();
    loadAllStockTypeInfo();
    loadAllStocks();
    loadAllStockWeights();
    loadAllZhBond10();
    loadHistoryFinanceField();

    HKU_INFO("Loading block...");
    m_blockDriver->load();

    // 获取K线数据驱动并预加载指定的数据
    HKU_INFO("Loading KData...");

    auto driver = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);

    // 加载K线及历史财务信息
    loadAllKData();

    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start_time;
    HKU_INFO("{:<.2f}s Loaded Data.", sec.count());
}

void StockManager::loadAllKData() {
    // 按 K 线类型控制加载顺序
    vector<KQuery::KType> ktypes;
    vector<string> low_ktypes;

    // 如果上下文指定了 ktype list，则按上下文指定的 ktype 顺序加载，否则按默认顺序加载
    const auto& context_ktypes = m_context.getKTypeList();
    if (context_ktypes.empty()) {
        ktypes = KQuery::getAllKType();

    } else {
        // 使用上下文预加载参数覆盖全局预加载参数
        ktypes = context_ktypes;
        for (const auto& ktype : ktypes) {
            auto low_ktype = ktype;
            to_lower(low_ktype);
            m_preloadParam.set<bool>(low_ktype, true);
        }
    }

    const auto& context_preload_num = m_context.getPreloadNum();
    low_ktypes.reserve(ktypes.size());
    for (const auto& ktype : ktypes) {
        auto& back = low_ktypes.emplace_back(ktype);
        to_lower(back);

        // 判断上下文是否指定了预加载数量，如果指定了，则覆盖默认值
        string preload_key = fmt::format("{}_max", back);
        auto context_iter = context_preload_num.find(preload_key);
        if (context_iter != context_preload_num.end()) {
            m_preloadParam.set<int>(preload_key, context_iter->second);
        }

        HKU_INFO_IF(m_preloadParam.tryGet<bool>(back, false),
                    "Preloading {} kdata to buffer (max: {})!", back,
                    m_preloadParam.tryGet<int>(preload_key, 0));
    }

    // 先加载同类K线
    auto driver = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
    if (!driver->getPrototype()->canParallelLoad()) {
        for (size_t i = 0, len = ktypes.size(); i < len; i++) {
            for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
                const auto& low_ktype = low_ktypes[i];
                if (m_preloadParam.tryGet<bool>(low_ktype, false)) {
                    iter->second.loadKDataToBuffer(ktypes[i]);
                }
            }
        }

        if (m_hikyuuParam.tryGet<bool>("load_history_finance", true)) {
            ThreadPool tg;
            for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
                tg.submit([stk = iter->second]() { stk.getHistoryFinance(); });
            }
            tg.join();
        }

        m_data_ready = true;

    } else {
        // 异步并行加载
        std::thread t = std::thread([this, ktypes, low_ktypes]() {
            this->m_load_tg = std::make_unique<ThreadPool>();
            for (size_t i = 0, len = ktypes.size(); i < len; i++) {
                std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
                for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
                    if (m_preloadParam.tryGet<bool>(low_ktypes[i], false)) {
                        m_load_tg->submit(
                          [stk = iter->second, ktype = std::move(ktypes[i])]() mutable {
                              stk.loadKDataToBuffer(ktype);
                          });
                    }
                }
            }

            if (m_hikyuuParam.tryGet<bool>("load_history_finance", true)) {
                std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
                for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
                    m_load_tg->submit([stk = iter->second]() { stk.getHistoryFinance(); });
                }
            }

            m_load_tg->join();
            m_load_tg.reset();
            m_data_ready = true;
        });
        t.detach();
    }
}

void StockManager::reload() {
    HKU_IF_RETURN(m_initializing, void());
    m_initializing = true;

    HKU_INFO("start reload ...");
    loadData();
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
    size_t pos = query_str.find('.');
    if (pos != string::npos) {
        // 后缀表示法
        std::string suffix = query_str.substr(pos + 1);
        std::string prefix = query_str.substr(0, pos);
        query_str = suffix + prefix;
    }
    std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
    auto iter = m_stockDict.find(query_str);
    return (iter != m_stockDict.end()) ? iter->second : result;
}

StockList StockManager::getStockList(std::function<bool(const Stock&)>&& filter) const {
    StockList ret;
    std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
    ret.reserve(m_stockDict.size());
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

    std::shared_lock<std::shared_mutex> lock(*m_marketInfoDict_mutex);
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
    std::shared_lock<std::shared_mutex> lock(*m_stockTypeInfo_mutex);
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
    std::shared_lock<std::shared_mutex> lock(*m_marketInfoDict_mutex);
    auto iter = m_marketInfoDict.begin();
    for (; iter != m_marketInfoDict.end(); ++iter) {
        result.push_back(iter->first);
    }
    return result;
}

Block StockManager::getBlock(const string& category, const string& name) {
    return m_blockDriver ? m_blockDriver->getBlock(category, name) : Block();
}

void StockManager::saveBlock(const Block& blk) {
    if (m_blockDriver) {
        HKU_CHECK(!blk.category().empty(), "block's category can not be empty!");
        HKU_CHECK(!blk.name().empty(), "block's name can not be empty!");
        HKU_CHECK(!blk.empty(), "Can't save empty block!");
        m_blockDriver->save(blk);
    }
}
void StockManager::removeBlock(const string& category, const string& name) {
    if (m_blockDriver) {
        m_blockDriver->remove(category, name);
    }
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

bool StockManager::isHoliday(const Datetime& d) const {
    std::shared_lock<std::shared_mutex> lock(*m_holidays_mutex);
    return m_holidays.count(d);
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
    std::unique_lock<std::shared_mutex> lock(*m_stockDict_mutex);
    HKU_ERROR_IF_RETURN(m_stockDict.find(market_code) != m_stockDict.end(), false,
                        "The stock had exist! {}", market_code);
    m_stockDict[market_code] = stock;
    return true;
}

void StockManager::removeStock(const string& market_code) {
    string n_market_code(market_code);
    to_upper(n_market_code);
    std::unique_lock<std::shared_mutex> lock(*m_stockDict_mutex);
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
        auto load_stock_code_list = m_context.getAllNeedLoadStockCodeList();
        auto all_market = getAllMarket();
        for (auto stkcode : load_stock_code_list) {
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

    auto kdriver = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);

    std::unique_lock<std::shared_mutex> lock(*m_stockDict_mutex);
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

        string market_code = fmt::format("{}{}", info.market, info.code);
        to_upper(market_code);

        auto iter = m_stockDict.find(market_code);
        if (iter == m_stockDict.end()) {
            Stock _stock(info.market, info.code, info.name, info.type, info.valid, startDate,
                         endDate, info.tick, info.tickValue, info.precision, info.minTradeNumber,
                         info.maxTradeNumber);
            _stock.setKDataDriver(kdriver);
            m_stockDict[market_code] = std::move(_stock);
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
            if (!stock.getKDataDirver()) {
                stock.setKDataDriver(kdriver);
            }
        }
    }
}

void StockManager::loadAllMarketInfos() {
    HKU_INFO("Loading market information...");
    auto marketInfos = m_baseInfoDriver->getAllMarketInfo();
    std::unique_lock<std::shared_mutex> lock(*m_marketInfoDict_mutex);
    m_marketInfoDict.clear();
    m_marketInfoDict.reserve(marketInfos.size());
    for (auto& marketInfo : marketInfos) {
        string market = marketInfo.market();
        to_upper(market);
        m_marketInfoDict[market] = marketInfo;
    }

    // add special Market, for temp csv file
    m_marketInfoDict["TMP"] =
      MarketInfo("TMP", "Temp Csv file", "temp load from csv file", "000001", Null<Datetime>(),
                 TimeDelta(0), TimeDelta(0), TimeDelta(0), TimeDelta(0));
}

void StockManager::loadAllStockTypeInfo() {
    HKU_INFO("Loading stock type information...");
    auto stkTypeInfos = m_baseInfoDriver->getAllStockTypeInfo();
    std::unique_lock<std::shared_mutex> lock(*m_stockTypeInfo_mutex);
    m_stockTypeInfo.clear();
    m_stockTypeInfo.reserve(stkTypeInfos.size());
    for (auto& stkTypeInfo : stkTypeInfos) {
        m_stockTypeInfo[stkTypeInfo.type()] = stkTypeInfo;
    }
}

void StockManager::loadAllHolidays() {
    auto holidays = m_baseInfoDriver->getAllHolidays();
    std::unique_lock<std::shared_mutex> lock(*m_holidays_mutex);
    m_holidays = std::move(holidays);
}

void StockManager::loadAllStockWeights() {
    HKU_IF_RETURN(!m_hikyuuParam.tryGet<bool>("load_stock_weight", true), void());
    HKU_INFO("Loading stock weight...");
    if (m_context.isAll()) {
        auto all_stkweight_dict = m_baseInfoDriver->getAllStockWeightList();
        std::shared_lock<std::shared_mutex> lock1(*m_stockDict_mutex);
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
            auto weight_iter = all_stkweight_dict.find(iter->first);
            if (weight_iter != all_stkweight_dict.end()) {
                Stock& stock = iter->second;
                std::lock_guard<std::mutex> lock2(stock.m_data->m_weight_mutex);
                stock.m_data->m_weightList.swap(weight_iter->second);
            }
        }
    } else {
        std::shared_lock<std::shared_mutex> lock1(*m_stockDict_mutex);
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
    if (m_hikyuuParam.tryGet<bool>("load_history_finance", true)) {
        auto fields = m_baseInfoDriver->getHistoryFinanceField();
        for (const auto& field : fields) {
            m_field_ix_to_name[field.first - 1] = field.second;
            m_field_name_to_ix[field.second] = field.first - 1;
        }
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

}  // namespace hku
