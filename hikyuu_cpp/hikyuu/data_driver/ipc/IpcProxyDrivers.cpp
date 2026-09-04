/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-01
 *      Author: fasiondog
 */

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <algorithm>
#include <chrono>
#include <shared_mutex>
#include <thread>
#include "IpcProxyDrivers.h"
#include "hikyuu/StockManager.h"
#include "hikyuu/utilities/Log.h"

namespace hku {
namespace ipc {

namespace {
/// 探测类请求的短超时（毫秒）：服务端不可用时单次探测最多阻塞该时长
constexpr uint32_t PROBE_TIMEOUT_MS = 1000;

/// 探测连续失败后的基础退避间隔（毫秒），随失败次数指数增长
constexpr int64_t PROBE_BACKOFF_BASE_MS = 5000;

/// 退避间隔的最大左移位数，即 5s → 10s → 20s 封顶；
/// 封顶值同时决定了服务端重启完成后客户端感知新快照段的最大延迟
constexpr int PROBE_BACKOFF_MAX_SHIFT = 2;

/**
 * 判断指定证券的分时/分笔价格是否需从服务端加工态还原为驱动原始值
 * @details 服务端经 Stock 层应答，而 Stock 层对 ETF/FUND/B 的分时·分笔价格统一施加 ×0.1
 * （缓冲与非缓冲分支均如此，见 Stock::getKRecordList / getTimeLineList / getTransList），
 * 快照亦取自已加工的预加载缓冲。客户端 Stock 层拿到驱动结果后会再加工一次，
 * 故代理驱动必须在此还原，否则价格被缩放两次（0.01×）。
 * @note 还原为 ÷0.1，与客户端后续的 ×0.1 数学上互逆；浮点上存在 1 ULP 级差异，
 *       对价格无实际影响。仅分时/分笔需要，故先判类型再查证券，避开无用开销。
 */
bool needUnscalePrice(const KQuery::KType& ktype, const std::string& market,
                      const std::string& code) {
    if (ktype != KQuery::TIMELINE && ktype != KQuery::TRANS) {
        return false;
    }
    Stock stk = StockManager::instance().getStock(market + code);
    HKU_IF_RETURN(stk.isNull(), false);
    uint32_t t = stk.type();
    return t == STOCKTYPE_ETF || t == STOCKTYPE_FUND || t == STOCKTYPE_B;
}

/// 还原 KRecordList 中服务端已施加的价格缩放。
/// @note 仅还原 closePrice：服务端 Stock.cpp 只对 TIMELINE/TRANS 的 ETF/FUND/B 缩放 closePrice
///       （×0.1），不涉及 open/high/low。二者隐式配对——若服务端改为缩放全部 OHLC，本函数须同步
///       更新，否则 open/high/low 将静默产生 10 倍误差。调用点由 needUnscalePrice 按同一条件守卫。
void unscalePrice(KRecordList& ks) {
    for (auto& k : ks) {
        k.closePrice /= 0.1;
    }
}

/// 还原 TimeLineList 中服务端已施加的价格缩放
void unscalePrice(TimeLineList& tls) {
    for (auto& t : tls) {
        t.price /= 0.1;
    }
}

/// 还原 TransList 中服务端已施加的价格缩放
void unscalePrice(TransList& ts) {
    for (auto& t : ts) {
        t.price /= 0.1;
    }
}
}  // namespace

///////////////////////////////////////////////////////////////////////////////
// IpcConnector
///////////////////////////////////////////////////////////////////////////////
IpcConnector::IpcConnector(const std::string& addr) {
    init(addr);
}

bool IpcConnector::init(const std::string& addr) {
    m_addr = addr;
    m_client.setAddr(addr);
    return m_client.dial();
}

bool IpcConnector::request(Cmd cmd, const std::vector<uint8_t>& body,
                           std::vector<uint8_t>& res_body) {
    std::vector<uint8_t> req_frame = encodeRequest(cmd, body);
    std::vector<uint8_t> res_frame;
    if (!m_client.request(req_frame, res_frame)) {
        return false;
    }

    RetCode ret = RetCode::ERROR;
    if (!decodeResponse(res_frame, ret, res_body)) {
        HKU_ERROR("Invalid ipc response frame!");
        return false;
    }

    if (ret != RetCode::SUCCESS) {
        Reader rd(res_body.data(), res_body.size());
        std::string errmsg = (ret == RetCode::ERROR) ? rd.getString() : "";
        HKU_ERROR_IF(ret == RetCode::ERROR, "Ipc server error: {}", errmsg);
        return false;
    }
    return true;
}

bool IpcConnector::probeRequest(Cmd cmd, std::vector<uint8_t>& res_body) {
    auto now = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lock(m_probe_mutex);
        if (m_probe_fails > 0) {
            int64_t backoff = PROBE_BACKOFF_BASE_MS
                              << std::min(m_probe_fails - 1, PROBE_BACKOFF_MAX_SHIFT);
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_probe).count() <
                backoff) {
                return false;  // 退避期内跳过探测，调用方沿用已有数据或回退本地驱动
            }
        }
        // 在发起请求前记录时间点：即使请求阻塞至超时，也不会因耗时而立即绕过退避判定
        m_last_probe = now;
    }

    const std::vector<uint8_t> empty_body;
    std::vector<uint8_t> req_frame = encodeRequest(cmd, empty_body);
    std::vector<uint8_t> res_frame;
    bool ok = m_client.request(req_frame, res_frame, PROBE_TIMEOUT_MS);
    if (ok) {
        RetCode ret = RetCode::ERROR;
        if (!decodeResponse(res_frame, ret, res_body)) {
            HKU_ERROR("Invalid ipc response frame!");
            ok = false;
        } else if (ret != RetCode::SUCCESS) {
            ok = false;
        }
    }

    std::lock_guard<std::mutex> lock(m_probe_mutex);
    if (ok) {
        if (m_probe_fails > 0) {
            HKU_INFO("Ipc data server probe recovered after {} failure(s)!", m_probe_fails);
        }
        m_probe_fails = 0;
    } else {
        m_probe_fails++;
        if (m_probe_fails == 1) {
            HKU_WARN(
              "Ipc data server probe failed (server may be restarting), "
              "will retry with exponential backoff!");
        }
    }
    return ok;
}

