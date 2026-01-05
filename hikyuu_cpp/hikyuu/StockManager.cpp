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
#include "plugin/interface/plugins.h"
#include "plugin/device.h"
#include "plugin/hkuextra.h"

namespace hku {
StockManager* StockManager::m_sm = nullptr;

void StockManager::quit() {
    if (m_sm) {
        delete m_sm;
        m_sm = nullptr;
    }
}

StockManager::StockManager() {
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

static void registerPredefinedExtraKType() {
    if (isValidLicense()) {
        registerExtraKType(KQuery::DAY3, KQuery::DAY, 3);
        registerExtraKType(KQuery::DAY5, KQuery::DAY, 5);
        registerExtraKType(KQuery::DAY7, KQuery::DAY, 7);

        registerExtraKType(KQuery::MIN3, KQuery::MIN, 3, [](const Datetime& d) {
            auto m = d.minute();
            if (m % 3 == 0) {
                return d;
            }
            m = (m / 3 + 1) * 3;
            return Datetime(d.year(), d.month(), d.day(), d.hour(), m);
        });
    }
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

    if (m_i18n_path.empty()) {
        loadLocalLanguage(fmt::format("{}/i18n", getDllSelfDir()));
    } else {
        loadLocalLanguage(m_i18n_path);
    }

    m_baseInfoDriverParam = baseInfoParam;
    m_blockDriverParam = blockParam;
    m_kdataDriverParam = kdataParam;
    m_preloadParam = preloadParam;
    m_hikyuuParam = hikyuuParam;
    m_context = context;

    // 获取路径信息
    m_tmpdir = hikyuuParam.tryGet<string>("tmpdir", ".");
    m_datadir = hikyuuParam.tryGet<string>("datadir", ".");

    // 设置插件路径
    auto plugin_path = getPluginPath();
    if (plugin_path.empty() || plugin_path == ".") {
        m_plugin_manager.pluginPath(m_hikyuuParam.tryGet<string>(
          "plugindir", fmt::format("{}/.hikyuu/plugin", getUserDir())));
    }
    HKU_INFO(htr("Plugin path: {}", getPluginPath()));

    // 注册扩展K线处理
    registerPredefinedExtraKType();

    string basedrivername = m_baseInfoDriverParam.tryGet<string>("type", "");
    to_lower(basedrivername);
    if (basedrivername == "clickhouse") {
        auto* plugin = getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
        HKU_CHECK(plugin, "{}: {}", htr("Can not find plugin"), HKU_PLUGIN_CLICKHOUSE_DRIVER);
        auto driver = plugin->getBaseInfoDriver();
        HKU_CHECK(driver, "{}", htr("Can not get clickhouse driver! Check your license!"));
        DataDriverFactory::regBaseInfoDriver(driver);
    }

    string kdrivername = m_kdataDriverParam.tryGet<string>("type", "");
    to_lower(kdrivername);
    if (kdrivername == "clickhouse") {
        auto* plugin = getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
        HKU_CHECK(plugin, "{}: {}!", htr("Can not find plugin"), HKU_PLUGIN_CLICKHOUSE_DRIVER);
        auto kdriver = plugin->getKDataDriver();
        HKU_CHECK(kdriver, "{}", htr("Can not get clickhouse driver! Check your license!"));
        DataDriverFactory::regKDataDriver(kdriver);
    }

    string blockdrivername = m_blockDriverParam.tryGet<string>("type", "");
    to_lower(blockdrivername);
    if (blockdrivername == "clickhouse") {
        auto* plugin = getPlugin<DataDriverPluginInterface>(HKU_PLUGIN_CLICKHOUSE_DRIVER);
        HKU_CHECK(plugin, "{}: {}!", htr("Can not find plugin"), HKU_PLUGIN_CLICKHOUSE_DRIVER);
        auto driver = plugin->getBlockInfoDriver();
        HKU_CHECK(driver, "{}", htr("Can not get clickhouse driver! Check your license!"));
        DataDriverFactory::regBlockDriver(driver);
    }

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

    // 加载数据
    loadData();

    // 初始化内部定时任务（重加载）
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

    HKU_INFO(htr("Loading block..."));
    m_blockDriver->load();

    // 获取K线数据驱动并预加载指定的数据
    HKU_INFO(htr("Loading KData..."));

    // 加载K线及历史财务信息
    loadAllKData();

    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start_time;
    auto seconds = sec.count();
    HKU_INFO(htr("{:<.2f}s Loaded Data.", seconds));
}

void StockManager::loadAllKData() {
    // 按 K 线类型控制加载顺序
    vector<KQuery::KType> ktypes;
    vector<string> low_ktypes;

    // 如果上下文指定了 ktype list，则按上下文指定的 ktype 顺序加载，否则按默认顺序加载
    const auto& context_ktypes = m_context.getKTypeList();
    if (context_ktypes.empty()) {
        ktypes = KQuery::getBaseKTypeList();

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
            m_preloadParam.set<int64_t>(preload_key, context_iter->second);
        }

        int64_t preload_max_num = m_preloadParam.tryGet<int64_t>(preload_key, 0);
        if (preload_max_num <= 0) {
            preload_max_num = std::numeric_limits<int64_t>::max();
            m_preloadParam.set<int64_t>(preload_key, preload_max_num);
            HKU_INFO_IF(m_preloadParam.tryGet<bool>(back, false),
                        htr("Preloading {} kdata to buffer (max: no limit)!", back));
        } else {
            HKU_INFO_IF(m_preloadParam.tryGet<bool>(back, false),
                        htr("Preloading {} kdata to buffer (max: {})!", back, preload_max_num));
        }
    }

    bool lazy_preload = m_hikyuuParam.tryGet<bool>("lazy_preload", false);
    HKU_INFO_IF(lazy_preload && canLazyLoad(KQuery::MIN), htr("Use lazy preload!"));

    // 先加载同类K线
    auto driver = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
    if (!driver->getPrototype()->canParallelLoad()) {
        for (size_t i = 0, len = ktypes.size(); i < len; i++) {
            if (m_cancel_load) {
                break;
            }
            if (canLazyLoad(ktypes[i])) {
                continue;
            }
            for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
                if (m_cancel_load) {
                    break;
                }
                const auto& low_ktype = low_ktypes[i];
                if (m_preloadParam.tryGet<bool>(low_ktype, false)) {
                    iter->second.loadKDataToBuffer(ktypes[i]);
                }
            }
        }

        if (!m_cancel_load && m_hikyuuParam.tryGet<bool>("load_history_finance", true)) {
            ThreadPool tg;
            for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
                if (m_cancel_load) {
                    break;
                }
                tg.submit([stk = iter->second, this]() {
                    HKU_IF_RETURN(m_cancel_load, void());
                    stk.getHistoryFinance();
                });
            }
            tg.join();
        }

