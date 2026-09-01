/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-01
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "hikyuu/KQuery.h"
#include "hikyuu/KRecord.h"

namespace hku {
namespace ipc {

constexpr uint32_t SHM_CACHE_MAGIC = 0x53434B48;  // "HKCS" 小端
constexpr uint32_t SHM_CACHE_VERSION = 1;

/*
 * K 线共享内存缓存段布局（全定长、无指针，发布后只读）：
 * [ShmCacheHeader][ShmKTypeInfo × ktype_count]
 * 每个 ktype：[ShmStockEntry × entry_count，按 market_code 升序，供二分查找]
 * 记录区：ShmKRecord × N
 * 段名含代数（epoch），每次发布创建新段并删除旧段，读端映射不受重写影响。
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

/** 证券索引项 */
struct ShmStockEntry {
    char market_code[16];    ///< NUL 填充，如 "SH600000"
    uint64_t record_offset;  ///< 记录区相对段首字节偏移
    uint64_t record_count;   ///< 记录数
};

/** ktype 索引表项 */
struct ShmKTypeInfo {
    char ktype[16];          ///< NUL 填充，如 "DAY"（需容纳 "HALFYEAR"/"TIMELINE" 等 8 字符类型）
    uint64_t entry_offset;   ///< entry 数组相对段首字节偏移
    uint32_t entry_count;
    uint32_t _pad;
};

/** 段头 */
struct ShmCacheHeader {
    uint32_t magic;
    uint32_t version;
    uint64_t epoch;          ///< 代数，每次发布唯一
    uint64_t data_size;      ///< 段实际数据字节数（发布完成后写入）
    uint32_t ktype_count;
    uint32_t _pad;
};

class KDataShmPublisher;
typedef std::shared_ptr<KDataShmPublisher> KDataShmPublisherPtr;

/**
 * 主进程端：将预加载的 K 线缓冲发布为只读共享内存快照
 * @details 快照仅包含已缓冲的基础 K 线类型（不含分时/分笔）。
 * 发布后的段不可变，实时新增数据由 IPC 查询兜底。
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
     * 构建并发布快照（同步执行，耗时约与数据量成正比）
     * @param epoch 代数
     * @return 成功返回段名，失败返回空
     */
    std::string publish(uint64_t epoch);

    /** 删除当前持有的段 */
    void removeAll();

    /** 删除指定名称的段（容忍失败） */
    static void removeSegment(const std::string& name);

private:
    std::string m_prefix;
    std::string m_current_name;
};

class KDataShmReader;
typedef std::shared_ptr<KDataShmReader> KDataShmReaderPtr;

/**
 * 客户端端：只读映射共享内存 K 线缓存
 * @details 快照未覆盖的证券/类型返回 false，由调用方回退 IPC 查询。
 * 查询语义与主进程缓冲模式一致（索引空间为缓冲自身，支持负索引与日期查询）。
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
