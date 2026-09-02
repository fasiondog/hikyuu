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
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>

// boost/interprocess/detail/workaround.hpp 会传递引入 <sys/param.h>（macOS/Linux），
// 其定义的 ALIGN/MAX/MIN 宏与 hikyuu 同名指标函数冲突：ALIGN 导致宏展开编译失败，
// MAX/MIN 更会静默改变语义。crt/*.h 中的 #undef 受 include guard 限制，在 unity build
// 下仅首次展开生效，故必须在宏的引入点立即消解。
#if defined(ALIGN)
#undef ALIGN
#endif
#if defined(MAX)
#undef MAX
#endif
#if defined(MIN)
#undef MIN
#endif

#include "hikyuu/KQuery.h"
#include "hikyuu/KRecord.h"

namespace hku {
namespace ipc {

constexpr uint32_t SHM_CACHE_MAGIC = 0x53434B48;  // "HKCS" 小端
constexpr uint32_t SHM_CACHE_VERSION = 2;

/*
 * K 线共享内存缓存段布局（全定长、无指针）：
 * [ShmCacheHeader][ShmKTypeInfo × ktype_count]
 * 每个 ktype：[ShmStockEntry × entry_count，按 market_code 升序，供二分查找]
 * 记录区：每证券 [已发布数据][1 个交易日预留区]，主进程实时数据镜像写入尾部
 * 并发模型：单写者（主进程 realtimeUpdate 持证券×ktype 写锁）多读者（客户端进程），
 * entry 级 seqlock + 原子 record_count 保证读端无撕裂；历史区发布后不可变。
 * 段名含代数（epoch），重发布创建新段并删除旧段，读端已映射的旧段不受影响。
 */

/** K线记录（与网络协议编码一致，datetime 为 12 位 YYYYMMDDHHMM） */
struct ShmKRecord {
    uint64_t datetime;
    double openPrice;
    double highPrice;
    double lowPrice;
    double closePrice;
    double transAmount;
    double transCount;
};

/** 证券索引项（record_count/seq 为跨进程原子量，需 lock-free） */
struct ShmStockEntry {
    char market_code[16];                ///< NUL 填充，如 "SH600000"
    uint64_t record_offset;              ///< 记录区相对段首字节偏移
    std::atomic<uint64_t> record_count;  ///< 当前有效记录数（镜像追加时增长）
    uint64_t record_capacity;            ///< 记录区容量 = 发布数 + 1 交易日预留
    std::atomic<uint32_t> seq;           ///< seqlock 序号，奇数表示写入中
    uint32_t _pad;
};

static_assert(std::atomic<uint64_t>::is_always_lock_free &&
                std::atomic<uint32_t>::is_always_lock_free,
              "ShmStockEntry atomics must be lock-free for cross-process use!");

/** ktype 索引表项 */
struct ShmKTypeInfo {
    char ktype[16];         ///< NUL 填充，如 "DAY"（需容纳 "HALFYEAR"/"TIMELINE" 等 8 字符类型）
    uint64_t entry_offset;  ///< entry 数组相对段首字节偏移
    uint32_t entry_count;
    uint32_t _pad;
};

/** 段头 */
struct ShmCacheHeader {
    std::atomic<uint32_t> magic;  ///< 完整性判据，数据全部写入后最后以 release 落盘
    uint32_t version;
    uint64_t epoch;      ///< 代数，每次发布唯一
    uint64_t data_size;  ///< 段实际数据字节数（发布完成后写入）
    uint32_t ktype_count;
    uint32_t _pad;
};

// magic 改为原子量后仍须与 uint32_t 同宽同对齐，保证段布局与既有版本字节兼容
static_assert(sizeof(ShmCacheHeader) == 32 && sizeof(std::atomic<uint32_t>) == sizeof(uint32_t) &&
                alignof(std::atomic<uint32_t>) == alignof(uint32_t),
              "ShmCacheHeader layout must stay compatible with plain uint32_t magic!");

class KDataShmPublisher;
typedef std::shared_ptr<KDataShmPublisher> KDataShmPublisherPtr;

/**
 * 主进程端：将预加载的 K 线缓冲发布为共享内存缓存，并镜像后续实时更新
 * @details 快照包含 [preload] 中开启且已缓冲的基础 K 线类型（分时/分笔亦在基础类型之列，
 * 配置开启时同样发布，但 Stock::getTimeLineList / getTransList 不读该缓冲）。
 * 每证券按 ktype 预留 1 个交易日容量，Stock::realtimeUpdate 时同步镜像写入，
 * 客户端读到准实时数据；预留区写满后停止镜像（每日定时重启恢复）。
 * @ingroup DataDriver
 */
class HKU_API KDataShmPublisher {
public:
    /**
     * @param shm_name_prefix 段名前缀（受系统共享内存名长度限制，建议不超过 11 字符）
     */
    explicit KDataShmPublisher(const std::string& shm_name_prefix);
    ~KDataShmPublisher();

