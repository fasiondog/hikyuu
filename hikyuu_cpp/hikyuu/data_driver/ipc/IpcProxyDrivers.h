/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-01
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include "BaseInfoShmCache.h"
#include "IpcTransport.h"
#include "KDataShmCache.h"
#include "hikyuu/data_driver/BaseInfoDriver.h"
#include "hikyuu/data_driver/BlockInfoDriver.h"
#include "hikyuu/data_driver/DataDriverFactory.h"
#include "hikyuu/data_driver/KDataDriver.h"

namespace hku {
namespace ipc {

/**
 * IPC 共享连接，供各代理驱动复用同一个 nng REQ 通道
 * @ingroup DataDriver
 */
class HKU_API IpcConnector {
public:
    /** 服务端发布的两类共享内存快照信息 */
    struct ShmInfo {
        uint64_t kdata_epoch{0};     ///< K 线快照代数
        std::string kdata_name;      ///< K 线快照段名，空表示尚未发布
        uint64_t bi_epoch{0};        ///< 基础信息（权息/历史财务）快照代数
        std::string bi_name;         ///< 基础信息段名，空表示尚未发布
    };

    IpcConnector() = default;
    explicit IpcConnector(const std::string& addr);
    ~IpcConnector() = default;

    IpcConnector(const IpcConnector&) = delete;
    IpcConnector& operator=(const IpcConnector&) = delete;

    /** 连接服务端，成功返回 true */
    bool init(const std::string& addr);

    bool connected() const noexcept {
        return m_client.connected();
    }

    const std::string& addr() const noexcept {
        return m_addr;
    }

    /**
     * 发送命令并接收响应体
     * @param cmd 命令字
     * @param body 请求体
     * @param res_body [out] 响应体
     * @return true 收到 SUCCESS 响应 | false 通讯失败或服务端返回 ERROR
     */
    bool request(Cmd cmd, const std::vector<uint8_t>& body, std::vector<uint8_t>& res_body);

    /**
     * 发送轻量探测类请求（如快照协商）
     * @details 与 request 的区别：使用短超时，且连续失败后按指数退避抑制后续探测，
     * 避免服务端不可用期间（如每日定时重启窗口）每次查询都阻塞一个完整超时周期。
     * 退避状态由本连接器持有，各代理驱动克隆实例共享。
     * @param cmd 命令字
     * @param res_body [out] 响应体
     * @return true 探测成功 | false 处于退避抑制期、通讯失败或服务端返回 ERROR
     */
    bool probeRequest(Cmd cmd, std::vector<uint8_t>& res_body);

    /**
     * 阻塞等待服务端数据全部就绪
     * @param timeout_seconds 超时秒数，0 表示无限等待
     * @param progress_cb 加载中进度回调 (已加载, 总数)
     * @return true 就绪 | false 超时或通讯失败
     */
    bool waitReady(uint64_t timeout_seconds,
                   std::function<void(uint64_t, uint64_t)>&& progress_cb = nullptr);

    /**
     * 拉取服务端共享内存快照信息（带最小间隔限流，探测退避由 probeRequest 统一处理）
     * @details 结果在本连接内缓存，K 线代理驱动与基础信息代理驱动共用同一次探测，
     * 避免两个驱动各自发起一轮 IPC。限流未到期时不发起探测，**调用方仍须读取缓存**
     * （见 getShmInfo / kdataShmEpoch / biShmEpoch）并自行判断是否需要重映射：
     * 一个限流窗口内只有首个调用者能拿到 true，若其余调用者据此直接返回，
     * 两类驱动会互相饿死，导致某一类快照长期映射不上、换代不上。
     * @return true 本次确实发起了探测并收到响应 | false 限流未到期或通讯失败（仅作诊断用）
     */
    bool refreshShmInfo();

    /** 取缓存的快照信息（可能为零值，表示服务端尚未发布） */
    ShmInfo getShmInfo() const;

    /**
     * 取缓存的 K 线快照代数（轻量，不复制段名）
     * @note 供查询热路径先比对代数，仅在代数变化时才取完整 ShmInfo 并重映射
     */
    uint64_t kdataShmEpoch() const;

    /** 取缓存的基础信息快照代数（轻量，不复制段名），语义同 kdataShmEpoch */
    uint64_t biShmEpoch() const;

private:
    std::string m_addr;
    IpcClient m_client;
    std::mutex m_probe_mutex;
    std::chrono::steady_clock::time_point m_last_probe;
    int m_probe_fails{0};  ///< 连续探测失败次数，成功后归零