///////////////////////////////////////////////////////////////////////////////
// 客户端实时更新转发
///////////////////////////////////////////////////////////////////////////////
namespace {
/// 转发连接注册表：进入客户端模式时注册（StockManager::_negotiateIpcDataServer），
/// 退出时注销；读写锁保护，转发频度低（行情推送级），锁开销可忽略
std::shared_mutex g_fwd_mutex;
IpcConnectorPtr g_fwd_conn;
}  // namespace

void registerRealtimeForwarder(const IpcConnectorPtr& conn) {
    std::unique_lock<std::shared_mutex> lock(g_fwd_mutex);
    g_fwd_conn = conn;
}

bool ipcForwardRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                              const KRecord& record) {
    IpcConnectorPtr conn;
    {
        std::shared_lock<std::shared_mutex> lock(g_fwd_mutex);
        conn = g_fwd_conn;
    }
    HKU_IF_RETURN(!conn, false);

    // 请求体：[market_code string][ktype string][record KRecord]，响应体：[applied u8]
    Encoder enc;
    enc.putString(market_code);
    enc.putString(ktype);
    encodeKRecord(enc, record);
    std::vector<uint8_t> res_body;
    if (!conn->request(Cmd::KDATA_REALTIME_UPDATE, enc.data(), res_body)) {
        // 主进程不可用（如每日定时重启窗口）期间行情持续到达，仅首次失败告警避免刷屏
        static std::atomic_bool s_warned{false};
        if (!s_warned.exchange(true)) {
            HKU_WARN("Failed forward realtime update to data server ({} {})!", market_code, ktype);
        }
        return false;
    }
    Reader rd(res_body.data(), res_body.size());
    uint8_t applied = rd.getU8();
    HKU_IF_RETURN(!rd.ok(), false);
    return applied != 0;
}

Datetime ipcForwardGetLastUpdateTime(const std::string& market_code, const KQuery::KType& ktype) {
    IpcConnectorPtr conn;
    {
        std::shared_lock<std::shared_mutex> lock(g_fwd_mutex);
        conn = g_fwd_conn;
    }
    HKU_IF_RETURN(!conn, Datetime::min());

    // 请求体：[market_code string][ktype string]，响应体：[last_update 全精度 Datetime]
    Encoder enc;
    enc.putString(market_code);
    enc.putString(ktype);
    std::vector<uint8_t> res_body;
    HKU_IF_RETURN(!conn->request(Cmd::KDATA_GET_LAST_UPDATE_TIME, enc.data(), res_body),
                  Datetime::min());
    Reader rd(res_body.data(), res_body.size());
    Datetime last = decodeDatetimeFull(rd);
    HKU_IF_RETURN(!rd.ok(), Datetime::min());
    return last;
}

bool IpcConnector::waitReady(uint64_t timeout_seconds,
                             std::function<void(uint64_t, uint64_t)>&& progress_cb) {
    auto start_tp = std::chrono::steady_clock::now();
    const std::vector<uint8_t> empty_body;
    bool first_log = true;

    while (true) {
        std::vector<uint8_t> res_body;
        std::vector<uint8_t> req_frame = encodeRequest(Cmd::STATUS_READY, empty_body);
        std::vector<uint8_t> res_frame;
        if (m_client.request(req_frame, res_frame)) {
            RetCode ret = RetCode::ERROR;
            if (decodeResponse(res_frame, ret, res_body)) {
                Reader rd(res_body.data(), res_body.size());
                uint8_t ready = rd.getU8();
                uint64_t loaded = rd.getU64();
                uint64_t total = rd.getU64();
                if (rd.ok()) {
                    if (ready != 0) {
                        return true;
                    }
                    HKU_INFO_IF(first_log,
                                "Waiting for the hikyuu data server to finish loading data ...");
                    first_log = false;
                    HKU_TRACE("Data server loading progress: {}/{}", loaded, total);
                    if (progress_cb) {
                        progress_cb(loaded, total);
                    }
                }
            }
        } else {
            HKU_WARN("Ipc data server connection lost while waiting ready!");
        }

        if (timeout_seconds > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::steady_clock::now() - start_tp)
                             .count();
            if (elapsed >= (int64_t)timeout_seconds) {
                HKU_WARN("Wait ipc data server ready timeout ({} seconds)!", timeout_seconds);
                return false;
            }
        }

        // 响应外部中断（如 Python 环境下的 Ctrl+C），避免长时间不可中断的等待；
        // 在 sleep 前后各检查一次，保证快速响应。
        if (checkInterrupted()) {
            HKU_WARN("Wait ipc data server ready interrupted!");
            return false;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

bool IpcConnector::refreshShmInfo() {
    if (!connected()) {
        return false;
    }
    // 拉取限流：最小间隔内最多协商一次，避免高频未命中查询每查一次额外 IPC；
    // 服务端发布新段后客户端最多延迟该间隔感知。服务端不可用时的退避抑制由
    // probeRequest 在连接内统一处理（连续失败指数退避）。
    constexpr int64_t MIN_CHECK_INTERVAL_MS = 5000;
    auto now = std::chrono::steady_clock::now();
    {
        std::lock_guard<std::mutex> lock(m_shm_info_mutex);
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_shm_last_check).count() <
            MIN_CHECK_INTERVAL_MS) {
            return false;
        }
        m_shm_last_check = now;
    }

    std::vector<uint8_t> res_body;
    if (!probeRequest(Cmd::STATUS_SHM_INFO, res_body)) {
        return false;  // 退避抑制或通讯失败，保持现状，查询自然回退 IPC/本地
    }

    Reader rd(res_body.data(), res_body.size());
    ShmInfo info;
    info.kdata_epoch = rd.getU64();
    info.kdata_name = rd.getString();
    // 向后兼容：基础信息快照字段为后续新增，旧版服务端仅回 kdata 两项；
    // 读到末尾后 rd.ok() 为假，仅丢弃 bi 字段，kdata 部分仍有效。
    if (rd.ok()) {
        info.bi_epoch = rd.getU64();
        info.bi_name = rd.getString();
    }

    {
        std::lock_guard<std::mutex> lock(m_shm_info_mutex);
        m_shm_info = info;
    }
    return true;
}

