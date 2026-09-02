/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-01
 *      Author: fasiondog
 */

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

#include <unordered_map>
#include "HikyuuDataServer.h"
#include "KDataShmCache.h"
#include "hikyuu/StockManager.h"
#include "hikyuu/utilities/Log.h"

namespace hku {
namespace ipc {

///////////////////////////////////////////////////////////////////////////////
// FileLock 跨平台独占文件锁，进程退出或崩溃后自动释放
///////////////////////////////////////////////////////////////////////////////
class FileLock {
public:
    FileLock() = default;

    ~FileLock() {
        release();
    }

    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;

    bool tryAcquire(const std::string& path) {
        release();
#if defined(_WIN32)
        m_handle = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, NULL);
        if (m_handle == INVALID_HANDLE_VALUE) {
            return false;
        }
        ZeroMemory(&m_ov, sizeof(m_ov));
        if (!LockFileEx(m_handle, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0,
                        &m_ov)) {
            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
            return false;
        }
        return true;
#else
        m_fd = ::open(path.c_str(), O_CREAT | O_RDWR, 0644);
        if (m_fd < 0) {
            return false;
        }
        if (::flock(m_fd, LOCK_EX | LOCK_NB) != 0) {
            ::close(m_fd);
            m_fd = -1;
            return false;
        }
        return true;
#endif
    }

    void release() {
#if defined(_WIN32)
        if (m_handle != INVALID_HANDLE_VALUE) {
            UnlockFileEx(m_handle, 0, 1, 0, &m_ov);
            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
        }
#else
        if (m_fd >= 0) {
            ::flock(m_fd, LOCK_UN);
            ::close(m_fd);
            m_fd = -1;
        }
#endif
    }

private:
#if defined(_WIN32)
    HANDLE m_handle{INVALID_HANDLE_VALUE};
    OVERLAPPED m_ov;
#else
    int m_fd{-1};
#endif
};

///////////////////////////////////////////////////////////////////////////////
// HikyuuDataServer
///////////////////////////////////////////////////////////////////////////////
HikyuuDataServer::HikyuuDataServer() = default;

HikyuuDataServer::~HikyuuDataServer() {
    stop();
}

bool HikyuuDataServer::start(const std::string& addr, const std::string& lock_path,
                             const std::string& tmpdir) {
    HKU_IF_RETURN(m_running, true);
    HKU_IF_RETURN(addr.empty() || lock_path.empty(), false);

    m_lock = std::make_unique<FileLock>();
    if (!m_lock->tryAcquire(lock_path)) {
        HKU_INFO("Another hikyuu data server is running, become client mode.");
        m_lock.reset();
        return false;
    }

    try {
        m_addr = addr;
        m_server.setAddr(addr);
        m_server.setHandler([this](Cmd cmd, std::vector<uint8_t>&& body, RetCode& ret) {
            return this->_handle(cmd, std::move(body), ret);
        });
        m_server.start();
    } catch (const std::exception& e) {
        HKU_ERROR("Failed start ipc data server! {}", e.what());
        m_lock.reset();
        return false;
    } catch (...) {
        HKU_ERROR("Failed start ipc data server! Unknown error!");
        m_lock.reset();
        return false;
    }

    m_running = true;
    return true;
}

void HikyuuDataServer::stop() {
    HKU_IF_RETURN(!m_running, void());
    m_running = false;
    m_server.stop();
    m_lock.reset();
}

void HikyuuDataServer::updateProgress(uint64_t loaded, uint64_t total) {
    m_kloaded.store(loaded, std::memory_order_relaxed);
    m_ktotal.store(total, std::memory_order_relaxed);
}

void HikyuuDataServer::setAllReady() noexcept {
    m_all_ready = true;
}

void HikyuuDataServer::refreshBlocks() {
    auto blocks = StockManager::instance().getBlockList("");
    std::unique_lock<std::shared_mutex> lock(m_block_mutex);
    m_blocks = std::move(blocks);
}

bool HikyuuDataServer::publishShmCache(const std::string& shm_name_prefix) {
    try {
        // 以当前时间戳作为代数，保证段名唯一且单调（快速重发布时递增），
        // 客户端以代数变化感知新快照；发布新段成功后由发布器删除旧段，
        // 已映射旧段的客户端不受影响（POSIX 段删除不影响已有映射）
        uint64_t epoch =
          static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::system_clock::now().time_since_epoch())
                                  .count());
        {
            std::shared_lock<std::shared_mutex> lock(m_shm_mutex);
            if (epoch <= m_shm_epoch) {
                epoch = m_shm_epoch + 1;
            }
        }
        if (!m_shm_publisher) {
            m_shm_publisher = std::make_unique<KDataShmPublisher>(shm_name_prefix);
        }
        std::string name = m_shm_publisher->publish(epoch);
        HKU_IF_RETURN(name.empty(), false);