        m_data_ready = true;

    } else {
        // 异步并行加载
        std::thread t = std::thread([this, ktypes, low_ktypes]() {
            auto loaded_codes = tryLoadAllKDataFromColumnFirst(ktypes);

            // 加载其他证券K线(可能不同不同K线驱动的证券)
            this->m_load_tg = std::make_unique<ThreadPool>();
            for (size_t i = 0, len = ktypes.size(); i < len; i++) {
                if (m_cancel_load) {
                    break;
                }
                if (canLazyLoad(ktypes[i])) {
                    continue;
                }
                std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
                for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
                    if (m_cancel_load) {
                        break;
                    }
                    if (loaded_codes.find(iter->first) != loaded_codes.end()) {
                        continue;
                    }
                    if (m_preloadParam.tryGet<bool>(low_ktypes[i], false)) {
                        m_load_tg->submit(
                          [this, stk = iter->second, ktype = std::move(ktypes[i])]() mutable {
                              HKU_IF_RETURN(m_cancel_load, void());
                              stk.loadKDataToBuffer(ktype);
                          });
                    }
                }
            }

            if (!m_cancel_load && m_hikyuuParam.tryGet<bool>("load_history_finance", true)) {
                std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
                for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
                    if (m_cancel_load) {
                        break;
                    }
                    if (loaded_codes.find(iter->first) != loaded_codes.end()) {
                        continue;
                    }
                    m_load_tg->submit([this, stk = iter->second]() {
                        HKU_IF_RETURN(m_cancel_load, void());
                        stk.getHistoryFinance();
                    });
                }
            }

            m_load_tg->join();
            m_load_tg.reset();
            m_data_ready = true;
        });
        t.detach();
    }
}

std::unordered_set<string> StockManager::tryLoadAllKDataFromColumnFirst(
  const vector<KQuery::KType>& ktypes) {
    std::unordered_set<string> loaded_codes;
    HKU_IF_RETURN(!m_context.isAll(), loaded_codes);
    auto driver = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
    HKU_IF_RETURN(!driver || !driver->getPrototype()->isColumnFirst(), loaded_codes);

    // 尝试优先加载 SH000001 K线
    Stock sh000001;
    {
        std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
        auto sh000001_iter = m_stockDict.find("SH000001");
        if (sh000001_iter != m_stockDict.end()) {
            sh000001 = sh000001_iter->second;
        }
    }

    HKU_IF_RETURN(sh000001.isNull(), loaded_codes);

    for (size_t i = 0, len = ktypes.size(); i < len; i++) {
        if (m_cancel_load) {
            break;
        }
        auto low_ktype = ktypes[i];
        to_lower(low_ktype);
        if (m_preloadParam.tryGet<bool>(low_ktype, false)) {
            sh000001.loadKDataToBuffer(ktypes[i]);
        }
    }

    HKU_IF_RETURN(m_cancel_load, loaded_codes);

    // 主要受带宽限制，无需多线程
    for (size_t i = 0, len = ktypes.size(); i < len; i++) {
        if (m_cancel_load) {
            break;
        }

        if (canLazyLoad(ktypes[i])) {
            continue;
        }

        auto low_ktype = ktypes[i];
        to_lower(low_ktype);
        if (!m_preloadParam.tryGet<bool>(low_ktype, false)) {
            continue;
        }

        auto k = sh000001.getKRecord(0, ktypes[i]);
        if (k.isValid()) {
            auto datas =
              driver->getConnect()->getAllKRecordList(ktypes[i], k.datetime, m_cancel_load);
            if (!datas.empty() && !m_cancel_load) {
                std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
                for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
                    if (m_cancel_load) {
                        break;
                    }
                    auto date_iter = datas.find(iter->second.market_code());
                    if (date_iter != datas.end()) {
                        iter->second.loadKDataToBufferFromKRecordList(ktypes[i],
                                                                      std::move(date_iter->second));
                        loaded_codes.insert(iter->second.market_code());
                    }
                }
            }
        }
    }

    if (!m_cancel_load && m_hikyuuParam.tryGet<bool>("load_history_finance", true)) {
        auto finances = m_baseInfoDriver->getAllHistoryFinance(m_cancel_load);
        if (!finances.empty() && !m_cancel_load) {
            std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
            for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
                if (m_cancel_load) {
                    break;
                }
                auto finance_iter = finances.find(iter->second.market_code());
                if (finance_iter != finances.end()) {
                    iter->second.setHistoryFinance(std::move(finance_iter->second));
                }
            }
        }
    }

    return loaded_codes;
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