IpcConnector::ShmInfo IpcConnector::getShmInfo() const {
    std::lock_guard<std::mutex> lock(m_shm_info_mutex);
    return m_shm_info;
}

uint64_t IpcConnector::kdataShmEpoch() const {
    std::lock_guard<std::mutex> lock(m_shm_info_mutex);
    return m_shm_info.kdata_epoch;
}

uint64_t IpcConnector::biShmEpoch() const {
    std::lock_guard<std::mutex> lock(m_shm_info_mutex);
    return m_shm_info.bi_epoch;
}

///////////////////////////////////////////////////////////////////////////////
// IpcBaseInfoDriver
///////////////////////////////////////////////////////////////////////////////
IpcBaseInfoDriver::IpcBaseInfoDriver(const IpcConnectorPtr& conn, const BaseInfoDriverPtr& local)
: BaseInfoDriver("ipc"), m_conn(conn), m_local(local) {
    // 不在构造函数中协商共享内存快照：构造可能发生在连接池锁内，
    // 阻塞 IPC 会拖垮整个驱动连接池；首次查询入口会限流协商。
    m_shm_state = std::make_shared<BiShmState>();
}

bool IpcBaseInfoDriver::_init() {
    if (!m_conn || !m_conn->connected()) {
        HKU_WARN("Ipc connection is unavailable, IpcBaseInfoDriver will fallback to local!");
    }
    return true;
}

BaseInfoShmReaderPtr IpcBaseInfoDriver::_shmReader() const {
    std::shared_lock<std::shared_mutex> lock(m_shm_state->mutex);
    auto reader = m_shm_state->reader;
    return (reader && reader->valid()) ? reader : nullptr;
}

bool IpcBaseInfoDriver::_preferLocalDriver(const std::string& table,
                                           const BaseInfoShmReaderPtr& reader) const {
    // 快照已映射且其 coversTable 返回 false ⇒ 主进程未加载该项（受 load_stock_weight /
    // load_history_finance 门控），走 IPC 只会让主进程现场取数；本地驱动共享同一数据目录，
    // 结果一致且更快。快照尚未映射（reader 为空）时不偏好本地，保持原 IPC 优先行为。
    return m_local && reader && !reader->coversTable(table);
}

void IpcBaseInfoDriver::_tryRefreshShm() {
    if (!m_shm_enabled || !m_conn) {
        return;
    }
    // 探测由连接内统一限流（与 K 线代理驱动共用同一次协商）。返回值不可作为是否继续的依据：
    // 一个限流窗口内只有首个调用者拿到 true，若据此直接返回，则 K 线驱动与基础信息驱动
    // 会互相饿死（K 线查询频度高时，基础信息快照可能永远映射不上/换代不上）。
    // 故此处忽略返回值，一律读缓存代数并自行判断是否需要重映射。
    m_conn->refreshShmInfo();

    uint64_t epoch = m_conn->biShmEpoch();
    if (epoch == 0) {
        // 服务端尚未发布基础信息快照；若本地持有旧段则继续暂用，等待新段发布
        return;
    }

    {
        // 热路径：仅比对代数（不复制段名），已映射同代则直接返回
        std::shared_lock<std::shared_mutex> lock(m_shm_state->mutex);
        if (epoch == m_shm_state->epoch && m_shm_state->reader && m_shm_state->reader->valid()) {
            return;
        }
    }

    auto info = m_conn->getShmInfo();
    const std::string& name = info.bi_name;
    if (name.empty()) {
        return;
    }

    auto reader = std::make_shared<BaseInfoShmReader>();
    if (!reader->open(name)) {
        HKU_WARN("Failed map hikyuu base info shm cache: {}, fallback to ipc query!", name);
        return;
    }

    std::unique_lock<std::shared_mutex> lock(m_shm_state->mutex);
    if (m_shm_state->epoch == reader->epoch() && m_shm_state->reader &&
        m_shm_state->reader->valid()) {
        return;
    }
    // 换代时比对财务字段数：字段表变更（主进程数据集升级）会使客户端按旧下标读取
    // 新布局的 values（IFinance 以 values.at(field_ix) 取值，将抛 out_of_range），
    // 故显式告警提示重加载，避免故障时无从定位
    size_t new_value_count = reader->financeValueCount();
    if (m_shm_state->reader && m_shm_state->finance_value_count != 0 && new_value_count != 0 &&
        m_shm_state->finance_value_count != new_value_count) {
        HKU_WARN(
          "History finance field count changed in base info shm cache ({} -> {}), the local field "
          "index map may be stale, please reload hikyuu!",
          m_shm_state->finance_value_count, new_value_count);
    }
    m_shm_state->reader = reader;
    m_shm_state->epoch = reader->epoch();
    m_shm_state->finance_value_count = new_value_count;
    HKU_INFO("Mapped hikyuu base info shm cache: {} (epoch: {}, covered stock entries: {})", name,
             m_shm_state->epoch, reader->coveredCount());
}

