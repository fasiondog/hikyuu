/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-03
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

// 同 KDataShmCache.h：boost/interprocess 会传递引入 <sys/param.h>，其 ALIGN/MAX/MIN 宏
// 与 hikyuu 同名指标函数冲突；crt/*.h 中的 #undef 受 include guard 限制，在 unity build
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

#include "hikyuu/HistoryFinanceInfo.h"
#include "hikyuu/StockWeight.h"

namespace hku {
namespace ipc {

constexpr uint32_t SHM_BI_MAGIC = 0x53494248;  // "HBIS" 小端
constexpr uint32_t SHM_BI_VERSION = 1;

/** 共享内存表名（定长字段，需 < 16 字节） */
inline constexpr const char* const SHM_BI_TABLE_WEIGHT = "WEIGHT";   ///< 权息
inline constexpr const char* const SHM_BI_TABLE_FINANCE = "FINANCE";  ///< 历史财务

/*
 * 基础信息共享内存段布局（全定长、无指针）：
 * [ShmBiHeader][ShmBiTableInfo × table_count]
 * 每个表：[ShmBiEntry × entry_count，按 market_code 升序，供二分查找]
 * 记录区：WEIGHT 表为 ShmWeightRecord × n；FINANCE 表为定长 ShmFinanceRecord × n
 *
 * 与 KDataShmCache 的差异：权息与历史财务发布后即不可变，不存在实时镜像写入，
 * 因此**不需要** seqlock、尾部预留区与 fork 双写者防护；新数据到达时以新代数（epoch）
 * 重建整段替换，读者经 epoch 变化感知。
 */

/** 权息记录（与 StockWeight 字段一一对应） */
struct ShmWeightRecord {
    uint64_t datetime;
    double countAsGift;
    double countForSell;
    double priceForSell;
    double bonus;
    double increasement;
    double totalCount;
    double freeCount;
    double suogu;
};

/** 历史财务记录头，其后紧跟 value_count 个 float（段内按 8 字节对齐补齐） */
struct ShmFinanceRecord {
    uint64_t reportDate;
    uint64_t fileDate;
};

static_assert(sizeof(ShmWeightRecord) == 72, "ShmWeightRecord must be 72 bytes!");
static_assert(sizeof(ShmFinanceRecord) == 16, "ShmFinanceRecord head must be 16 bytes!");

/** 单条历史财务记录的段内步幅（8 字节对齐，保证后续记录起始处仍对齐） */
inline size_t financeRecordStride(uint32_t value_count) {
    size_t raw = sizeof(ShmFinanceRecord) + (size_t)value_count * sizeof(float);
    return (raw + 7) & ~(size_t)7;
}

/** 表索引项 */
struct ShmBiTableInfo {
    char name[16];        ///< NUL 填充，如 "FINANCE"
    uint64_t entry_offset;
    uint32_t entry_count;
    uint32_t value_count;  ///< FINANCE 每条记录的字段数；WEIGHT 为 0
};

/** 证券索引项 */
struct ShmBiEntry {
    char market_code[16];  ///< NUL 填充，如 "SH600000"
    uint64_t record_offset;
    uint64_t record_count;
    uint32_t value_count;  ///< 该证券每条记录的字段数（同表级，冗余便于校验）
    uint32_t _pad;
};  // 40B

static_assert(sizeof(ShmBiEntry) == 40, "ShmBiEntry must be 40 bytes!");

/** 段头 */
struct ShmBiHeader {
    std::atomic<uint32_t> magic;  ///< 完整性判据，数据全部写入后最后以 release 语义写入
    uint32_t version;
    uint64_t epoch;      ///< 代数，每次发布唯一
    uint64_t data_size;  ///< 段实际数据字节数
    uint32_t table_count;
    uint32_t _pad;
};

// magic 为原子量后仍须与 uint32_t 同宽同对齐，保证段布局字节兼容
static_assert(sizeof(ShmBiHeader) == 32 && sizeof(std::atomic<uint32_t>) == sizeof(uint32_t) &&
                alignof(std::atomic<uint32_t>) == alignof(uint32_t),
              "ShmBiHeader layout must stay compatible with plain uint32_t magic!");
static_assert(sizeof(ShmBiTableInfo) == 32, "ShmBiTableInfo must be 32 bytes!");

/** BaseInfoShmBuilder 输入：一条权息 entry（原始，不做长度/排序校验） */
struct BaseInfoShmBuildWeightEntry {
    std::string market_code;
    StockWeightList weights;
};

/** BaseInfoShmBuilder 输入：一条历史财务 entry（原始，不做长度/排序校验） */
struct BaseInfoShmBuildFinanceEntry {
    std::string market_code;
    std::vector<HistoryFinanceInfo> finances;
};

/** BaseInfoShmBuilder 输入：一张表（WEIGHT 用 weight_entries，FINANCE 用 finance_entries） */
struct BaseInfoShmBuildTable {
    std::string name;  ///< SHM_BI_TABLE_WEIGHT / SHM_BI_TABLE_FINANCE
    uint32_t value_count{0};
    std::vector<BaseInfoShmBuildWeightEntry> weight_entries;   ///< 顺序即写入顺序，本类不排序
    std::vector<BaseInfoShmBuildFinanceEntry> finance_entries;  ///< 同上
};

class BaseInfoShmBuilder;
typedef std::shared_ptr<BaseInfoShmBuilder> BaseInfoShmBuilderPtr;

/**
 * 段写入原语：把「表定义 + entry 列表 + 权息/财务记录」写成一个已创建并填充的共享内存段
 * @details 纯数据 → 字节：不依赖 StockManager、不做 epoch 编排、不加 market_code 长度与
 * entry 排序门控（以便单测构造边界 / 坏段）；段随本对象存活，析构或 reset 时删除。
 * 基础信息发布后不可变，无实时镜像，故不需 KDataShmBuilder 那样的镜像定位接口。
 * @ingroup DataDriver
 */
class HKU_API BaseInfoShmBuilder {
public:
    explicit BaseInfoShmBuilder(const std::string& shm_name_prefix);
    ~BaseInfoShmBuilder();

