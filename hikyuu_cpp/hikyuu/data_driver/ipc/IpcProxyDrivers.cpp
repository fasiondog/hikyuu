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

/// 还原 KRecordList 中服务端已施加的价格缩放
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

///////////////////////////////////////////////////////////////////////////////
// IpcBaseInfoDriver
///////////////////////////////////////////////////////////////////////////////
IpcBaseInfoDriver::IpcBaseInfoDriver(const IpcConnectorPtr& conn, const BaseInfoDriverPtr& local)
: BaseInfoDriver("ipc"), m_conn(conn), m_local(local) {}

bool IpcBaseInfoDriver::_init() {
    if (!m_conn || !m_conn->connected()) {
        HKU_WARN("Ipc connection is unavailable, IpcBaseInfoDriver will fallback to local!");
    }
    return true;
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
        uint64_t count = rd.getU64();
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
        uint64_t count = rd.getU64();
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
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::BLOCK_LOAD, {}, res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto blocks = decodeBlockList(rd);
        if (rd.ok()) {
            m_blocks = std::move(blocks);
            return;
        }
        HKU_ERROR("Failed decode block list from ipc server!");
    }

    HKU_WARN("Fallback to local driver: load blocks");
    if (m_local) {
        m_local->load();
        m_blocks = m_local->getBlockList();
    } else {
        m_blocks.clear();
    }
}

StringList IpcBlockDriver::getAllCategory() {
    StringList result;
    for (const auto& blk : m_blocks) {
        if (std::find(result.begin(), result.end(), blk.category()) == result.end()) {
            result.push_back(blk.category());
        }
    }
    return result;
}

Block IpcBlockDriver::getBlock(const std::string& category, const std::string& name) {
    for (const auto& blk : m_blocks) {
        if (blk.category() == category && blk.name() == name) {
            return blk;
        }
    }
    return Block();
}

BlockList IpcBlockDriver::getBlockList(const std::string& category) {
    BlockList result;
    for (const auto& blk : m_blocks) {
        if (category.empty() || blk.category() == category) {
            result.push_back(blk);
        }
    }
    return result;
}

BlockList IpcBlockDriver::getBlockList() {
    return m_blocks;
}

void IpcBlockDriver::save(const Block& block) {
    HKU_WARN("The client mode is read-only, can not save block: {}:{}!", block.category(),
             block.name());
}

void IpcBlockDriver::remove(const std::string& category, const std::string& name) {
    HKU_WARN("The client mode is read-only, can not remove block: {}:{}!", category, name);
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
    if (!m_shm_enabled || !m_conn || !m_conn->connected()) {
        return;
    }
    // 拉取限流：无论服务端是否已发布，最小间隔内最多协商一次，
    // 避免高频未命中查询（如快照未覆盖的证券、越界区间）退化为每查询一次额外 IPC；
    // 服务端发布新段后客户端最多延迟该间隔感知；分时/分笔本地优先，不参与协商。
    // 服务端不可用时的额外抑制（短超时 + 指数退避）由 probeRequest 在各克隆实例间统一处理。
    constexpr int64_t MIN_CHECK_INTERVAL_MS = 5000;
    auto now = std::chrono::steady_clock::now();
    {
        std::unique_lock<std::shared_mutex> lock(m_shm_state->mutex);
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_shm_state->last_check)
              .count() < MIN_CHECK_INTERVAL_MS) {
            return;
        }
        m_shm_state->last_check = now;
    }

    std::vector<uint8_t> res_body;
    if (!m_conn->probeRequest(Cmd::STATUS_SHM_INFO, res_body)) {
        return;  // 退避抑制或通讯失败，保持现状，查询自然回退 IPC/本地
    }

    Reader rd(res_body.data(), res_body.size());
    uint64_t epoch = rd.getU64();
    std::string name = rd.getString();
    HKU_IF_RETURN(!rd.ok(), void());

    if (epoch == 0 || name.empty()) {
        // 服务端尚未发布（或已撤销）快照；若本地持有旧段则继续暂用，等待新段发布，
        // 旧段数据在快照语义下仍为有效的历史子集；服务端重加载后数据变化由新 epoch 感知。
        return;
    }

    {
        std::shared_lock<std::shared_mutex> lock(m_shm_state->mutex);
        if (epoch == m_shm_state->epoch && m_shm_state->reader && m_shm_state->reader->valid()) {
            return;
        }
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