std::vector<StockInfo> IpcBaseInfoDriver::getAllStockInfo() {
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_ALL_STOCK_INFO, {}, res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto infos = decodeStockInfoList(rd);
        HKU_IF_RETURN(rd.ok(), infos);
        HKU_ERROR("Failed decode stock info list from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getAllStockInfo");
    return m_local ? m_local->getAllStockInfo() : std::vector<StockInfo>();
}

StockInfo IpcBaseInfoDriver::getStockInfo(std::string market, const std::string& code) {
    Encoder enc;
    enc.putString(market + code);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_STOCK_INFO, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto info = decodeStockInfo(rd);
        HKU_IF_RETURN(rd.ok(), info);
        HKU_ERROR("Failed decode stock info from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getStockInfo");
    return m_local ? m_local->getStockInfo(market, code) : StockInfo();
}

StockWeightList IpcBaseInfoDriver::getStockWeightList(const std::string& market,
                                                      const std::string& code, Datetime start,
                                                      Datetime end) {
    _tryRefreshShm();
    auto reader = _shmReader();
    if (reader) {
        if (_preferLocalDriver(SHM_BI_TABLE_WEIGHT, reader)) {
            // 主进程未加载权息数据（load_stock_weight=false），本地驱动共享同一数据源，直读
            return m_local->getStockWeightList(market, code, start, end);
        }
        StockWeightList ws;
        if (reader->tryGetWeightList(market + code, start, end, ws)) {
            return ws;  // 快照命中（可能为空，表示该证券本无权息）
        }
        // 表已覆盖但本证券不在快照中：回退 IPC 让主进程从其本地驱动应答
    }
    Encoder enc;
    enc.putString(market);
    enc.putString(code);
    enc.putDatetime(start);
    enc.putDatetime(end);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_STOCK_WEIGHT_LIST, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto ws = decodeStockWeightList(rd);
        HKU_IF_RETURN(rd.ok(), ws);
        HKU_ERROR("Failed decode stock weight list from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getStockWeightList");
    return m_local ? m_local->getStockWeightList(market, code, start, end) : StockWeightList();
}

std::unordered_map<std::string, StockWeightList> IpcBaseInfoDriver::getAllStockWeightList() {
    _tryRefreshShm();
    auto reader = _shmReader();
    if (reader) {
        if (_preferLocalDriver(SHM_BI_TABLE_WEIGHT, reader)) {
            return m_local->getAllStockWeightList();
        }
        std::unordered_map<std::string, StockWeightList> result;
        if (reader->tryGetAllWeightList(result)) {
            return result;
        }
    }
    std::unordered_map<std::string, StockWeightList> result;
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_ALL_STOCK_WEIGHT_LIST, {}, res_body)) {
        Reader rd(res_body.data(), res_body.size());
        uint64_t count = rd.getU64();
        for (uint64_t i = 0; i < count && rd.ok(); i++) {
            std::string market_code = rd.getString();
            result[market_code] = decodeStockWeightList(rd);
        }
        HKU_IF_RETURN(!rd.ok(), m_local ? m_local->getAllStockWeightList() : result);
        return result;
    }
    HKU_WARN("Fallback to local driver: getAllStockWeightList");
    return m_local ? m_local->getAllStockWeightList() : result;
}

std::vector<HistoryFinanceInfo> IpcBaseInfoDriver::getHistoryFinance(const std::string& market,
                                                                     const std::string& code,
                                                                     Datetime start, Datetime end) {
    _tryRefreshShm();
    auto reader = _shmReader();
    if (reader) {
        if (_preferLocalDriver(SHM_BI_TABLE_FINANCE, reader)) {
            // 主进程未加载历史财务数据（load_history_finance=false），本地驱动直读
            return m_local->getHistoryFinance(market, code, start, end);
        }
        std::vector<HistoryFinanceInfo> fins;
        if (reader->tryGetHistoryFinance(market + code, start, end, fins)) {
            return fins;  // 快照命中（可能为空，表示该证券无财务记录）
        }
        // 表已覆盖但本证券不在快照中：回退 IPC 让主进程从其本地驱动应答
    }
    Encoder enc;
    enc.putString(market);
    enc.putString(code);
    enc.putDatetime(start);
    enc.putDatetime(end);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_HISTORY_FINANCE, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto fins = decodeHistoryFinanceList(rd);
        HKU_IF_RETURN(rd.ok(), fins);
        HKU_ERROR("Failed decode history finance from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getHistoryFinance");
    return m_local ? m_local->getHistoryFinance(market, code, start, end)
                   : std::vector<HistoryFinanceInfo>();
}