        std::unique_lock<std::shared_mutex> lock(m_shm_mutex);
        m_shm_name = name;
        m_shm_epoch = epoch;
        return true;
    } catch (const std::exception& e) {
        HKU_ERROR("Failed publish kdata shm cache: {}", e.what());
    } catch (...) {
        HKU_ERROR("Failed publish kdata shm cache: unknown error!");
    }
    return false;
}

std::vector<uint8_t> HikyuuDataServer::_handle(Cmd cmd, std::vector<uint8_t>&& body, RetCode& ret) {
    Encoder enc;
    Reader rd(body.data(), body.size());
    auto& sm = StockManager::instance();

    switch (cmd) {
        case Cmd::STATUS_READY: {
            enc.putU8(m_all_ready ? 1 : 0);
            enc.putU64(m_kloaded.load(std::memory_order_relaxed));
            enc.putU64(m_ktotal.load(std::memory_order_relaxed));
            if (!m_all_ready) {
                ret = RetCode::LOADING;
            }
            break;
        }

        case Cmd::BASE_ALL_STOCK_INFO: {
            // 使用持锁快照，避免与证券字典的并发修改产生数据竞争；
            // 服务运行期间证券字典可能被重新加载线程修改（rehash 会使迭代器失效）
            auto stocks = sm.getStockList(nullptr);
            std::vector<StockInfo> infos;
            infos.reserve(stocks.size());
            for (const Stock& stk : stocks) {
                StockInfo info;
                info.market = stk.market();
                info.code = stk.code();
                info.name = stk.name();
                info.type = stk.type();
                info.valid = stk.valid() ? 1 : 0;
                // StockInfo 约定为 8 位 YYYYMMDD，客户端 loadAllStocks 会乘以 10000 还原；
                // 而 Datetime::number() 为 12 位，直接写入会导致客户端日期溢出为 Null，必须换算；
                // endDate 为无穷（Null）时，按 8 位上限编码，保证客户端 Datetime 构造不抛异常。
                info.startDate =
                  stk.startDatetime().isNull() ? 0 : stk.startDatetime().number() / 10000;
                info.endDate =
                  stk.lastDatetime().isNull() ? 99999999 : stk.lastDatetime().number() / 10000;
                info.precision = stk.precision();
                info.tick = stk.tick();
                info.tickValue = stk.tickValue();
                info.minTradeNumber = stk.minTradeNumber();
                info.maxTradeNumber = stk.maxTradeNumber();
                infos.emplace_back(std::move(info));
            }
            encodeStockInfoList(enc, infos);
            break;
        }

        case Cmd::BASE_STOCK_INFO: {
            std::string market_code = rd.getString();
            HKU_CHECK(rd.ok(), "Invalid request body!");
            Stock stk = sm.getStock(market_code);
            StockInfo info;
            if (!stk.isNull()) {
                info.market = stk.market();
                info.code = stk.code();
                info.name = stk.name();
                info.type = stk.type();
                info.valid = stk.valid() ? 1 : 0;
                // 同 BASE_ALL_STOCK_INFO：8 位 YYYYMMDD 约定，12 位写入会导致客户端日期溢出为
                // Null。
                info.startDate =
                  stk.startDatetime().isNull() ? 0 : stk.startDatetime().number() / 10000;
                info.endDate =
                  stk.lastDatetime().isNull() ? 99999999 : stk.lastDatetime().number() / 10000;
                info.precision = stk.precision();
                info.tick = stk.tick();
                info.tickValue = stk.tickValue();
                info.minTradeNumber = stk.minTradeNumber();
                info.maxTradeNumber = stk.maxTradeNumber();
            }
            encodeStockInfo(enc, info);
            break;
        }

        case Cmd::BASE_ALL_MARKET_INFO: {
            auto markets = sm.getAllMarket();
            enc.putU64(markets.size());
            for (const auto& market : markets) {
                encodeMarketInfo(enc, sm.getMarketInfo(market));
            }
            break;
        }

        case Cmd::BASE_ALL_STOCK_TYPE_INFO: {
            auto infos = sm.getStockTypeInfoList();
            enc.putU64(infos.size());
            for (const auto& info : infos) {
                encodeStockTypeInfo(enc, info);
            }
            break;
        }

        case Cmd::BASE_ALL_HOLIDAYS: {
            auto driver = sm.getBaseInfoDriver();
            HKU_CHECK(driver, "BaseInfo driver is null!");
            auto holidays = driver->getAllHolidays();
            encodeHolidaySet(enc, holidays);
            break;
        }

        case Cmd::BASE_ALL_ZHBOND10: {
            encodeZhBond10List(enc, sm.getZhBond10());
            break;
        }

        case Cmd::BASE_STOCK_WEIGHT_LIST: {
            std::string market = rd.getString();
            std::string code = rd.getString();
            Datetime start = rd.getDatetime();
            Datetime end = rd.getDatetime();
            HKU_CHECK(rd.ok(), "Invalid request body!");
            Stock stk = sm.getStock(market + code);
            HKU_CHECK(!stk.isNull(), "Not found stock: {}{}!", market, code);
            encodeStockWeightList(enc, stk.getWeight(start, end));
            break;
        }

        case Cmd::BASE_ALL_STOCK_WEIGHT_LIST: {
            // 使用持锁快照，避免与证券字典的并发修改产生数据竞争；快照后再逐一取权息，减少持锁时间。
            auto stocks = sm.getStockList(nullptr);
            enc.putU64(stocks.size());
            for (const Stock& stk : stocks) {
                enc.putString(stk.market_code());
                encodeStockWeightList(enc, stk.getWeight());
            }
            break;
        }

        case Cmd::BASE_HISTORY_FINANCE_FIELD: {
            encodeFinanceField(enc, sm.getHistoryFinanceAllFields());
            break;
        }

        case Cmd::BASE_HISTORY_FINANCE: {
            std::string market = rd.getString();
            std::string code = rd.getString();
            Datetime start = rd.getDatetime();
            Datetime end = rd.getDatetime();
            HKU_CHECK(rd.ok(), "Invalid request body!");
            auto driver = sm.getBaseInfoDriver();
            HKU_CHECK(driver, "BaseInfo driver is null!");
            auto infos = driver->getHistoryFinance(market, code, start, end);
            encodeHistoryFinanceList(enc, infos);
            break;
        }

        case Cmd::BASE_FINANCE_INFO: {
            std::string market = rd.getString();
            std::string code = rd.getString();
            HKU_CHECK(rd.ok(), "Invalid request body!");
            auto driver = sm.getBaseInfoDriver();
            HKU_CHECK(driver, "BaseInfo driver is null!");
            Parameter finance = driver->getFinanceInfo(market, code);
            std::unordered_map<std::string, double> params;
            for (const auto& name : finance.getNameList()) {
                params[name] = finance.tryGet<double>(name, 0.0);
            }
            encodeParamMap(enc, params);
            break;
        }

        case Cmd::KDATA_COUNT: {
            std::string market_code = rd.getString();
            KQuery query = decodeKQuery(rd);
            HKU_CHECK(rd.ok(), "Invalid request body!");
            Stock stk = sm.getStock(market_code);
            HKU_CHECK(!stk.isNull(), "Not found stock: {}!", market_code);
            enc.putU64(stk.getCount(query.kType()));
            break;
        }

        case Cmd::KDATA_INDEX_RANGE_BY_DATE: {
            std::string market_code = rd.getString();
            KQuery query = decodeKQuery(rd);
            HKU_CHECK(rd.ok(), "Invalid request body!");
            Stock stk = sm.getStock(market_code);
            HKU_CHECK(!stk.isNull(), "Not found stock: {}!", market_code);
            size_t start_ix = 0, end_ix = 0;
            bool ok = stk.getIndexRange(query, start_ix, end_ix);
            enc.putU8(ok ? 1 : 0);
            enc.putU64(start_ix);
            enc.putU64(end_ix);
            break;
        }

        case Cmd::KDATA_GET_KRECORD_LIST: {
            std::string market_code = rd.getString();
            KQuery query = decodeKQuery(rd);
            HKU_CHECK(rd.ok(), "Invalid request body!");
            Stock stk = sm.getStock(market_code);
            HKU_CHECK(!stk.isNull(), "Not found stock: {}!", market_code);
            encodeKRecordList(enc, stk.getKRecordList(query));
            break;
        }

        case Cmd::KDATA_GET_TIMELINE_LIST: {
            std::string market_code = rd.getString();
            KQuery query = decodeKQuery(rd);
            HKU_CHECK(rd.ok(), "Invalid request body!");
            Stock stk = sm.getStock(market_code);
            HKU_CHECK(!stk.isNull(), "Not found stock: {}!", market_code);
            encodeTimeLineList(enc, stk.getTimeLineList(query));
            break;
        }

        case Cmd::KDATA_GET_TRANS_LIST: {
            std::string market_code = rd.getString();
            KQuery query = decodeKQuery(rd);
            HKU_CHECK(rd.ok(), "Invalid request body!");
            Stock stk = sm.getStock(market_code);
            HKU_CHECK(!stk.isNull(), "Not found stock: {}!", market_code);
            encodeTransList(enc, stk.getTransList(query));
            break;
        }

        case Cmd::BLOCK_LOAD: {
            std::shared_lock<std::shared_mutex> lock(m_block_mutex);
            encodeBlockList(enc, m_blocks);
            break;
        }

        case Cmd::STATUS_SHM_INFO: {
            std::shared_lock<std::shared_mutex> lock(m_shm_mutex);
            enc.putU64(m_shm_epoch);
            enc.putString(m_shm_name);
            break;
        }

        default:
            HKU_ERROR("Unknown ipc cmd: {}!", static_cast<uint32_t>(cmd));
            ret = RetCode::ERROR;
            enc.putString("Unknown cmd!");
            break;
    }

    return enc.data();
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
