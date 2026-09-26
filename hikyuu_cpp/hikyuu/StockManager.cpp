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
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fmt/format.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "hikyuu/utilities/ini_parser/IniParser.h"
#include "hikyuu/utilities/thread/ThreadPool.h"
#include "hikyuu/utilities/thread/algorithm.h"
#include "StockManager.h"
#include "global/schedule/inner_tasks.h"
#include "plugin/interface/plugins.h"
#include "plugin/device.h"
#include "plugin/hkuextra.h"
#include "plugin/extind.h"
#include "global/sysinfo.h"
#include "data_driver/ipc/ShmClientHook.h"

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
}

StockManager::~StockManager() {
    // Wait for the background preload thread to exit first: otherwise it would still access the
    // members (m_load_tg) after their destruction, causing a UAF; at the same time it guarantees
    // that m_preload_thread is not joinable at its destruction (otherwise the std::thread
    // destructor would trigger std::terminate). Idempotent: clean() has usually joined already, so
    // calling it again here is a no-op (it also covers the destruction path without clean()).
    joinPreloadThread();
    // Unregister the shm client forwarding callbacks (breaking the reference to the plugin
    // implementation): after that the forwarding call of Stock::realtimeUpdate returns directly,
    // avoiding blocking on an already invalid connection during the exit. The server shutdown has
    // been moved to the plugin (the stopShmServer facade)
    ipc::registerShmClient(ipc::ShmClientForwarders());
    delete m_stockDict_mutex;
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
    std::lock_guard<std::mutex> lock(m_init_mutex);
    HKU_WARN_IF_RETURN(m_initializing, void(),
                       "The last initialization has not finished. Please try again later!");

    // Prevent a duplicated init
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

    // Get the path information
    m_tmpdir = hikyuuParam.tryGet<string>("tmpdir", ".");
    m_datadir = hikyuuParam.tryGet<string>("datadir", ".");

    // Set the plugin path
    auto plugin_path = getPluginPath();
    if (plugin_path.empty() || plugin_path == ".") {
        m_plugin_manager.pluginPath("./plugin");
    }
    HKU_INFO(htr("Plugin path: {}", getPluginPath()));

    // Register the extended K-line handling
    registerPredefinedExtraKType();

    global_submit_task([this]() {
        getPlugin<ExtendIndicatorsPluginInterface>(HKU_PLUGIN_EXTEND_INDICATOR);
        getPlugin<TMReportPluginInterface>(HKU_PLUGIN_TMREPORT);
    });

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

    // Load the basic security information
    m_baseInfoDriver = DataDriverFactory::getBaseInfoDriver(baseInfoParam);
    HKU_CHECK(m_baseInfoDriver, "Failed get base info driver!");

    // Get the block driver
    m_blockDriver = DataDriverFactory::getBlockDriver(blockParam);

    auto driver = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
    HKU_CHECK(driver, "driver is null!");
    if (m_kdataDriverParam != driver->getPrototype()->getParameter()) {
        m_kdataDriverParam = driver->getPrototype()->getParameter();
    }

    // The pure client negotiates the shm data service (on a successful connection it is replaced by
    // the proxy driver and the local preload is turned off; on failure it degrades to the
    // standalone mode)
    _negotiateShmServer();

    // Load the data
    loadData();

    // The basic data and the snapshot publishing are now done by the plugin subscribing to
    // LoadEvent (see design §5.2), the core library no longer notifies the readiness actively.

    // Initialize the internal scheduled task (reload)
    initInnerTask();

    m_initializing = false;
}