MarketInfo StockManager::getMarketInfo(const string& market) const noexcept {
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

Stock StockManager::getMarketStock(const string& market) const {
    auto market_info = getMarketInfo(market);
    return getStock(fmt::format("{}{}", market_info.market(), market_info.code()));
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

vector<StockTypeInfo> StockManager::getStockTypeInfoList() const {
    vector<StockTypeInfo> result;
    std::shared_lock<std::shared_mutex> lock(*m_stockTypeInfo_mutex);
    result.reserve(m_stockTypeInfo.size());
    for (auto& item : m_stockTypeInfo) {
        result.push_back(item.second);
    }
    return result;
}

StringList StockManager::getAllMarket() const {
    StringList result;
    std::shared_lock<std::shared_mutex> lock(*m_marketInfoDict_mutex);
    auto iter = m_marketInfoDict.begin();
    for (; iter != m_marketInfoDict.end(); ++iter) {
        result.push_back(iter->first);
    }
    return result;
}

StringList StockManager::getAllCategory() {
    return m_blockDriver ? m_blockDriver->getAllCategory() : StringList();
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

BlockList StockManager::getBlockListByIndexStock(const Stock& stk) {
    BlockList all = getBlockList();
    BlockList result;
    for (auto& blk : all) {
        if (blk.getIndexStock() == stk) {
            result.push_back(blk);
        }
    }
    return result;
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
    HKU_IF_RETURN(d.dayOfWeek() == 0 || d.dayOfWeek() == 6, true);
    std::shared_lock<std::shared_mutex> lock(*m_holidays_mutex);
    return m_holidays.count(d.startOfDay());
}

bool StockManager::isTradingHours(const Datetime& d, const string& market) const {
    HKU_IF_RETURN(isHoliday(d), false);
    auto hour = d - d.startOfDay();
    MarketInfo marketinfo = getMarketInfo(market);
    HKU_CHECK(marketinfo != Null<MarketInfo>(), "{}: {}!", htr("Not found market info"), market);
    HKU_IF_RETURN((hour >= marketinfo.openTime1() && hour <= marketinfo.closeTime1()) ||
                    (hour >= marketinfo.openTime2() && hour <= marketinfo.closeTime2()),
                  true);
    return false;
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
    HKU_INFO(htr("Loading stock information..."));
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

    auto base_ktypes = KQuery::getBaseKTypeList();
    vector<KQuery::KType> preload_ktypes;
    for (auto& ktype : base_ktypes) {
        auto nktype = ktype;
        to_lower(nktype);
        if (m_preloadParam.tryGet<bool>(nktype, false)) {
            preload_ktypes.push_back(ktype);
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
            _stock.setPreload(preload_ktypes);
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
                // 强制释放所有已缓存K线数据
                stock.m_data->m_lastUpdate.clear();
                for (const auto& ktype : base_ktypes) {
                    stock.releaseKDataBuffer(ktype);
                    stock.m_data->m_lastUpdate[ktype] = Datetime::min();
                }
                auto ktype_list = KQuery::getExtraKTypeList();
                for (const auto& ktype : ktype_list) {
                    stock.m_data->m_lastUpdate[ktype] = Datetime::min();
                }
            }
            stock.setPreload(preload_ktypes);
            if (!stock.getKDataDirver()) {
                stock.setKDataDriver(kdriver);
            }
        }
    }
}

void StockManager::loadAllMarketInfos() {
    HKU_INFO(htr("Loading market information..."));
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
    HKU_INFO(htr("Loading stock type information..."));
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
    HKU_INFO(htr("Loading stock weight..."));
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

}  // namespace hku