std::unordered_map<std::string, std::vector<HistoryFinanceInfo>>
IpcBaseInfoDriver::getAllHistoryFinance(const std::atomic_bool& cancel_flag) {
    std::unordered_map<std::string, std::vector<HistoryFinanceInfo>> result;
    _tryRefreshShm();
    auto reader = _shmReader();
    if (reader) {
        if (_preferLocalDriver(SHM_BI_TABLE_FINANCE, reader)) {
            // 主进程未加载历史财务数据（load_history_finance=false），本地驱动直读
            return m_local->getAllHistoryFinance(cancel_flag);
        }
        if (reader->tryGetAllHistoryFinance(result)) {
            return result;
        }
    }
    // 协议未设全量历史财务的命令字：该入口供列式驱动的批量初始化使用，数据量大，
    // 且客户端不会走到（客户端无预加载任务、IpcKDataDriver::isColumnFirst 为 false），
    // 故不为其扩充协议，快照未命中时直接回退本地驱动。
    // 必须 override：否则会落入基类实现（仅打 ERROR 并返回空、不回退），与其他方法
    // “失败即降级”的行为不一致，调用方无从感知
    HKU_WARN("Fallback to local driver: getAllHistoryFinance");
    if (m_local) {
        return m_local->getAllHistoryFinance(cancel_flag);
    }
    HKU_ERROR("Failed getAllHistoryFinance: neither shm cache nor local driver is available!");
    return result;
}

std::vector<std::pair<size_t, std::string>> IpcBaseInfoDriver::getHistoryFinanceField() {
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_HISTORY_FINANCE_FIELD, {}, res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto fields = decodeFinanceField(rd);
        HKU_IF_RETURN(rd.ok(), fields);
        HKU_ERROR("Failed decode history finance field from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getHistoryFinanceField");
    return m_local ? m_local->getHistoryFinanceField()
                   : std::vector<std::pair<size_t, std::string>>();
}

Parameter IpcBaseInfoDriver::getFinanceInfo(const std::string& market, const std::string& code) {
    Encoder enc;
    enc.putString(market);
    enc.putString(code);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_FINANCE_INFO, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto params = decodeParamMap(rd);
        if (rd.ok()) {
            Parameter result;
            for (const auto& item : params) {
                result.set<double>(item.first, item.second);
            }
            return result;
        }
        HKU_ERROR("Failed decode finance info from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getFinanceInfo");
    return m_local ? m_local->getFinanceInfo(market, code) : Parameter();
}

MarketInfo IpcBaseInfoDriver::getMarketInfo(const std::string& market) {
    Encoder enc;
    enc.putString(market);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_ALL_MARKET_INFO, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        uint64_t count = rd.getU64();
        for (uint64_t i = 0; i < count && rd.ok(); i++) {
            MarketInfo info = decodeMarketInfo(rd);
            if (rd.ok() && info.market() == market) {
                return info;
            }
        }
        if (rd.ok()) {
            return Null<MarketInfo>();
        }
        HKU_ERROR("Failed decode market info from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getMarketInfo");
    return m_local ? m_local->getMarketInfo(market) : Null<MarketInfo>();
}

std::vector<MarketInfo> IpcBaseInfoDriver::getAllMarketInfo() {
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_ALL_MARKET_INFO, {}, res_body)) {
        Reader rd(res_body.data(), res_body.size());
        std::vector<MarketInfo> result;
        // MarketInfo 最小编码尺寸 48B：4×putString(各 2B 长度前缀) + putDatetime(8B) + 4×putI64(各 8B)。
        // 用 getCount 而非裸 getU64：损坏帧的超大 count 会被校验拦截（置 rd 失败并返回 0），避免
        // reserve 天文数字抛 length_error/bad_alloc，转而经下方 !rd.ok() 优雅回退本地驱动
        uint64_t count = rd.getCount(48);
        result.reserve(count);
        for (uint64_t i = 0; i < count && rd.ok(); i++) {
            result.emplace_back(decodeMarketInfo(rd));
        }
        HKU_IF_RETURN(!rd.ok(), m_local ? m_local->getAllMarketInfo() : result);
        return result;
    }
    HKU_WARN("Fallback to local driver: getAllMarketInfo");
    return m_local ? m_local->getAllMarketInfo() : std::vector<MarketInfo>();
}

std::vector<StockTypeInfo> IpcBaseInfoDriver::getAllStockTypeInfo() {
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_ALL_STOCK_TYPE_INFO, {}, res_body)) {
        Reader rd(res_body.data(), res_body.size());
        std::vector<StockTypeInfo> result;
        // StockTypeInfo 最小编码尺寸 50B：putU32(4B) + putString(2B) + 3×putDouble(各 8B) +
        // putI32(4B) + 2×putDouble(各 8B)。同 getAllMarketInfo，用 getCount 拦截损坏帧超大 count
        uint64_t count = rd.getCount(50);
        result.reserve(count);
        for (uint64_t i = 0; i < count && rd.ok(); i++) {
            result.emplace_back(decodeStockTypeInfo(rd));
        }
        HKU_IF_RETURN(!rd.ok(), m_local ? m_local->getAllStockTypeInfo() : result);
        return result;
    }
    HKU_WARN("Fallback to local driver: getAllStockTypeInfo");
    return m_local ? m_local->getAllStockTypeInfo() : std::vector<StockTypeInfo>();
}