void StockManager::loadData() {
    std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
    m_data_ready.store(false, std::memory_order_release);

    loadAllHolidays();
    loadAllMarketInfos();
    loadAllStockTypeInfo();
    loadAllStocks();
    loadInnerBlocks();
    loadAllStockWeights();
    // The ex-rights/ex-dividend data is ready, dispatch the BASE_DATA_READY event: the plugin
    // publishes a basic information snapshot accordingly (the historical finance is rebuilt after
    // the preload thread finishes). The two publishes happen in the main thread and the preload
    // thread respectively; at this moment the historical finance has not been preloaded yet, so the
    // plugin side must publish with include_finance=false, otherwise the historical finance of
    // every security would be lazily loaded one by one (see Stock::getHistoryFinance).
    _fireLoadEvent(LoadEvent::BASE_DATA_READY);
    loadAllZhBond10();
    loadHistoryFinanceField();

    HKU_INFO(htr("Loading block..."));
    m_blockDriver->load();
    // The blocks are loaded, dispatch the BLOCKS_LOADED event: the plugin refreshes the block cache
    // of the IPC service accordingly (the former refreshBlocks)
    _fireLoadEvent(LoadEvent::BLOCKS_LOADED);

    // Get the K-line data driver and preload the given data
    HKU_INFO(htr("Loading KData..."));

    // Load the K-lines and the historical financial information
    loadAllKData();

    // Update the license expire time
    updateSysInfoExpiredTime(getExpireDate());

    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start_time;
    auto seconds = sec.count();
    HKU_INFO(htr("{:<.2f}s Loaded Data.", seconds));
}

KDataDriverConnectPoolPtr StockManager::_getKDataDriverPool() {
    if (m_ipc_kdata_pool) {
        return m_ipc_kdata_pool;
    }
    return DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
}

void StockManager::_negotiateShmServer() {
    // The master gate: when it is off nothing participates at all (no detection, no mapping, no
    // forwarding) and the behavior equals to the feature being disabled. It is off by default (the
    // process runs in the standalone mode by default); connecting to an existing service as a
    // client requires enabling it explicitly in the config
    HKU_IF_RETURN(!m_hikyuuParam.tryGet<bool>("use_shm_server", false), void());
    // This process is in the server role: it never enters the client mode (to prevent the
    // realtimeUpdate self-forwarding loop, see design §5.5)
    HKU_IF_RETURN(isShmServerRole(), void());

    // All the client capabilities (detection / connection / waiting for readiness / the proxy
    // driver / the three forwardings) are provided by the shmserver plugin; the core library only
    // depends on the single contract ShmServerPluginInterface: it does not link any implementation
    // symbol and degrades directly to the standalone mode (loading all the data by itself) when the
    // plugin is not installed or not authorized. print=false: the startup log should not produce
    // noise for the community users without the plugin. The three realtime forwardings are
    // registered by the plugin itself after a successful connect (ipc::registerShmClient); the core
    // library no longer holds the client pointer
    auto* plugin = getPlugin<ShmServerPluginInterface>(HKU_PLUGIN_SHM_SERVER, false);
    HKU_IF_RETURN(!plugin, void());

    // Connect to the existing service and wait for its data to be ready (the retries, the
    // interruption check and the forwarding registration are done inside the plugin; the client
    // never starts the service itself)
    auto wait_timeout = m_hikyuuParam.tryGet<int64_t>("shm_server_wait_timeout", 600);
    HKU_WARN_IF_RETURN(!plugin->connect(m_datadir, wait_timeout < 0 ? 0 : (uint64_t)wait_timeout),
                       void(), "Failed connect to hikyuu shm server, fallback to standalone mode!");

    // Switch to the client mode: install the proxy driver provided by the plugin and turn off the
    // local preload (an in-memory override only, the config file is not modified)
    m_ipc_client_mode = true;
    m_baseInfoDriver = plugin->createBaseInfoDriver(m_baseInfoDriver);
    m_blockDriver = plugin->createBlockDriver(m_blockDriver);
    // The whole local driver connection pool is passed in (instead of its prototype): the types not
    // preloaded by the service process and the time-sharing / tick data are served by the local
    // driver of the client directly, and a connection must be taken from the pool to avoid multiple
    // clones reusing the same connection / file handle concurrently
    auto local_pool = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
    m_ipc_kdata_pool =
      std::make_shared<KDataDriverConnectPool>(plugin->createKDataDriver(local_pool));
    for (const auto& ktype : KQuery::getBaseKTypeList()) {
        auto low_ktype = ktype;
        to_lower(low_ktype);
        m_preloadParam.set<bool>(low_ktype, false);
    }
    // The client has no preload buffer; the update is applied to the buffer by the service process
    // and mirrored to the shared memory, visible to all the clients
    HKU_INFO("Connected to hikyuu shm server: {}, running in client mode.", plugin->serverAddr());
}

bool StockManager::isIpcClientMode() const {
    return m_ipc_client_mode;
}