    mutable std::mutex m_shm_info_mutex;
    ShmInfo m_shm_info;
    std::chrono::steady_clock::time_point m_shm_last_check;
};

typedef std::shared_ptr<IpcConnector> IpcConnectorPtr;

/**
 * 注册客户端实时更新转发连接（进入客户端模式时调用，传 nullptr 注销）
 * @details 客户端无预加载缓冲，Stock::realtimeUpdate 无法本地应用；注册后
 * 经 ipcForwardRealtimeUpdate 转发至主进程，由主进程应用到缓冲并镜像至
 * 共享内存段，全体客户端由此读到该更新（与主进程自身行情接收幂等收敛）。
 */
HKU_API void registerRealtimeForwarder(const IpcConnectorPtr& conn);

/**
 * 客户端进程将实时 K 线更新转发至主进程（Stock::realtimeUpdate 调用）
 * @return true 主进程已应用 | false 未注册转发连接、通讯失败或主进程未应用
 *         （如证券不存在、无对应缓冲），调用方仅记录日志不中断流程
 */
HKU_API bool ipcForwardRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                                      const KRecord& record);

/**
 * 客户端进程向主进程查询指定证券×类型的缓冲最后更新时刻（Stock::getLastUpdateTime 调用）
 * @details 客户端无预加载缓冲，本地 m_lastUpdate 恒为 Datetime::min()；转发取主进程
 * 缓冲的刷新时刻，与客户端经共享内存读到的数据保持一致。
 * @return 主进程缓冲的最后更新时刻；未注册转发连接、通讯失败或证券不存在时返回 Datetime::min()
 */
HKU_API Datetime ipcForwardGetLastUpdateTime(const std::string& market_code,
                                             const KQuery::KType& ktype);

/**
 * 基础信息 IPC 代理驱动
 * @details 从数据服务进程获取证券基础信息，通讯失败时降级至本地驱动
 */
class HKU_API IpcBaseInfoDriver : public BaseInfoDriver {
public:
    IpcBaseInfoDriver(const IpcConnectorPtr& conn, const BaseInfoDriverPtr& local);

    bool _init() override;

    std::vector<StockInfo> getAllStockInfo() override;
    StockInfo getStockInfo(std::string market, const std::string& code) override;
    StockWeightList getStockWeightList(const std::string& market, const std::string& code,
                                       Datetime start, Datetime end) override;
    std::unordered_map<std::string, StockWeightList> getAllStockWeightList() override;
    std::vector<HistoryFinanceInfo> getHistoryFinance(const std::string& market,
                                                      const std::string& code, Datetime start,
                                                      Datetime end) override;
    std::unordered_map<std::string, std::vector<HistoryFinanceInfo>> getAllHistoryFinance(
      const std::atomic_bool& cancel_flag) override;
    std::vector<std::pair<size_t, std::string>> getHistoryFinanceField() override;
    Parameter getFinanceInfo(const std::string& market, const std::string& code) override;
    MarketInfo getMarketInfo(const std::string& market) override;
    std::vector<MarketInfo> getAllMarketInfo() override;
    std::vector<StockTypeInfo> getAllStockTypeInfo() override;
    StockTypeInfo getStockTypeInfo(uint32_t type) override;
    std::unordered_set<Datetime> getAllHolidays() override;
    ZhBond10List getAllZhBond10() override;

private:
    /**
     * 快照状态
     * @details 权息与历史财务经共享内存快照零拷贝读取，缺失时回退 IPC，
     * 主进程未加载的表直接由本地驱动服务（见 _preferLocalDriver）。
     */
    struct BiShmState {
        std::shared_mutex mutex;
        BaseInfoShmReaderPtr reader;
        uint64_t epoch{0};
        size_t finance_value_count{0};  ///< 已映射段的财务字段数，用于换代时侦测字段表变更
    };
    typedef std::shared_ptr<BiShmState> BiShmStatePtr;

    /** 取当前快照读取器的引用副本，保证单次调用内不跨代 */
    BaseInfoShmReaderPtr _shmReader() const;

    /** 映射服务端最新发布的基础信息快照（探测由 IpcConnector 统一限流/退避） */
    void _tryRefreshShm();

    /**
     * 判断指定表是否应直接由本地驱动服务（跳过 IPC）
     * @details 快照中缺失的表意味着主进程未加载该项（分别受 load_stock_weight /
     * load_history_finance 门控），走 IPC 只会让主进程现场取数并序列化；
     * 客户端与主进程共享同一数据目录，本地读取结果一致且更快。
     * @note 快照尚未映射时返回 false，保持原有 IPC 优先行为
     * @param table 表名（SHM_BI_TABLE_WEIGHT / SHM_BI_TABLE_FINANCE）
     * @param reader 调用方已取出的快照读取器，复用之以免单次查询内重复加锁取到不同代
     */
    bool _preferLocalDriver(const std::string& table, const BaseInfoShmReaderPtr& reader) const;

