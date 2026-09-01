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
#include "hikyuu/utilities/Log.h"

namespace hku {
namespace ipc {

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
IpcKDataDriver::IpcKDataDriver(const IpcConnectorPtr& conn, const KDataDriverPtr& local)
: KDataDriver("ipc"), m_conn(conn), m_local(local) {}

KDataDriverPtr IpcKDataDriver::_clone() {
    return std::make_shared<IpcKDataDriver>(m_conn, m_local);
}

bool IpcKDataDriver::isIndexFirst() {
    return false;
}

bool IpcKDataDriver::canParallelLoad() {
    return false;
}

size_t IpcKDataDriver::getCount(const std::string& market, const std::string& code,
                                const KQuery::KType& kType) {
    Encoder enc;
    enc.putString(market + code);
    encodeKQuery(enc, KQuery(0, Null<int64_t>(), kType));
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::KDATA_COUNT, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        uint64_t count = rd.getU64();
        HKU_IF_RETURN(rd.ok(), static_cast<size_t>(count));
        HKU_ERROR("Failed decode kdata count from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getCount");
    return m_local ? m_local->getCount(market, code, kType) : 0;
}

bool IpcKDataDriver::getIndexRangeByDate(const std::string& market, const std::string& code,
                                         const KQuery& query, size_t& out_start, size_t& out_end) {
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
    return m_local ? m_local->getIndexRangeByDate(market, code, query, out_start, out_end)
                   : false;
}

KRecordList IpcKDataDriver::getKRecordList(const std::string& market, const std::string& code,
                                           const KQuery& query) {
    Encoder enc;
    enc.putString(market + code);
    encodeKQuery(enc, query);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::KDATA_GET_KRECORD_LIST, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto ks = decodeKRecordList(rd);
        HKU_IF_RETURN(rd.ok(), ks);
        HKU_ERROR("Failed decode krecord list from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getKRecordList");
    return m_local ? m_local->getKRecordList(market, code, query) : KRecordList();
}

TimeLineList IpcKDataDriver::getTimeLineList(const std::string& market, const std::string& code,
                                             const KQuery& query) {
    Encoder enc;
    enc.putString(market + code);
    encodeKQuery(enc, query);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::KDATA_GET_TIMELINE_LIST, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto tls = decodeTimeLineList(rd);
        HKU_IF_RETURN(rd.ok(), tls);
        HKU_ERROR("Failed decode timeline list from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getTimeLineList");
    return m_local ? m_local->getTimeLineList(market, code, query) : TimeLineList();
}

TransList IpcKDataDriver::getTransList(const std::string& market, const std::string& code,
                                       const KQuery& query) {
    Encoder enc;
    enc.putString(market + code);
    encodeKQuery(enc, query);
    std::vector<uint8_t> res_body;
    if (m_conn && m_conn->request(Cmd::KDATA_GET_TRANS_LIST, enc.data(), res_body)) {
        Reader rd(res_body.data(), res_body.size());
        auto ts = decodeTransList(rd);
        HKU_IF_RETURN(rd.ok(), ts);
        HKU_ERROR("Failed decode trans list from ipc server!");
    }
    HKU_WARN("Fallback to local driver: getTransList");
    return m_local ? m_local->getTransList(market, code, query) : TransList();
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