StockTypeInfo IpcBaseInfoDriver::getStockTypeInfo(uint32_t type) {
    // 直接获取全量证券类型信息，数据量很小
    auto infos = getAllStockTypeInfo();
    for (const auto& info : infos) {
        if (info.type() == type) {
            return info;
        }
    }
    return Null<StockTypeInfo>();
}

std::unordered_set<Datetime> IpcBaseInfoDriver::getAllHolidays() {
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_ALL_HOLIDAYS, {}, res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto holidays = decodeHolidaySet(rd);
        HKU_IF_RETURN(rd.ok(), holidays);
        HKU_ERROR("Failed decode holidays from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getAllHolidays");
    return m_local ? m_local->getAllHolidays() : std::unordered_set<Datetime>();
}

ZhBond10List IpcBaseInfoDriver::getAllZhBond10() {
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BASE_ALL_ZHBOND10, {}, res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto bonds = decodeZhBond10List(rd);
        HKU_IF_RETURN(rd.ok(), bonds);
        HKU_ERROR("Failed decode zhbond10 list from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getAllZhBond10");
    return m_local ? m_local->getAllZhBond10() : ZhBond10List();
}

///////////////////////////////////////////////////////////////////////////////
// IpcBlockDriver
///////////////////////////////////////////////////////////////////////////////
IpcBlockDriver::IpcBlockDriver(const IpcConnectorPtr& conn, const BlockInfoDriverPtr& local)
: BlockInfoDriver("ipc"), m_conn(conn), m_local(local) {}

bool IpcBlockDriver::_init() {
    if (!m_conn || !m_conn->connected()) {
        HKU_WARN("Ipc connection is unavailable, IpcBlockDriver will fallback to local!");
    }
    return true;
}

void IpcBlockDriver::load() {
    // 先在锁外完成 IPC/本地取数（可能阻塞一个往返），仅对最终的 m_blocks 赋值持写锁，
    // 避免长时间持锁阻塞读方；每日定时重载与用户遍历并发时不会撕裂 vector。
    BlockList new_blocks;
    bool ok = false;
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BLOCK_LOAD, {}, res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto blocks = decodeBlockList(rd);
        if (rd.ok()) {
            new_blocks = std::move(blocks);
            ok = true;
        } else {
            HKU_ERROR("Failed decode block list from ipc server!");
        }
    }

    if (!ok) {
        HKU_WARN("Fallback to local driver: load blocks");
        if (m_local) {
            m_local->load();
            new_blocks = m_local->getBlockList();
        }
        // 无本地驱动时 new_blocks 保持为空，等价于原先的 clear()
    }

    std::unique_lock<std::shared_mutex> lock(m_blocks_mutex);
    m_blocks = std::move(new_blocks);
}

StringList IpcBlockDriver::getAllCategory() {
    StringList result;
    std::shared_lock<std::shared_mutex> lock(m_blocks_mutex);
    for (const auto& blk : m_blocks) {
        if (std::find(result.begin(), result.end(), blk.category()) == result.end()) {
            result.push_back(blk.category());
        }
    }
    return result;
}

Block IpcBlockDriver::getBlock(const std::string& category, const std::string& name) {
    std::shared_lock<std::shared_mutex> lock(m_blocks_mutex);
    for (const auto& blk : m_blocks) {
        if (blk.category() == category && blk.name() == name) {
            return blk;
        }
    }
    return Block();
}

BlockList IpcBlockDriver::getBlockList(const std::string& category) {
    BlockList result;
    std::shared_lock<std::shared_mutex> lock(m_blocks_mutex);
    for (const auto& blk : m_blocks) {
        if (category.empty() || blk.category() == category) {
            result.push_back(blk);
        }
    }
    return result;
}

BlockList IpcBlockDriver::getBlockList() {
    std::shared_lock<std::shared_mutex> lock(m_blocks_mutex);
    return m_blocks;
}

void IpcBlockDriver::save(const Block& block) {
    HKU_THROW("The client mode is read-only, can not save block: {}:{}!", block.category(),
              block.name());
}

void IpcBlockDriver::remove(const std::string& category, const std::string& name) {
    HKU_THROW("The client mode is read-only, can not remove block: {}:{}!", category, name);
}

///////////////////////////////////////////////////////////////////////////////
// IpcKDataDriver
///////////////////////////////////////////////////////////////////////////////
IpcKDataDriver::IpcKDataDriver(const IpcConnectorPtr& conn,
                               const KDataDriverConnectPoolPtr& local_pool)
: KDataDriver("ipc"), m_conn(conn), m_local_pool(local_pool) {
    // 不在构造函数中协商共享内存快照：构造可能发生在连接池锁内，
    // 阻塞 IPC 会拖垮整个驱动连接池；首次查询入口会限流协商。
    m_shm_state = std::make_shared<ShmState>();
}

KDataDriverPtr IpcKDataDriver::_clone() {
    auto driver = std::make_shared<IpcKDataDriver>(m_conn, m_local_pool);
    // 共享快照状态：同一进程内所有克隆必须看到同一代快照，避免跨代索引漂移
    driver->m_shm_state = m_shm_state;
    driver->m_shm_enabled = m_shm_enabled;
    return driver;
}

KDataShmReaderPtr IpcKDataDriver::_shmReader() const {
    std::shared_lock<std::shared_mutex> lock(m_shm_state->mutex);
    auto reader = m_shm_state->reader;
    return (reader && reader->valid()) ? reader : nullptr;
}

