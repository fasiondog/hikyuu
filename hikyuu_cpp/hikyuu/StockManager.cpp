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
#include <cstdlib>
#include <fmt/format.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "hikyuu/utilities/ini_parser/IniParser.h"
#include "hikyuu/utilities/thread/ThreadPool.h"
#include "hikyuu/utilities/thread/algorithm.h"
#include "StockManager.h"
#include "global/schedule/inner_tasks.h"
#include "data_driver/kdata/cvs/KDataTempCsvDriver.h"
#include "plugin/interface/plugins.h"
#include "plugin/device.h"
#include "plugin/hkuextra.h"
#include "plugin/extind.h"
#include "global/sysinfo.h"

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
    // 先等后台预加载线程退出：避免其在成员（m_load_tg / m_ipc_server）析构后仍访问 → UAF；
    // 同时保证 m_preload_thread 析构时非 joinable（否则 std::thread 析构会触发 std::terminate）。
    // 幂等：clean() 通常已 join 过，此处再调用为无操作（兼顾未经 clean() 的析构路径）。
    joinPreloadThread();
#if HKU_ENABLE_NODE
    // 注销客户端实时更新转发连接：此后 Stock::realtimeUpdate 的转发调用直接返回，
    // 避免退出期在已失效的连接上阻塞
    ipc::registerRealtimeForwarder(nullptr);
    stopIpcDataServer();
#endif
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
        m_plugin_manager.pluginPath("./plugin");
    }
    HKU_INFO(htr("Plugin path: {}", getPluginPath()));

    // 注册扩展K线处理
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

#if HKU_ENABLE_NODE
    // 自动协商单机 IPC 数据服务（客户端模式将替换为代理驱动并关闭本地预加载）
    _negotiateIpcDataServer();
#endif

    // 加载数据
    loadData();

    // 基础数据（证券/市场/节假日/权息/板块等）加载完毕即可对外提供服务；
    // K线预加载仅为缓存预热，服务端的K线请求均通过驱动实时查询，
    // 后台预热期间不影响客户端查询，无需让客户端等待预加载完成。
    _notifyIpcBaseDataReady();

    // 初始化内部定时任务（重加载）
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
    // 权息已就绪，先发布一次共享内存快照（历史财务要等预加载线程跑完再发布）。
    // 两次发布分处主线程与预加载线程，时序上本处同步完成后才启动预加载，
    // 服务端内部仍以 m_bi_pub_mutex 串行化并保证 epoch 单调，以防 reload 等路径下重叠。
    // 此时历史财务尚未预加载，须 include_finance=false，否则逐个证券触发历史财务懒加载（见 Stock::getHistoryFinance）。
    _publishBaseInfoShmIfMaster(false);
    loadAllZhBond10();
    loadHistoryFinanceField();

    HKU_INFO(htr("Loading block..."));
    m_blockDriver->load();
#if HKU_ENABLE_NODE
    if (m_ipc_server && m_ipc_server->running()) {
        // 刷新 IPC 服务的板块缓存（驱动缓存加载完成后才能取到数据）
        m_ipc_server->refreshBlocks();
    }
#endif

    // 获取K线数据驱动并预加载指定的数据
    HKU_INFO(htr("Loading KData..."));

    // 加载K线及历史财务信息
    loadAllKData();

    // 更新 license expire time
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