// ── LoadEvent event bus (the core library only dispatches and the plugin subscribes; see design
// §5.2) ─────────────────────────────────────
namespace {
// The callback container is held with new entirely and never deleted: if its member lock were
// destroyed during the static destruction, stopShmServer() would throw EINVAL while locking when
// unregistering the callbacks in clean() (the static destruction period) and the noexcept
// destruction chain would call std::terminate (the same as constraint 2 of the old §4.7). Therefore
// the container itself is also placed on the heap and never released.
struct LoadEventState {
    std::shared_mutex mutex;
    std::vector<std::pair<size_t, LoadEventCallback>> callbacks;
    size_t next_id{1};
};
LoadEventState* g_load_event = new LoadEventState;
std::atomic<bool> g_shm_server_role{false};
}  // namespace

size_t registerLoadEventCallback(LoadEventCallback&& cb) {
    std::unique_lock<std::shared_mutex> lock(g_load_event->mutex);
    size_t id = g_load_event->next_id++;
    g_load_event->callbacks.emplace_back(id, std::move(cb));
    return id;
}

void unregisterLoadEventCallback(size_t id) {
    std::unique_lock<std::shared_mutex> lock(g_load_event->mutex);
    auto& v = g_load_event->callbacks;
    v.erase(
      std::remove_if(v.begin(), v.end(),
                     [id](const std::pair<size_t, LoadEventCallback>& p) { return p.first == id; }),
      v.end());
}

void setShmServerRole(bool role) noexcept {
    g_shm_server_role.store(role, std::memory_order_relaxed);
}

bool isShmServerRole() noexcept {
    return g_shm_server_role.load(std::memory_order_relaxed);
}

void StockManager::_fireLoadEvent(LoadEvent event) {
    // There are very few callbacks (the plugin registers them once each at start / stop) and they
    // are triggered synchronously only in the loading sequence; with no registration the traversal
    // is empty and the cost is negligible
    std::shared_lock<std::shared_mutex> lock(g_load_event->mutex);
    for (const auto& [id, cb] : g_load_event->callbacks) {
        cb(event);
    }
}

void StockManager::joinPreloadThread() {
    if (m_preload_thread.joinable()) {
        m_preload_thread.join();
    }
}

void StockManager::loadAllKData() {
    // Control the loading order by the K-line type
    vector<KQuery::KType> ktypes;
    vector<string> low_ktypes;

    // If the context gives a ktype list, load in the order of the ktypes given by the context,
    // otherwise load in the default order
    const auto& context_ktypes = m_context.getKTypeList();
    if (context_ktypes.empty()) {
        ktypes = KQuery::getBaseKTypeList();

    } else {
        // Override the global preload parameters with the context preload parameters
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

        // Judge whether the context gives the preload numbers; when it does, they override the
        // default values
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

    // Load the K-lines of the same kind first (the preload is only a cache warm-up, it always runs
    // asynchronously in the background and does not block the initialization; the queries during
    // the warm-up are fetched from the driver in real time and their results are not affected; the
    // scenarios needing to wait for the warm-up to finish can call waitDataReady() explicitly)
    auto driver = _getKDataDriverPool();
    if (isIpcClientMode()) {
        // In the client mode the data is provided by the server, there is no local preload task and
        // it is ready directly
        m_data_ready.store(true, std::memory_order_release);
        return;
    }

    // The preload thread is now the joinable member m_preload_thread (no longer detached): at the
    // exit joinPreloadThread() waits for its exit before stopping m_load_tg / destroying the IPC
    // service, eradicating the concurrent access race (C3). If the previous preload thread still
    // exists (a duplicated initialization), join it before the new assignment, avoiding terminate
    // on assigning a joinable thread.
    joinPreloadThread();
    if (!driver->getPrototype()->canParallelLoad()) {
        m_preload_thread = std::thread([this, ktypes, low_ktypes]() mutable {
            _loadAllKDataSerial(std::move(ktypes), std::move(low_ktypes));
        });
    } else {
        // Asynchronous parallel loading
        m_preload_thread = std::thread([this, ktypes, low_ktypes]() mutable {
            _loadAllKDataParallel(std::move(ktypes), std::move(low_ktypes));
        });
    }
}

void StockManager::_loadAllKDataSerial(vector<KQuery::KType> ktypes, vector<string> low_ktypes) {
    // The progress reporting has moved to the plugin side (through LoadEvent + its own polling, see
    // design §5.2); loaded/total is no longer counted here

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
            const auto& low_ktype = low_ktypes[i];
            if (m_preloadParam.tryGet<bool>(low_ktype, false)) {
                iter->second.loadKDataToBuffer(ktypes[i]);
            }
        }
    }

    // Dispatch KDATA_PRELOAD_FINISHED before the historical finance is loaded, so that the clients
    // get the hot K-line data as early as possible; it is not dispatched when the preload is
    // cancelled (the process exit), avoiding a full serialization that is destroyed immediately
    if (!m_cancel_load) {
        _fireLoadEvent(LoadEvent::KDATA_PRELOAD_FINISHED);
    }

    if (!m_cancel_load && m_hikyuuParam.tryGet<bool>("load_history_finance", true)) {
        ThreadPool tg;
        std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
            if (m_cancel_load) {
                break;
            }
            tg.submit([stk = iter->second, this]() {
                HKU_IF_RETURN(m_cancel_load, void());
                stk.getHistoryFinance();
            });
        }
        lock.unlock();
        tg.join();
    }

    // The historical finance is ready, dispatch HISTORY_FINANCE_LOADED: the plugin rebuilds the
    // basic snapshot as a whole accordingly (the ex-rights/ex-dividend data and the finance are
    // collected together); The shared memory snapshot of an already connected session is fixed
    // after the negotiation during the connection and does not switch generation automatically with
    // a republish at runtime; the new snapshot is visible only to the sessions negotiated
    // afterwards; it is not dispatched when the preload is cancelled (the process exit): this
    // avoids both a useless full publish and a pointless serialization in the exit sequence
    if (!m_cancel_load) {
        _fireLoadEvent(LoadEvent::HISTORY_FINANCE_LOADED);
    }

    m_data_ready.store(true, std::memory_order_release);
}