    IpcConnectorPtr m_conn;
    BaseInfoDriverPtr m_local;  // 降级兜底
    bool m_shm_enabled{true};
    BiShmStatePtr m_shm_state;
};

/**
 * 板块 IPC 代理驱动
 * @details 初始化时一次性从数据服务进程拉取全部板块并缓存，客户端只读
 */
class HKU_API IpcBlockDriver : public BlockInfoDriver {
public:
    IpcBlockDriver(const IpcConnectorPtr& conn, const BlockInfoDriverPtr& local);

    bool _init() override;
    void load() override;
    StringList getAllCategory() override;
    Block getBlock(const std::string& category, const std::string& name) override;
    BlockList getBlockList(const std::string& category) override;
    BlockList getBlockList() override;
    void save(const Block& block) override;
    void remove(const std::string& category, const std::string& name) override;

private:
    IpcConnectorPtr m_conn;
    BlockInfoDriverPtr m_local;  // 降级兜底
    // 每日定时重载（reloadHikyuuTask）在调度线程对 m_blocks 做 move-assign，
    // 与用户线程的遍历并发，故需读写锁保护（与被替换的 SQLite/MySQL 板块驱动对齐）
    std::shared_mutex m_blocks_mutex;
    BlockList m_blocks;
};

/**
 * K 线 IPC 代理驱动
 * @details 优先从服务端发布的只读共享内存缓存快照直接读取热数据（零拷贝），
 * 快照未覆盖的查询回退 IPC 请求；主进程未预加载的类型不存在实时更新链路，
 * 直接由本地驱动服务（与主进程共享同一数据目录）。
 * 服务端预加载完成时间可能晚于客户端接入，未命中时按最小间隔向服务端
 * 拉取最新快照信息。
 */
class HKU_API IpcKDataDriver : public KDataDriver {
public:
    /**
     * @param conn 共享的 IPC 连接
     * @param local_pool 本地 K 线驱动连接池，用于本地优先与降级兜底路径。
     *        必须持有整个池而非其 prototype：各驱动实例持有自己的连接/文件句柄
     *        （如 MySQLKDataDriver::m_connect、H5KDataDriver::m_h5file_map），
     *        多个克隆共享同一 prototype 将导致并发复用同一连接。
     */
    IpcKDataDriver(const IpcConnectorPtr& conn, const KDataDriverConnectPoolPtr& local_pool);

    KDataDriverPtr _clone() override;
    bool isIndexFirst() override;
    bool canParallelLoad() override;
    size_t getCount(const std::string& market, const std::string& code,
                    const KQuery::KType& kType) override;
    bool getIndexRangeByDate(const std::string& market, const std::string& code,
                             const KQuery& query, size_t& out_start, size_t& out_end) override;
    KRecordList getKRecordList(const std::string& market, const std::string& code,
                               const KQuery& query) override;
    TimeLineList getTimeLineList(const std::string& market, const std::string& code,
                                 const KQuery& query) override;
    TransList getTransList(const std::string& market, const std::string& code,
                           const KQuery& query) override;

private:
    /** 向服务端拉取共享内存快照信息并映射新段（带最小间隔限流） */
    void _tryRefreshShm();

    /**
     * 判断指定类型是否应直接由本地驱动服务（跳过 IPC）
     * @details 快照的 ktype 表列出主进程已预加载的类型，而主进程仅为已预加载的类型
     * 注册行情处理函数（见 GlobalSpotAgent），故未预加载的类型在主进程侧不存在实时更新；
     * 客户端与主进程共享同一数据目录（服务地址由 datadir 哈希派生），本地读取结果一致，
     * 走 IPC 只会让主进程现场懒加载并序列化传输，且主进程不可用时还要白等一个超时周期。
     * @note 快照尚未映射时返回 false，保持原有 IPC 优先行为
     */
    bool _preferLocalDriver(const KQuery::KType& ktype) const;

    /** 取当前快照读取器的引用副本，保证单次驱动调用内不跨代 */
    KDataShmReaderPtr _shmReader() const;

    /**
     * 各克隆实例共享的快照状态
     * @details 状态若为实例私有，连接池中的不同克隆会各自映射到不同代（epoch）的段，
     * 同一次逻辑查询的多次驱动调用（如先 getCount 再 getKRecordList）可能落在两代快照上，
     * 造成索引空间漂移。共享后同一进程内任一时刻只有一代快照生效。
     */
    struct ShmState {
        std::shared_mutex mutex;
        KDataShmReaderPtr reader;
        uint64_t epoch{0};
    };
    typedef std::shared_ptr<ShmState> ShmStatePtr;

    IpcConnectorPtr m_conn;
    KDataDriverConnectPoolPtr m_local_pool;  // 本地优先/降级兜底
    bool m_shm_enabled{true};
    ShmStatePtr m_shm_state;  ///< 由首个实例创建，_clone 时共享
};

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