#if HKU_ENABLE_NODE
void StockManager::_negotiateIpcDataServer() {
    HKU_IF_RETURN(!m_hikyuuParam.tryGet<bool>("kdata_server", true), void());

    // 以数据目录区分服务地址与文件锁，避免不同项目/数据集间互扰
    // （地址构造与单元测试共用 ipc::makeIpcServerPaths，含 Windows 命名管道平台适配）
    string addr, lock_path;
    ipc::makeIpcServerPaths(m_datadir, addr, lock_path);

    // 切换为客户端模式：替换为代理驱动并关闭本地预加载（仅内存覆盖，不改配置文件）
    auto enterClientMode = [this](const ipc::IpcConnectorPtr& conn) {
        m_ipc_client_mode = true;
        m_ipc_conn = conn;
        m_baseInfoDriver = std::make_shared<ipc::IpcBaseInfoDriver>(conn, m_baseInfoDriver);
        m_blockDriver = std::make_shared<ipc::IpcBlockDriver>(conn, m_blockDriver);
        // 传入整个本地驱动连接池（而非其 prototype）：主进程未预加载的类型与分时/分笔
        // 由客户端本地驱动直接服务，需经池取连接以避免多个克隆并发复用同一连接/文件句柄
        auto local_pool = DataDriverFactory::getKDataDriverPool(m_kdataDriverParam);
        auto ipc_kdriver = std::make_shared<ipc::IpcKDataDriver>(conn, local_pool);
        m_ipc_kdata_pool = std::make_shared<KDataDriverConnectPool>(ipc_kdriver);
        for (const auto& ktype : KQuery::getBaseKTypeList()) {
            auto low_ktype = ktype;
            to_lower(low_ktype);
            m_preloadParam.set<bool>(low_ktype, false);
        }
        // 注册实时更新转发：客户端无预加载缓冲，Stock::realtimeUpdate 经此连接
        // 转发至主进程应用（缓冲 + 共享内存镜像），保留客户端主动更新行情的能力
        ipc::registerRealtimeForwarder(conn);
        HKU_INFO("Connected to hikyuu data server: {}, running in client mode.",
                 m_ipc_conn->addr());
    };

    // 连接服务并等待其数据就绪，成功返回 true 并切换为客户端模式
    auto waitTimeout = [this]() -> uint64_t {
        return m_hikyuuParam.tryGet<int64_t>("kdata_server_wait_timeout", 600);
    };
    auto tryConnect = [&](const ipc::IpcConnectorPtr& conn) -> bool {
        HKU_IF_RETURN(!conn->connected(), false);
        HKU_IF_RETURN(!conn->waitReady(waitTimeout()), false);
        enterClientMode(conn);
        return true;
    };

    // 1. 探测是否已有数据服务（客户端模式）
    auto conn = std::make_shared<ipc::IpcConnector>();
    if (conn->init(addr) && tryConnect(conn)) {
        return;
    }

    // 2. 竞争文件锁成为数据服务主进程（服务先行，客户端可轮询等待加载进度）
    m_ipc_server = std::make_shared<ipc::HikyuuDataServer>();
    if (m_ipc_server->start(addr, lock_path, m_tmpdir)) {
        return;
    }
    m_ipc_server.reset();

    // 3. 锁已被其他进程持有但服务尚未监听（对方正在启动中），重试连接后降级独立模式
    for (int i = 0; i < 20; i++) {
        if (ipc::checkInterrupted()) {
            HKU_WARN("Connect to hikyuu data server interrupted!");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (conn->init(addr) && tryConnect(conn)) {
            return;
        }
    }
    HKU_WARN("Failed connect to hikyuu data server, fallback to standalone mode!");
}

void StockManager::stopIpcDataServer() {
    if (!m_ipc_server) {
        return;
    }
    // 先停机（等回调结束、关闭 ctx/aio、释放文件锁），再销毁服务；
    // 销毁会连带析构发布器，解除共享内存映射并删除当前段。
    // at_process_exit=true：本函数仅在进程退出路径（clean() 与 ~StockManager）调用，
    // Windows 下据此跳过阻塞式 nng 拆除，避免命名管道在静态析构期取消在飞异步操作时卡死。
    m_ipc_server->stop(true);
#if !defined(_WIN32)
    // 非 Windows：正常销毁服务，连带析构发布器解除共享内存映射并删除当前段。
    // Windows 进程退出路径不销毁：IpcServer 仍有在飞 nng 回调，销毁会在其析构中触发
    // 阻塞式 nng 拆除而卡死；保留对象至进程退出，由 OS 统一回收（含共享内存段）。
    m_ipc_server.reset();
#endif
}

bool StockManager::_isIpcClientMode() const {
    return m_ipc_client_mode;
}

bool StockManager::isIpcClientMode() const {
    return m_ipc_client_mode;
}

// 主进程 K 线预加载完成后，将热数据发布为只读共享内存缓存快照，
// 供客户端进程零拷贝直接读取，避免全市场遍历时逐证券 IPC 往返
void StockManager::_publishShmCacheIfMaster() {
    HKU_IF_RETURN(!m_ipc_server || !m_ipc_server->running(), void());
    HKU_IF_RETURN(!m_hikyuuParam.tryGet<bool>("kdata_server_shm_cache", true), void());
    m_ipc_server->publishShmCache("hikyuu_ks");
}

// 权息与历史财务发布为只读共享内存快照，供客户端零拷贝读取。
// 发布点有两处：权息在 loadData 中加载后立即发布（此时财务尚未预加载，须传 include_finance=false，
// 否则会逐证券触发历史财务懒加载）；历史财务在预加载线程中于其加载完成后发布
// （此时整段重建，权息与财务一并重新收录）。
void StockManager::_publishBaseInfoShmIfMaster(bool include_finance) {
    HKU_IF_RETURN(!m_ipc_server || !m_ipc_server->running(), void());
    HKU_IF_RETURN(!m_hikyuuParam.tryGet<bool>("kdata_server_shm_cache", true), void());
    m_ipc_server->publishBaseInfoShm("hikyuu_bi", include_finance);
}

void StockManager::_reportLoadProgress(uint64_t loaded, uint64_t total) {
    HKU_IF_RETURN(!m_ipc_server, void());
    m_ipc_server->updateProgress(std::min(loaded, total), total);
}

void StockManager::_notifyIpcBaseDataReady() {
    HKU_IF_RETURN(!m_ipc_server, void());
    m_ipc_server->setAllReady();
}

#else

/* 未启用单机数据服务时均为空操作，使调用方（两个加载函数与 init）无需条件编译 */
bool StockManager::_isIpcClientMode() const {
    return false;
}
bool StockManager::isIpcClientMode() const {
    return false;
}
void StockManager::_publishBaseInfoShmIfMaster(bool) {}
void StockManager::_publishShmCacheIfMaster() {}
void StockManager::_reportLoadProgress(uint64_t, uint64_t) {}
void StockManager::_notifyIpcBaseDataReady() {}

#endif  // HKU_ENABLE_NODE

void StockManager::joinPreloadThread() {
    if (m_preload_thread.joinable()) {
        m_preload_thread.join();
    }
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

    // 先加载同类K线（预加载仅为缓存预热，一律后台异步执行，不阻塞初始化；
    // 预热期间的查询经由驱动实时获取，结果不受影响；
    // 需要等待预热完成的场景可显式调用 waitDataReady()）
    auto driver = _getKDataDriverPool();
    if (_isIpcClientMode()) {
        // 客户端模式下数据由服务端提供，本地无预加载任务，直接就绪
        m_data_ready.store(true, std::memory_order_release);
        return;
    }

    // 预加载线程改为 joinable 成员 m_preload_thread（不再 detach）：退出时由 joinPreloadThread()
    // 等其退出后再停 m_load_tg / 销毁 IPC 服务，根除并发访问竞态（C3）。
    // 若上一次预加载线程仍存在（重复初始化），先 join 再重新赋值，避免对 joinable 线程赋值触发 terminate。
    joinPreloadThread();
    if (!driver->getPrototype()->canParallelLoad()) {
        m_preload_thread = std::thread([this, ktypes, low_ktypes]() mutable {
            _loadAllKDataSerial(std::move(ktypes), std::move(low_ktypes));
        });
    } else {
        // 异步并行加载
        m_preload_thread = std::thread([this, ktypes, low_ktypes]() mutable {
            _loadAllKDataParallel(std::move(ktypes), std::move(low_ktypes));
        });
    }
}

void StockManager::_loadAllKDataSerial(vector<KQuery::KType> ktypes, vector<string> low_ktypes) {
    size_t preload_ktype_count = 0;
    for (const auto& low_ktype : low_ktypes) {
        if (m_preloadParam.tryGet<bool>(low_ktype, false)) {
            preload_ktype_count++;
        }
    }
    // m_stockDict.size() 须在锁内读取：预加载线程异步运行，reload 路径中 loadAllStocks() 持写锁
    // 修改 m_stockDict 可能与旧预加载线程（尚未 join）的此处读取并发，裸读构成数据竞争；与下方
    // 迭代的 shared_lock 保持一致
    uint64_t total_task;
    {
        std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
        total_task = m_stockDict.size() * preload_ktype_count;
    }
    uint64_t loaded_task = 0;

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
                _reportLoadProgress(++loaded_task, total_task);
            }
        }
    }

    // 在历史财务加载之前发布，使客户端尽早获得 K 线热数据；
    // 取消预加载（进程退出）时不发布，避免白做全量序列化后立即被销毁
    if (!m_cancel_load) {
        _publishShmCacheIfMaster();
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

    // 历史财务已就绪，重新发布快照：此时整段重建，权息与财务一并收录，
    // 客户端经 epoch 变化自动换代；
    // 取消预加载（进程退出）时不发布：既避免白做一次全量发布，也避免在退出时序中
    // 继续访问已被 stopIpcDataServer() 销毁的服务对象
    if (!m_cancel_load) {
        _publishBaseInfoShmIfMaster();
    }

    m_data_ready.store(true, std::memory_order_release);
}