void StockManager::_loadAllKDataParallel(vector<KQuery::KType> ktypes, vector<string> low_ktypes) {
    // The progress reporting has moved to the plugin side (see design §5.2); loaded/total is no
    // longer counted here
    auto loaded_codes = tryLoadAllKDataFromColumnFirst(ktypes);

    // Load the K-lines of the other securities (they may use different K-line drivers)
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
                // ktypes[i] is reused by the inner stock loop within the outer ktype loop; a
                // std::move here would make the first stock submit an empty moved-from ktypes[i],
                // and the following stocks would call loadKDataToBuffer("") and all fail (only the
                // first stock fills the preload buffer). Therefore a copy is used; ktype is a short
                // string and the cost is negligible.
                m_load_tg->submit([this, stk = iter->second, ktype = ktypes[i]]() mutable {
                    HKU_IF_RETURN(m_cancel_load, void());
                    stk.loadKDataToBuffer(ktype);
                });
            }
        }
    }

    // Wait for all the K-line preload tasks to finish and then dispatch KDATA_PRELOAD_FINISHED for
    // the plugin to publish the shared memory snapshot; note that the dispatch must happen after
    // the join, otherwise the buffer may not be filled yet; it is not dispatched when the preload
    // is cancelled (the process exit), avoiding a full serialization that is destroyed immediately
    m_load_tg->join();
    m_load_tg.reset();

    if (!m_cancel_load) {
        _fireLoadEvent(LoadEvent::KDATA_PRELOAD_FINISHED);
    }

    if (!m_cancel_load && m_hikyuuParam.tryGet<bool>("load_history_finance", true)) {
        m_load_tg = std::make_unique<ThreadPool>();
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
        lock.unlock();
        m_load_tg->join();
        m_load_tg.reset();
    }

    // The historical finance is ready, dispatch HISTORY_FINANCE_LOADED: the plugin rebuilds the
    // basic snapshot as a whole accordingly (the ex-rights/ex-dividend data and the finance are
    // collected together); The snapshot of an already connected session is fixed for the session
    // lifetime and does not switch generation automatically with a republish at runtime (a new
    // session maps the latest epoch at the negotiation); it is not dispatched when the preload is
    // cancelled (the process exit), for the same reason as the serial branch
    if (!m_cancel_load) {
        _fireLoadEvent(LoadEvent::HISTORY_FINANCE_LOADED);
    }

    m_data_ready.store(true, std::memory_order_release);
}