    BaseInfoShmBuilder(const BaseInfoShmBuilder&) = delete;
    BaseInfoShmBuilder& operator=(const BaseInfoShmBuilder&) = delete;

    /**
     * 创建并填充段；成功返回段名，失败返回空
     * @note 段名 = {prefix}_{epoch:016x}；不校验 market_code 长度 / entry 升序，按给定原样写入
     */
    std::string build(uint64_t epoch, const std::vector<BaseInfoShmBuildTable>& tables);

    /** 解除映射并删除当前段（容忍失败） */
    void reset();

    /** 删除指定名称的段（静态，容忍失败） */
    static void removeSegment(const std::string& name);

    const std::string& name() const noexcept {
        return m_current_name;
    }

private:
    std::string m_prefix;
    std::string m_current_name;
    boost::interprocess::shared_memory_object m_shm;
    boost::interprocess::mapped_region m_region;
};

class BaseInfoShmReader;
typedef std::shared_ptr<BaseInfoShmReader> BaseInfoShmReaderPtr;

/**
 * 客户端端：只读映射权息与历史财务快照
 * @details 未收录的证券或表返回 false，由调用方回退 IPC 查询；
 * 可经 coversTable 区分“主进程未加载该项”与“仅该证券无数据”，
 * 前者应直接走本地驱动，后者应回退 IPC。
 * @ingroup DataDriver
 */
class HKU_API BaseInfoShmReader {
public:
    BaseInfoShmReader();
    ~BaseInfoShmReader();

    BaseInfoShmReader(const BaseInfoShmReader&) = delete;
    BaseInfoShmReader& operator=(const BaseInfoShmReader&) = delete;

    /** 打开并校验指定段，失败返回 false */
    bool open(const std::string& name);

    void close();

    bool valid() const noexcept {
        return m_impl != nullptr;
    }

    const std::string& name() const noexcept;

    uint64_t epoch() const noexcept;

    /** 快照覆盖的证券条目总数（所有表累加），用于诊断 */
    size_t coveredCount() const noexcept;

    /**
     * 快照是否包含指定表（即主进程是否加载并发布了该项）
     * @note 段未映射时返回 false，调用方不得据此判定“主进程未加载”
     */
    bool coversTable(const std::string& table) const;

    /** FINANCE 表的字段数（用于校验客户端财务字段表是否与主进程一致），未收录返回 0 */
    size_t financeValueCount() const noexcept;

    /**
     * 取指定证券在 [start, end) 区间内的权息；未收录返回 false
     * @note 区间语义与 Stock::getWeight(start, end) 一致（按完整日期比较，含头不含尾）
     */
    bool tryGetWeightList(const std::string& market_code, const Datetime& start, const Datetime& end,
                          StockWeightList& out) const;

    /** 取全市场权息；未收录返回 false（调用方回退 IPC） */
    bool tryGetAllWeightList(std::unordered_map<std::string, StockWeightList>& out) const;

    /**
     * 取指定证券在 [start, end) 区间内的历史财务；未收录返回 false
     * @note 区间语义与 BaseInfoDriver::getHistoryFinance 对齐：按 reportDate 的 **ymd**
     *       比较（含头不含尾），Null 日期分别等价于 Datetime::min() / max()，
     *       且 start >= end 时返回空结果（仍算命中）
     */
    bool tryGetHistoryFinance(const std::string& market_code, const Datetime& start,
                              const Datetime& end, std::vector<HistoryFinanceInfo>& out) const;

    /** 取全市场历史财务；未收录返回 false */
    bool tryGetAllHistoryFinance(
      std::unordered_map<std::string, std::vector<HistoryFinanceInfo>>& out) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