void StockManager::_loadAllKDataParallel(vector<KQuery::KType> ktypes, vector<string> low_ktypes) {
    auto loaded_cnt = std::make_shared<std::atomic<uint64_t>>(0);
    // 同 _loadAllKDataSerial：m_stockDict.size() 须在锁内读取，避免与 reload 路径 loadAllStocks()
    // 的写锁并发构成数据竞争
    uint64_t total_task;
    {
        std::shared_lock<std::shared_mutex> lock(*m_stockDict_mutex);
        total_task = m_stockDict.size();
    }
    _reportLoadProgress(0, total_task);

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
                // ktypes[i] 在外层 ktype 循环内被内层证券循环复用，此处若 std::move 会使首个
                // 证券 submit 后 ktypes[i] 变为 moved-from 空串，后续证券 loadKDataToBuffer("")
                // 全部失效（预加载缓冲仅首证券填充）。故用拷贝，ktype 为短字符串开销可忽略。
                m_load_tg->submit([this, stk = iter->second, ktype = ktypes[i], loaded_cnt,
                                   total_task]() mutable {
                    HKU_IF_RETURN(m_cancel_load, void());
                    stk.loadKDataToBuffer(ktype);
                    _reportLoadProgress(loaded_cnt->fetch_add(1) + 1, total_task);
                });
            }
        }
    }

    // 等待 K 线预加载任务全部完成后，再将热数据发布为共享内存快照；
    // 注意发布必须在 join 之后，否则缓冲区可能尚未填充；
    // 取消预加载（进程退出）时不发布，避免白做全量序列化后立即被销毁
    m_load_tg->join();
    m_load_tg.reset();

    if (!m_cancel_load) {
        _publishShmCacheIfMaster();
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

    // 历史财务已就绪，重新发布快照：此时整段重建，权息与财务一并收录，
    // 客户端经 epoch 变化自动换代；取消预加载（进程退出）时不发布，理由同串行分支
    if (!m_cancel_load) {
        _publishBaseInfoShmIfMaster();
    }

    m_data_ready.store(true, std::memory_order_release);
}