bool IpcKDataDriver::_preferLocalDriver(const KQuery::KType& ktype) const {
    auto reader = _shmReader();
    return m_local_pool && reader && !reader->coversKType(ktype);
}

void IpcKDataDriver::_tryRefreshShm() {
    if (!m_shm_enabled || !m_conn) {
        return;
    }
    // 探测由连接内统一限流（与基础信息代理驱动共用同一次协商）。返回值不可作为是否继续的
    // 依据：一个限流窗口内只有首个调用者拿到 true，若据此直接返回，则两类驱动会互相
    // 饿死（K 线查询频度高时，基础信息快照可能永远映射不上；反之亦然）。
    // 故此处忽略返回值，一律读缓存代数并自行判断是否需要重映射。
    m_conn->refreshShmInfo();

    uint64_t epoch = m_conn->kdataShmEpoch();
    if (epoch == 0) {
        // 服务端尚未发布（或已撤销）快照；若本地持有旧段则继续暂用，等待新段发布，
        // 旧段数据在快照语义下仍为有效的历史子集；服务端重加载后数据变化由新 epoch 感知。
        return;
    }

    {
        // 热路径：仅比对代数（不复制段名），已映射同代则直接返回
        std::shared_lock<std::shared_mutex> lock(m_shm_state->mutex);
        if (epoch == m_shm_state->epoch && m_shm_state->reader && m_shm_state->reader->valid()) {
            return;
        }
    }

    auto info = m_conn->getShmInfo();
    const std::string& name = info.kdata_name;
    if (name.empty()) {
        return;
    }

    // 映射在锁外执行（含文件 IO 与 O(entries) 升序校验），避免阻塞其他克隆的查询
    auto reader = std::make_shared<KDataShmReader>();
    if (!reader->open(name)) {
        HKU_WARN("Failed map hikyuu kdata shm cache: {}, fallback to ipc query!", name);
        return;
    }

    std::unique_lock<std::shared_mutex> lock(m_shm_state->mutex);
    // 并发下其他克隆可能已完成同代映射，无需重复替换
    if (m_shm_state->epoch == reader->epoch() && m_shm_state->reader &&
        m_shm_state->reader->valid()) {
        return;
    }
    m_shm_state->reader = reader;
    m_shm_state->epoch = reader->epoch();
    HKU_INFO("Mapped hikyuu kdata shm cache: {} (epoch: {}, covered stocks entries: {})", name,
             m_shm_state->epoch, reader->coveredCount());
}

bool IpcKDataDriver::isIndexFirst() {
    // 快照缓冲序、服务端 KQuery(start_ix, end_ix) 与本地驱动的索引空间均为 0 = 最早记录，
    // 按位置检索恒快于按日期检索；返回 false 会让 Stock::getKRecord(Datetime)/getMarketValue
    // 退化为整包日期查询，放大传输量与服务端代价。
    return true;
}

bool IpcKDataDriver::canParallelLoad() {
    return false;
}

size_t IpcKDataDriver::getCount(const std::string& market, const std::string& code,
                                const KQuery::KType& kType) {
    // 优先共享内存快照；限流协商保证服务端发布/重发布新段后客户端能延迟感知；
    // 快照取自服务端缓冲，故条数与服务端缓冲模式下的 getCount 语义一致
    _tryRefreshShm();
    if (auto reader = _shmReader()) {
        size_t count = 0;
        if (reader->tryGetCount(market + code, kType, count)) {
            return count;
        }
    }

    // 主进程未预加载的类型不存在实时更新，服务端只能现场懒加载后序列化返回，
    // 直接走本地驱动更快，且主进程不可用（如定时重启窗口）时无需等待超时
    if (_preferLocalDriver(kType)) {
        auto local = m_local_pool->getConnect();
        return local ? local->getCount(market, code, kType) : 0;
    }

    Encoder enc;
    enc.putString(market + code);
    // 显式 int64_t，避免 Null<int64_t> 隐式转换选中 KQuery 的日期重载；
    // 服务端 getCount 仅使用 kType，查询区间不影响结果。
    encodeKQuery(enc, KQuery((int64_t)0, (int64_t)Null<int64_t>(), kType));
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::KDATA_COUNT, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        uint64_t count = rd.getU64();
        HKU_IF_RETURN(rd.ok(), static_cast<size_t>(count));
        HKU_ERROR("Failed decode kdata count from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getCount");
    auto local = m_local_pool ? m_local_pool->getConnect() : nullptr;
    return local ? local->getCount(market, code, kType) : 0;
}

bool IpcKDataDriver::getIndexRangeByDate(const std::string& market, const std::string& code,
                                         const KQuery& query, size_t& out_start, size_t& out_end) {
    _tryRefreshShm();
    if (auto reader = _shmReader()) {
        if (reader->tryGetIndexRangeByDate(market + code, query, out_start, out_end)) {
            return true;
        }
        // 区分“快照覆盖但区间为空”（返回 false 且 out 保持 0/0）与“快照未覆盖”（回退）：
        // 用 tryGetCount 判定覆盖性，与服务端缓冲模式行为对齐。
        size_t count = 0;
        if (reader->tryGetCount(market + code, query.kType(), count)) {
            out_start = 0;
            out_end = 0;
            return false;
        }
    }

    // 未预加载的类型直接走本地，理由同 getCount
    if (_preferLocalDriver(query.kType())) {
        auto local = m_local_pool->getConnect();
        return local ? local->getIndexRangeByDate(market, code, query, out_start, out_end) : false;
    }

    Encoder enc;
    enc.putString(market + code);
    encodeKQuery(enc, query);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::KDATA_INDEX_RANGE_BY_DATE, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        uint8_t ok = rd.getU8();
        uint64_t start = rd.getU64();
        uint64_t end = rd.getU64();
        if (rd.ok()) {
            out_start = static_cast<size_t>(start);
            out_end = static_cast<size_t>(end);
            return ok != 0;
        }
        HKU_ERROR("Failed decode kdata index range from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getIndexRangeByDate");
    auto local = m_local_pool ? m_local_pool->getConnect() : nullptr;
    return local ? local->getIndexRangeByDate(market, code, query, out_start, out_end) : false;
}