std::unordered_set<string> StockManager::tryLoadAllKDataFromColumnFirst(
  const vector<KQuery::KType>& ktypes) {
    std::unordered_set<string> loaded_codes;
    HKU_IF_RETURN(!m_context.isAll(), loaded_codes);
    auto driver = _getKDataDriverPool();
    HKU_IF_RETURN(!driver || !driver->getPrototype()->isColumnFirst(), loaded_codes);

    // Try to load the SH000001 K-lines with priority
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

    // It is mainly bandwidth limited, no multi-threading is needed
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

void StockManager::reloadWith(const StrategyContext& context) {
    HKU_IF_RETURN(m_initializing, void());
    m_initializing = true;

    if (!context.empty()) {
        m_context = context;
    } else {
        HKU_INFO(htr("The new context is empty, use the original context"));
    }

    HKU_INFO("start reload ...");
    loadData();
    m_initializing = false;
}

const string& StockManager::tmpdir() const {
    return m_tmpdir;
}

const string& StockManager::datadir() const {
    return m_datadir;
}

Stock StockManager::getStock(const string& querystr) const {
    Stock result;
    string query_str = querystr;
    to_upper(query_str);
    size_t pos = query_str.find('.');
    if (pos != string::npos) {
        // The suffix notation
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
    result.reserve(m_stockTypeInfo.size());
    for (const auto& item : m_stockTypeInfo) {
        result.push_back(item.second);
    }
    return result;
}

StringList StockManager::getAllMarket() const {
    StringList result;
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
    Block result;
    HKU_IF_RETURN(!m_blockDriver || category.empty() || name.empty(), result);
    auto iter = m_innerBlocks.find(fmt::format("{}_{}", category, name));
    if (iter != m_innerBlocks.end()) {
        return iter->second;
    }
    result = m_blockDriver->getBlock(category, name);
    return result;
}

void StockManager::saveBlock(const Block& blk) {
    if (m_blockDriver) {
        HKU_CHECK(!blk.category().empty(), "block's category can not be empty!");
        HKU_CHECK(!blk.name().empty(), "block's name can not be empty!");
        m_blockDriver->save(blk);
    }
}
void StockManager::removeBlock(const string& category, const string& name) {
    if (m_blockDriver) {
        m_blockDriver->remove(category, name);
    }
}

BlockList StockManager::getBlockList(const string& category) {
    BlockList result;
    HKU_IF_RETURN(!m_blockDriver, BlockList());
    result =
      category.empty() ? m_blockDriver->getBlockList() : m_blockDriver->getBlockList(category);
    auto iter = m_innerBlocks.begin();
    if (category.empty()) {
        for (; iter != m_innerBlocks.end(); ++iter) {
            result.push_back(iter->second);
        }
    } else {
        for (; iter != m_innerBlocks.end(); ++iter) {
            if (iter->first == category) {
                result.push_back(iter->second);
            }
        }
    }
    return result;
}

BlockList StockManager::getBlockListByIndexStock(const Stock& stk) {
    BlockList all = getBlockList();
    BlockList result;
    for (const auto& blk : all) {
        if (blk.getIndexStock() == stk) {
            result.push_back(blk);
        }
    }
    return result;
}

BlockList StockManager::getStockBelongs(const Stock& stk, const string& category) {
    BlockList result;
    BlockList all = getBlockList(category);
    for (const auto& blk : all) {
        if (blk.have(stk)) {
            result.push_back(blk);
        }
    }
    return result;
}

DatetimeList StockManager::getTradingCalendar(const KQuery& query, const string& market) {
    auto marketinfo = getMarketInfo(market);
    return getStock(fmt::format("{}{}", marketinfo.market(), marketinfo.code()))
      .getDatetimeList(query);
}

DatetimeList StockManager::getTradingCalendar(const StockList& stk_list, const KQuery& query) {
    std::unordered_set<string> markets;
    for (const auto& stk : stk_list) {
        if (!stk.isNull()) {
            markets.insert(stk.market());
        }
    }

    std::set<Datetime> date_set;
    for (const auto& market : markets) {
        DatetimeList temp = getTradingCalendar(query, market);
        if (temp.size() > 0) {
            date_set.insert(temp.begin(), temp.end());
        }
    }

    DatetimeList result;
    result.reserve(date_set.size());
    for (const auto& date : date_set) {
        result.push_back(date);
    }

    return result;
}

const ZhBond10List& StockManager::getZhBond10() const {
    return m_zh_bond10;
}

bool StockManager::isHoliday(const Datetime& d) const {
    HKU_IF_RETURN(d.dayOfWeek() == 0 || d.dayOfWeek() == 6, true);
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
    for (const auto& ktype : base_ktypes) {
        auto nktype = ktype;
        to_lower(nktype);
        if (m_preloadParam.tryGet<bool>(nktype, false)) {
            preload_ktypes.push_back(ktype);
        }
    }

    auto kdriver = _getKDataDriverPool();

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
                // Force releasing all the cached K-line data
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
    m_stockTypeInfo.clear();
    m_stockTypeInfo.reserve(stkTypeInfos.size());
    for (auto& stkTypeInfo : stkTypeInfos) {
        m_stockTypeInfo[stkTypeInfo.type()] = stkTypeInfo;
    }
}

void StockManager::loadAllHolidays() {
    auto holidays = m_baseInfoDriver->getAllHolidays();
    std::unordered_set<Datetime> tmp_holidays(holidays.begin(), holidays.end());
    m_holidays = std::move(tmp_holidays);
}

void StockManager::loadInnerBlocks() {
    Block blocka = Block("A", "ALL");
    Block blocksh = Block("A", "SH");
    Block blocksz = Block("A", "SZ");
    Block blockbj = Block("A", "BJ");
    Block blocka_shsz = Block("A", "沪深");
    Block blockzxb = Block("A", "中小板");
    Block blockg = Block("G", "创业板");
    Block blockstart = Block("START", "科创板");
    Block blocketf = Block("ETF", "ALL");

    std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
    auto iter = m_stockDict.begin();
    for (; iter != m_stockDict.end(); ++iter) {
        const Stock& stock = iter->second;
        if (stock.type() == STOCKTYPE_A) {
            blocka.add(stock);
            blocka_shsz.add(stock);
            if (stock.market() == "SH") {
                blocksh.add(stock);
            } else if (stock.market() == "SZ") {
                blocksz.add(stock);
                if (stock.code().size() >= 3 && stock.code().substr(0, 3) == "002") {
                    blockzxb.add(stock);
                }
            }
        } else if (stock.type() == STOCKTYPE_A_BJ) {
            blocka.add(stock);
            blockbj.add(stock);
        } else if (stock.type() == STOCKTYPE_GEM) {
            blockg.add(stock);
        } else if (stock.type() == STOCKTYPE_START) {
            blockstart.add(stock);
        } else if (stock.type() == STOCKTYPE_ETF) {
            blocketf.add(stock);
        }
    }

    iter = m_stockDict.find("SH000001");
    if (iter != m_stockDict.end()) {
        blocka.setIndexStock(iter->second);
        blocka_shsz.setIndexStock(iter->second);
        blocksh.setIndexStock(iter->second);
    }
    iter = m_stockDict.find("SZ399001");
    if (iter != m_stockDict.end()) {
        blocksz.setIndexStock(iter->second);
    }
    iter = m_stockDict.find("BJ899050");
    if (iter != m_stockDict.end()) {
        blockbj.setIndexStock(iter->second);
    }
    iter = m_stockDict.find("SZ399005");
    if (iter != m_stockDict.end()) {
        blockzxb.setIndexStock(iter->second);
    }
    iter = m_stockDict.find("SZ399006");
    if (iter != m_stockDict.end()) {
        blockg.setIndexStock(iter->second);
    }
    iter = m_stockDict.find("SH000688");
    if (iter != m_stockDict.end()) {
        blockstart.setIndexStock(iter->second);
    }

    m_innerBlocks.clear();
    m_innerBlocks["A_ALL"] = std::move(blocka);
    m_innerBlocks["A_沪深"] = std::move(blocka_shsz);
    m_innerBlocks["A_SH"] = std::move(blocksh);
    m_innerBlocks["A_SZ"] = std::move(blocksz);
    m_innerBlocks["A_BJ"] = std::move(blockbj);
    m_innerBlocks["A_中小板"] = std::move(blockzxb);
    m_innerBlocks["G_创业板"] = std::move(blockg);
    m_innerBlocks["START_科创板"] = std::move(blockstart);
    m_innerBlocks["ETF_ALL"] = std::move(blocketf);
}

void StockManager::loadAllStockWeights() {
    HKU_IF_RETURN(!m_hikyuuParam.tryGet<bool>("load_stock_weight", true), void());
    // The client mode also materializes all the ex-rights/ex-dividend data at the startup according
    // to the config above: the shared memory snapshot has been mapped at once by the connection
    // after waitReady (IpcConnector::mapSessionShm) and IpcBaseInfoDriver reads all the
    // ex-rights/ex-dividend data from the snapshot (when the snapshot does not cover it, the local
    // driver is read directly, sharing the same data source with the main process); after that
    // Stock::getWeight hits the local cache directly, satisfying the high frequency reading of the
    // ex-rights/ex-dividend data; the securities not materialized (the config off, added by
    // addStock or newly constructed) are still handled by the on-demand lazy loading fallback of
    // Stock::getWeight.
    HKU_INFO(htr("Loading stock weight..."));
    if (m_context.isAll()) {
        auto all_stkweight_dict = m_baseInfoDriver->getAllStockWeightList();
        for (auto& item : all_stkweight_dict) {
            item.second.shrink_to_fit();
        }
        std::shared_lock<std::shared_mutex> lock1(*m_stockDict_mutex);
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
            auto weight_iter = all_stkweight_dict.find(iter->first);
            Stock& stock = iter->second;
            {
                std::unique_lock<std::shared_mutex> lock2(stock.m_data->m_weight_mutex);
                if (weight_iter != all_stkweight_dict.end()) {
                    stock.m_data->m_weightList.swap(weight_iter->second);
                }
                // It is marked materialized whether the security has the ex-rights/ex-dividend data
                // or not: not being collected means this security has none (such as most ETFs),
                // avoiding the client mode getWeight repeatedly triggering an empty lazy loading
                // query for the securities without the ex-rights/ex-dividend data
                stock.m_data->m_weight_ready.store(true, std::memory_order_release);
            }
        }
    } else {
        std::shared_lock<std::shared_mutex> lock1(*m_stockDict_mutex);
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
            Stock& stock = iter->second;
            auto sw_list = m_baseInfoDriver->getStockWeightList(
              stock.market(), stock.code(), m_context.startDatetime(), Null<Datetime>());
            sw_list.shrink_to_fit();
            {
                std::unique_lock<std::shared_mutex> lock2(stock.m_data->m_weight_mutex);
                stock.m_data->m_weightList = std::move(sw_list);
                stock.m_data->m_weight_ready.store(true, std::memory_order_release);
            }
        }
    }
}