    KDataShmPublisher(const KDataShmPublisher&) = delete;
    KDataShmPublisher& operator=(const KDataShmPublisher&) = delete;

    /**
     * 构建并发布快照（同步执行，耗时约与数据量成正比），并注册为镜像写入目标
     * @param epoch 代数
     * @return 成功返回段名，失败返回空
     */
    std::string publish(uint64_t epoch);

    /** 注销镜像并删除当前持有的段 */
    void removeAll();

    /** 删除指定名称的段（容忍失败） */
    static void removeSegment(const std::string& name);

    /**
     * 镜像实时 K 线更新到共享内存段（未覆盖的证券/类型静默跳过）
     * @note 必须由全局镜像注册表持读锁时调用；写者需保证同一
     * 证券×ktype 串行（Stock::realtimeUpdate 写锁已保证）
     */
    void mirrorUpdate(const std::string& market_code, const KQuery::KType& ktype,
                      const KRecord& record);

private:
    /** 镜像写入索引项（指向已发布段内位置，随 publish/removeAll 整体替换） */
    struct MirrorEntry {
        ShmStockEntry* entry{nullptr};
        ShmKRecord* records{nullptr};
        bool overflow_warned{false};
    };

    std::string m_prefix;
    std::string m_current_name;

    // 以下镜像状态由全局注册表读写锁保护（publish 成功/removeAll 时持写锁替换）；
    // 注意：头文件中必须用全限定名，unity build 下 bi 别名可能与其它翻译单元冲突
    boost::interprocess::shared_memory_object m_shm;
    boost::interprocess::mapped_region m_region;
    std::unordered_map<std::string, MirrorEntry> m_mirror_index;  // key: market_code|KTYPE
};

/**
 * 全局镜像入口：Stock::realtimeUpdate 调用，将实时更新镜像到共享内存段
 * @note 未注册发布器（客户端进程/未启用缓存）时仅一次原子判断，开销可忽略
 */
void shmMirrorRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                             const KRecord& record);

class KDataShmReader;
typedef std::shared_ptr<KDataShmReader> KDataShmReaderPtr;

/**
 * 客户端端：只读映射共享内存 K 线缓存
 * @details 快照未覆盖的证券/类型返回 false，由调用方回退 IPC 查询；
 * 可经 coversKType 区分“主进程未预加载该类型”与“仅该证券/区间未收录”。
 * 查询语义与主进程缓冲模式一致（索引空间为缓冲自身，支持负索引与日期查询）；
 * 尾部实时区经 seqlock 保护读取，并发冲突重试超限后返回 false 回退 IPC。
 * @ingroup DataDriver
 */
class HKU_API KDataShmReader {
public:
    KDataShmReader();
    ~KDataShmReader();

    KDataShmReader(const KDataShmReader&) = delete;
    KDataShmReader& operator=(const KDataShmReader&) = delete;

    /** 打开并校验指定段，失败返回 false */
    bool open(const std::string& name);

    void close();

    bool valid() const noexcept {
        return m_impl != nullptr;
    }

    const std::string& name() const noexcept;

    uint64_t epoch() const noexcept;

    /** 快照覆盖的证券数量（所有 ktype 的 entry 总和），用于诊断 */
    size_t coveredCount() const noexcept;

    /**
     * 快照的 ktype 表是否包含指定类型（即主进程是否预加载了该类型）
     * @details 表中包含主进程已预加载的全部类型，即使某类型因懒加载尚未产生 entry。
     * 主进程仅为已预加载的类型注册行情处理函数（见 GlobalSpotAgent），因此未覆盖的
     * 类型在主进程侧不存在实时更新，客户端可直接走本地驱动而无需 IPC 往返。
     * @note 段未映射时返回 false，调用方不得据此判定“主进程未预加载”
     */
    bool coversKType(const KQuery::KType& ktype) const;

    bool tryGetCount(const std::string& market_code, const KQuery::KType& ktype,
                     size_t& out_count) const;

    bool tryGetIndexRangeByDate(const std::string& market_code, const KQuery& query,
                                size_t& out_start, size_t& out_end) const;

    bool tryGetKRecordList(const std::string& market_code, const KQuery& query,
                           KRecordList& out) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