KRecordList IpcKDataDriver::getKRecordList(const std::string& market, const std::string& code,
                                           const KQuery& query) {
    // 优先共享内存快照（全市场遍历场景下避免逐证券 IPC 往返）
    _tryRefreshShm();
    // 单次调用内固定 reader 引用，避开刷新与读取之间的换代窗口
    bool unscale = needUnscalePrice(query.kType(), market, code);
    if (auto reader = _shmReader()) {
        KRecordList ks;
        if (reader->tryGetKRecordList(market + code, query, ks)) {
            // 快照取自服务端已加工的预加载缓冲，需还原为驱动原始值
            if (unscale) {
                unscalePrice(ks);
            }
            return ks;
        }
    }

    // 未预加载的类型直接走本地，理由同 getCount
    if (_preferLocalDriver(query.kType())) {
        auto local = m_local_pool->getConnect();
        return local ? local->getKRecordList(market, code, query) : KRecordList();
    }

    Encoder enc;
    enc.putString(market + code);
    encodeKQuery(enc, query);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::KDATA_GET_KRECORD_LIST, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto ks = decodeKRecordList(rd);
        if (rd.ok()) {
            // 服务端经 Stock 层应答，分时/分笔的 ETF/FUND/B 价格已缩放，需还原
            if (unscale) {
                unscalePrice(ks);
            }
            return ks;
        }
        HKU_ERROR("Failed decode krecord list from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getKRecordList");
    auto local = m_local_pool ? m_local_pool->getConnect() : nullptr;
    return local ? local->getKRecordList(market, code, query) : KRecordList();
}

bool IpcKDataDriver::tryGetKRecordView(const std::string& market, const std::string& code,
                                       const KQuery::KType& kType, size_t start_ix, size_t end_ix,
                                       KRecordView& out) {
    // 分时/分笔的 ETF/FUND/B 价格在服务端已缩放，需反缩放还原，无法裸指针视图；
    // 返回 false 由调用方回退 getKRecordList 拷贝路径（其内含 unscalePrice）
    if (needUnscalePrice(kType, market, code)) {
        return false;
    }
    _tryRefreshShm();
    // 单次调用内固定 reader 引用；out.pin 持有该 reader，使视图指针在 epoch 换代后仍有效
    auto reader = _shmReader();
    if (!reader) {
        return false;
    }
    const KRecord* data = nullptr;
    size_t count = 0;
    if (!reader->tryGetKRecordView(market + code, kType, start_ix, end_ix, data, count)) {
        return false;
    }
    out.data = data;
    out.count = count;
    out.pin = reader;  // 类型擦除持有映射存活
    return true;
}

TimeLineList IpcKDataDriver::getTimeLineList(const std::string& market, const std::string& code,
                                             const KQuery& query) {
    // 分时在主进程侧同样直接走驱动现场读取（Stock::getTimeLineList 不读预加载缓冲），
    // 行情代理也不为其注册处理函数，不存在实时更新；快照亦无分时读取接口。
    // 客户端与主进程共享同一数据目录（服务地址由 datadir 哈希派生），本地读取结果一致，
    // 故一律本地优先，省去序列化传输与服务端工作线程占用。
    if (m_local_pool) {
        auto local = m_local_pool->getConnect();
        if (local) {
            return local->getTimeLineList(market, code, query);
        }
    }

    Encoder enc;
    enc.putString(market + code);
    encodeKQuery(enc, query);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::KDATA_GET_TIMELINE_LIST, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto tls = decodeTimeLineList(rd);
        if (rd.ok()) {
            // 本地优先路径返回驱动原始值，而服务端经 Stock 层应答已缩放，此处还原以保持两者一致
            if (needUnscalePrice(KQuery::TIMELINE, market, code)) {
                unscalePrice(tls);
            }
            return tls;
        }
        HKU_ERROR("Failed decode timeline list from ipc server!");
    }
    HKU_WARN("Failed get timeline list from local driver and ipc server!");
    return TimeLineList();
}

TransList IpcKDataDriver::getTransList(const std::string& market, const std::string& code,
                                       const KQuery& query) {
    // 分笔同分时：主进程侧不读预加载缓冲且无实时更新，一律本地优先
    if (m_local_pool) {
        auto local = m_local_pool->getConnect();
        if (local) {
            return local->getTransList(market, code, query);
        }
    }

    Encoder enc;
    enc.putString(market + code);
    encodeKQuery(enc, query);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::KDATA_GET_TRANS_LIST, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto ts = decodeTransList(rd);
        if (rd.ok()) {
            // 同分时：还原服务端 Stock 层已施加的缩放
            if (needUnscalePrice(KQuery::TRANS, market, code)) {
                unscalePrice(ts);
            }
            return ts;
        }
        HKU_ERROR("Failed decode trans list from ipc server!");
    }
    HKU_WARN("Failed get trans list from local driver and ipc server!");
    return TransList();
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