void StockManager::releaseShmServerBaseInfoCache() {
    // Only the shm server role (and not the client mode) is allowed; the client / ordinary
    // standalone mode must not release it: the client does not materialize the historical finance
    // locally, and the ex-rights/ex-dividend materialization at the startup (load_stock_weight)
    // cannot rely on a shared snapshot in a non-server role, so after a release getWeight would
    // silently return empty without a lazy loading fallback
    HKU_IF_RETURN(!isShmServerRole() || isIpcClientMode(), void());
    HKU_DEBUG(htr("Release stock weight/finance cache after shm base info published"));
    std::shared_lock<std::shared_mutex> lock1(*m_stockDict_mutex);
    for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
        Stock& stock = iter->second;
        {
            std::unique_lock<std::shared_mutex> lock2(stock.m_data->m_weight_mutex);
            StockWeightList().swap(stock.m_data->m_weightList);
            // Set it to false: the next Stock::getWeight re-reads it through the driver lazy
            // loading (the server role has the lazy loading fallback)
            stock.m_data->m_weight_ready.store(false, std::memory_order_release);
        }
        {
            std::unique_lock<std::shared_mutex> lock2(stock.m_data->m_history_finance_mutex);
            vector<HistoryFinanceInfo>().swap(stock.m_data->m_history_finance);
            // Set it to false: the next Stock::getHistoryFinance re-reads it through the driver
            // lazy loading (every mode has the fallback)
            stock.m_data->m_history_finance_ready = false;
        }
    }
}

void StockManager::loadAllZhBond10() {
    m_zh_bond10 = m_baseInfoDriver->getAllZhBond10();
    m_zh_bond10.shrink_to_fit();
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

void StockManager::waitDataReady() const {
    HKU_INFO_IF(!dataReady(), htr("Waiting for preload data loading to complete..."));
    while (!dataReady()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

}  // namespace hku