std::unordered_set<string> StockManager::tryLoadAllKDataFromColumnFirst(
  const vector<KQuery::KType>& ktypes) {
    std::unordered_set<string> loaded_codes;
    HKU_IF_RETURN(!m_context.isAll(), loaded_codes);
    auto driver = _getKDataDriverPool();
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
    // 客户端模式下权息由主进程发布的共享内存快照提供：Stock::getWeight 按需经驱动
    // （IpcBaseInfoDriver，shm 优先）读取，无需在本地再物化一份全量权息，避免与快照
    // 重复占用客户端内存（历史财务同理——客户端 loadAllKData 提前返回，本就不预加载财务）。
    HKU_IF_RETURN(isIpcClientMode(), void());
    HKU_INFO(htr("Loading stock weight..."));
    if (m_context.isAll()) {
        auto all_stkweight_dict = m_baseInfoDriver->getAllStockWeightList();
        for (auto& item : all_stkweight_dict) {
            item.second.shrink_to_fit();
        }
        std::shared_lock<std::shared_mutex> lock1(*m_stockDict_mutex);
        for (auto iter = m_stockDict.begin(); iter != m_stockDict.end(); ++iter) {
            auto weight_iter = all_stkweight_dict.find(iter->first);
            if (weight_iter != all_stkweight_dict.end()) {
                Stock& stock = iter->second;
                std::unique_lock<std::shared_mutex> lock2(stock.m_data->m_weight_mutex);
                stock.m_data->m_weightList.swap(weight_iter->second);
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
            }
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
