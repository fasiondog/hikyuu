/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-01
 *      Author: fasiondog
 */

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include "KDataShmCache.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include "hikyuu/utilities/Log.h"

// 取当前进程 id，用于识别 fork 出的子进程（Windows 无 fork，仅为统一代码路径）
#if defined(_WIN32)
#include <process.h>
#define HKU_SHM_GETPID _getpid
#else
#include <unistd.h>
#define HKU_SHM_GETPID getpid
#endif

namespace bi = boost::interprocess;

namespace hku {
namespace ipc {

namespace {

/** 定长字符串字段写入（截断并 NUL 填充） */
void putFixedString(char* dst, size_t cap, const std::string& src) {
    std::memset(dst, 0, cap);
    size_t n = std::min(cap - 1, src.size());
    std::memcpy(dst, src.data(), n);
}

/** 定长字符串字段读出 */
std::string getFixedString(const char* src, size_t cap) {
    size_t n = 0;
    while (n < cap && src[n] != '\0') {
        n++;
    }
    return std::string(src, n);
}

/** 段名记录文件：正常退出靠发布器析构删段，异常退出（崩溃/被杀）时
 *  残留段由新 Master 首次发布时依此清理（新 Master 已持文件锁，清理安全） */
std::string segmentRecordFilePath(const std::string& prefix) {
    return (std::filesystem::temp_directory_path() / (prefix + ".last")).string();
}

std::string readSegmentRecord(const std::string& prefix) {
    try {
        std::ifstream ifs(segmentRecordFilePath(prefix));
        std::string name;
        if (ifs.is_open() && std::getline(ifs, name)) {
            return name;
        }
    } catch (...) {
    }
    return "";
}

void writeSegmentRecord(const std::string& prefix, const std::string& name) {
    try {
        std::ofstream ofs(segmentRecordFilePath(prefix), std::ios::trunc);
        if (ofs.is_open()) {
            ofs << name;
        }
    } catch (...) {
    }
}

/** seqlock 读重试上限，超限回退 IPC（仅在镜像写入极频繁时发生） */
constexpr int SHM_READ_MAX_RETRY = 8;

}  // namespace

//----------------------------------------------------------------------------
// KDataShmBuilder
//----------------------------------------------------------------------------
KDataShmBuilder::KDataShmBuilder(const std::string& shm_name_prefix) : m_prefix(shm_name_prefix) {}

KDataShmBuilder::~KDataShmBuilder() {
    reset();
}

std::string KDataShmBuilder::build(uint64_t epoch, const std::vector<KDataShmBuildKType>& ktypes) {
    // 新段名（创建成功后赋值），异常时仅清理新段，保留旧段继续服务
    std::string created_name;
    try {
        // 首次构建前清理上一个异常退出残留的段（本对象已持旧段时不清理）
        if (m_current_name.empty()) {
            std::string orphan = readSegmentRecord(m_prefix);
            if (!orphan.empty()) {
                removeSegment(orphan);
            }
        }
        HKU_IF_RETURN(ktypes.empty(), "");

        // 统计记录与容量（预留区由入参逐 entry 给定，Builder 不推导）
        size_t total_records = 0, total_capacity = 0, entries_total = 0;
        for (const auto& kt : ktypes) {
            entries_total += kt.entries.size();
            for (const auto& e : kt.entries) {
                total_records += e.records.size();
                total_capacity += e.records.size() + e.reserved;
            }
        }
        HKU_IF_RETURN(total_records == 0, "");

        // 计算布局：[header][ktypes][entries][对齐填充][records]，与 KDataShmReader 校验规则一致；
        // 记录区按 8 字节对齐（KRecord 含 double），每证券含尾部预留区
        size_t header_size = sizeof(ShmCacheHeader);
        size_t ktype_table_size = ktypes.size() * sizeof(ShmKTypeInfo);
        size_t entries_size = entries_total * sizeof(ShmStockEntry);
        size_t record_base = header_size + ktype_table_size + entries_size;
        record_base = (record_base + 7) & ~(size_t)7;
        size_t records_size = total_capacity * sizeof(KRecord);
        size_t total_size = record_base + records_size;

        std::string name = fmt::format("{}_{:016x}", m_prefix, epoch);
        // 段名受系统限制（POSIX 一般不超过 31 字符），超长时放弃构建
        HKU_WARN_IF_RETURN(name.size() > 30, "", "Shm segment name too long ({}), skip build!",
                           name);
        removeSegment(name);

        // 新段创建成功后才视为接管；异常时仅清理新段，保留旧段继续服务
        bi::shared_memory_object shm(bi::create_only, name.c_str(), bi::read_write);
        created_name = name;
        shm.truncate(total_size);
        bi::mapped_region region(shm, bi::read_write);
        uint8_t* base = static_cast<uint8_t*>(region.get_address());
        std::memset(base, 0, total_size);

        ShmCacheHeader* header = reinterpret_cast<ShmCacheHeader*>(base);
        header->version = SHM_CACHE_VERSION;
        header->epoch = epoch;
        header->data_size = total_size;
        header->ktype_count = (uint32_t)ktypes.size();

        // 不做 market_code 长度 / entry 升序门控：按入参原样写入，供单测构造边界 / 坏段；
        // entry 为空的 ktype 同样记入表（保持 ktype 覆盖语义，与 Reader coversKType 对应）
        size_t entry_offset = header_size + ktype_table_size;
        size_t record_offset = record_base;
        std::unordered_map<std::string, MirrorTarget> new_targets;
        new_targets.reserve(entries_total);
        for (size_t i = 0; i < ktypes.size(); i++) {
            ShmKTypeInfo* info =
              reinterpret_cast<ShmKTypeInfo*>(base + header_size + i * sizeof(ShmKTypeInfo));
            putFixedString(info->ktype, sizeof(info->ktype), ktypes[i].ktype);
            info->entry_offset = entry_offset;
            info->entry_count = (uint32_t)ktypes[i].entries.size();

            std::string up_ktype(ktypes[i].ktype);
            to_upper(up_ktype);
            for (const auto& e : ktypes[i].entries) {
                ShmStockEntry* se = reinterpret_cast<ShmStockEntry*>(base + entry_offset);
                putFixedString(se->market_code, sizeof(se->market_code), e.market_code);
                se->record_offset = record_offset;
                se->record_capacity = e.records.size() + e.reserved;
                se->record_count.store(e.records.size(), std::memory_order_relaxed);
                se->seq.store(0, std::memory_order_relaxed);
                entry_offset += sizeof(ShmStockEntry);

                KRecord* rec = reinterpret_cast<KRecord*>(base + record_offset);
                // KRecord 为标准布局、无虚表与资源所有权（见头文件 static_assert），可安全按字节 memcpy；
                // 因含 Datetime（非平凡可拷贝），显式转 void* 消除 -Wnontrivial-memcall
                std::memcpy(static_cast<void*>(rec), e.records.data(),
                            e.records.size() * sizeof(KRecord));

                new_targets.emplace(e.market_code + "|" + up_ktype, MirrorTarget{se, rec});
                record_offset += se->record_capacity * sizeof(KRecord);
            }
        }

        // 数据全部写入后再落 magic，读端以 magic 校验段完整性（release store 与读端 acquire 配对）
        header->magic.store(SHM_CACHE_MAGIC, std::memory_order_release);
        region.flush();

        // 接管：解除旧段映射并删除旧段（旧段读者不受影响）
        std::string old_name = m_current_name;
        m_shm.swap(shm);
        m_region.swap(region);
        m_targets.swap(new_targets);
        m_current_name = name;
        writeSegmentRecord(m_prefix, name);
        if (!old_name.empty() && old_name != name) {
            removeSegment(old_name);
        }
        HKU_INFO("Built kdata shm segment: {} ({} ktypes, {} entries, {} records, capacity {})",
                 name, ktypes.size(), entries_total, total_records, total_capacity);
        return name;
    } catch (const std::exception& e) {
        HKU_ERROR("Failed build kdata shm segment: {}", e.what());
        if (!created_name.empty() && created_name != m_current_name) {
            removeSegment(created_name);
        }
        return "";
    } catch (...) {
        HKU_ERROR("Failed build kdata shm segment: unknown error!");
        if (!created_name.empty() && created_name != m_current_name) {
            removeSegment(created_name);
        }
        return "";
    }
}

bool KDataShmBuilder::getMirrorTarget(const std::string& market_code, const KQuery::KType& ktype,
                                      ShmStockEntry*& entry, KRecord*& records) {
    if (m_targets.empty()) {
        return false;
    }
    std::string up_ktype(ktype);
    to_upper(up_ktype);
    auto it = m_targets.find(market_code + "|" + up_ktype);
    if (it == m_targets.end()) {
        return false;
    }
    entry = it->second.entry;
    records = it->second.records;
    return true;
}

void KDataShmBuilder::reset() {
    bi::mapped_region().swap(m_region);
    bi::shared_memory_object().swap(m_shm);
    m_targets.clear();
    if (!m_current_name.empty()) {
        removeSegment(m_current_name);
        m_current_name.clear();
    }
}

void KDataShmBuilder::removeSegment(const std::string& name) {
    try {
        bi::shared_memory_object::remove(name.c_str());
    } catch (...) {
    }
}

//----------------------------------------------------------------------------
// KDataShmReader
//----------------------------------------------------------------------------
struct KDataShmReader::Impl {
    std::string name;
    uint64_t epoch{0};
    size_t covered{0};
    bi::shared_memory_object shm;
    bi::mapped_region region;
    const uint8_t* base{nullptr};
    size_t size{0};

    struct KTypeIndex {
        std::string ktype;
        const ShmStockEntry* entries{nullptr};
        uint32_t entry_count{0};
    };
    std::vector<KTypeIndex> ktypes;

    /** 二分查找证券索引项，未找到返回 nullptr */
    static const ShmStockEntry* findEntry(const KTypeIndex& idx, const std::string& key) {
        if (key.size() >= sizeof(ShmStockEntry::market_code)) {
            return nullptr;
        }
        char buf[sizeof(ShmStockEntry::market_code)];
        putFixedString(buf, sizeof(buf), key);
        auto cmp = [](const ShmStockEntry& entry, const char* k) {
            return std::memcmp(entry.market_code, k, sizeof(ShmStockEntry::market_code)) < 0;
        };
        const ShmStockEntry* first = idx.entries;
        const ShmStockEntry* it = std::lower_bound(first, first + idx.entry_count, buf, cmp);
        if (it != first + idx.entry_count &&
            std::memcmp(it->market_code, buf, sizeof(ShmStockEntry::market_code)) == 0) {
            return it;
        }
        return nullptr;
    }

    const KTypeIndex* findKType(const KQuery::KType& ktype) const {
        for (const auto& k : ktypes) {
            if (k.ktype == ktype) {
                return &k;
            }
        }
        return nullptr;
    }

    /** 校验记录区边界，防止段损坏导致越界读取（8 字节对齐保证 double 字段对齐）；
     *  边界按容量（含预留区）计算，镜像追加不会超出此范围 */
    bool checkRecords(const ShmStockEntry* entry) const {
        return entry->record_offset % 8 == 0 && entry->record_capacity > 0 &&
               entry->record_offset + entry->record_capacity * sizeof(KRecord) <= size;
    }

    /** 读取当前有效记录数（clamp 到容量，防段异常导致越界） */
    static size_t readCount(const ShmStockEntry* entry) {
        uint64_t c = entry->record_count.load(std::memory_order_relaxed);
        return c > entry->record_capacity ? (size_t)-1 : (size_t)c;
    }

    /**
     * seqlock 会话内计算日期区间（纯计算，不触原子量，与主进程
     * _getIndexRangeByDateFromBuffer 语义一致）：先二分查找 >= startDatetime
     * 的首条，再查找 >= endDatetime 的首条；区间为空返回 false
     */
    static bool dateRange(const KRecord* recs, size_t total, const KQuery& query, size_t& out_start,
                          size_t& out_end) {
        if (total == 0) {
            return false;
        }
        size_t mid = total, low = 0, high = total - 1;
        while (low <= high) {
            if (query.startDatetime() > recs[high].datetime) {
                mid = high + 1;
                break;
            }
            if (recs[low].datetime >= query.startDatetime()) {
                mid = low;
                break;
            }
            mid = (low + high) / 2;
            if (query.startDatetime() > recs[mid].datetime) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        HKU_IF_RETURN(mid >= total, false);

        size_t startpos = mid;
        low = mid;
        high = total - 1;
        while (low <= high) {
            if (query.endDatetime() > recs[high].datetime) {
                mid = high + 1;
                break;
            }
            if (recs[low].datetime >= query.endDatetime()) {
                mid = low;
                break;
            }
            mid = (low + high) / 2;
            if (query.endDatetime() > recs[mid].datetime) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        size_t endpos = (mid >= total) ? total : mid;
        HKU_IF_RETURN(startpos >= endpos, false);
        out_start = startpos;
        out_end = endpos;
        return true;
    }
};

KDataShmReader::KDataShmReader() = default;
KDataShmReader::~KDataShmReader() = default;

bool KDataShmReader::open(const std::string& name) {
    close();
    try {
        auto impl = std::make_unique<Impl>();
        bi::shared_memory_object shm(bi::open_only, name.c_str(), bi::read_only);
        bi::mapped_region region(shm, bi::read_only);
        HKU_IF_RETURN(region.get_size() < sizeof(ShmCacheHeader), false);

        const uint8_t* base = static_cast<const uint8_t*>(region.get_address());
        const ShmCacheHeader* header = reinterpret_cast<const ShmCacheHeader*>(base);
        HKU_IF_RETURN(header->magic.load(std::memory_order_acquire) != SHM_CACHE_MAGIC, false);
        HKU_IF_RETURN(header->version != SHM_CACHE_VERSION, false);
        HKU_IF_RETURN(header->data_size == 0 || header->data_size > region.get_size(), false);

        impl->epoch = header->epoch;
        impl->base = base;
        impl->size = header->data_size;

        for (uint32_t i = 0; i < header->ktype_count; i++) {
            size_t info_off = sizeof(ShmCacheHeader) + i * sizeof(ShmKTypeInfo);
            HKU_IF_RETURN(info_off + sizeof(ShmKTypeInfo) > impl->size, false);
            const ShmKTypeInfo* info = reinterpret_cast<const ShmKTypeInfo*>(base + info_off);
            size_t entries_end =
              info->entry_offset + (size_t)info->entry_count * sizeof(ShmStockEntry);
            HKU_IF_RETURN(info->entry_offset % 8 != 0 ||
                            info->entry_offset < sizeof(ShmCacheHeader) || entries_end > impl->size,
                          false);

            Impl::KTypeIndex idx;
            idx.ktype = getFixedString(info->ktype, sizeof(info->ktype));
            to_upper(idx.ktype);
            idx.entries = reinterpret_cast<const ShmStockEntry*>(base + info->entry_offset);
            idx.entry_count = info->entry_count;

            // 校验发布端已按 market_code 升序排列（二分查找前提）
            bool sorted = true;
            for (uint32_t j = 1; j < idx.entry_count; j++) {
                if (std::memcmp(idx.entries[j - 1].market_code, idx.entries[j].market_code,
                                sizeof(ShmStockEntry::market_code)) >= 0) {
                    sorted = false;
                    break;
                }
            }
            HKU_IF_RETURN(!sorted, false);

            impl->covered += idx.entry_count;
            impl->ktypes.emplace_back(std::move(idx));
        }

        impl->name = name;
        impl->shm = std::move(shm);
        impl->region = std::move(region);
        m_impl = std::move(impl);
        return true;
    } catch (const std::exception& e) {
        HKU_WARN("Failed open kdata shm cache {}: {}", name, e.what());
    } catch (...) {
        HKU_WARN("Failed open kdata shm cache: {}!", name);
    }
    return false;
}

void KDataShmReader::close() {
    m_impl.reset();
}

const std::string& KDataShmReader::name() const noexcept {
    static const std::string empty;
    return m_impl ? m_impl->name : empty;
}

uint64_t KDataShmReader::epoch() const noexcept {
    return m_impl ? m_impl->epoch : 0;
}

size_t KDataShmReader::coveredCount() const noexcept {
    return m_impl ? m_impl->covered : 0;
}

bool KDataShmReader::coversKType(const KQuery::KType& ktype) const {
    HKU_IF_RETURN(!m_impl, false);
    std::string upper_ktype(ktype);
    to_upper(upper_ktype);
    return m_impl->findKType(upper_ktype) != nullptr;
}

bool KDataShmReader::tryGetCount(const std::string& market_code, const KQuery::KType& ktype,
                                 size_t& out_count) const {
    HKU_IF_RETURN(!m_impl, false);
    std::string upper_ktype(ktype);
    to_upper(upper_ktype);
    const Impl::KTypeIndex* idx = m_impl->findKType(upper_ktype);
    HKU_IF_RETURN(!idx, false);
    const ShmStockEntry* entry = Impl::findEntry(*idx, market_code);
    HKU_IF_RETURN(!entry, false);
    // 原子 count 随镜像追加实时增长，clamp 到容量防段异常
    size_t count = Impl::readCount(entry);
    HKU_IF_RETURN(count == 0 || count == (size_t)-1, false);
    out_count = count;
    return true;
}

bool KDataShmReader::tryGetIndexRangeByDate(const std::string& market_code, const KQuery& query,
                                            size_t& out_start, size_t& out_end) const {
    out_start = 0;
    out_end = 0;
    HKU_IF_RETURN(!m_impl || query.queryType() != KQuery::DATE, false);
    std::string upper_ktype(query.kType());
    to_upper(upper_ktype);
    const Impl::KTypeIndex* idx = m_impl->findKType(upper_ktype);
    HKU_IF_RETURN(!idx, false);
    const ShmStockEntry* entry = Impl::findEntry(*idx, market_code);
    HKU_IF_RETURN(!entry, false);
    HKU_IF_RETURN(!m_impl->checkRecords(entry), false);

    const KRecord* recs = reinterpret_cast<const KRecord*>(m_impl->base + entry->record_offset);

    // seqlock 读：尾部正被镜像写入时获得一致快照，重试超限回退 IPC
    for (int attempt = 0; attempt < SHM_READ_MAX_RETRY; attempt++) {
        uint32_t s1 = entry->seq.load(std::memory_order_acquire);
        if (s1 & 1) {
            std::this_thread::yield();
            continue;
        }
        size_t total = Impl::readCount(entry);
        HKU_IF_RETURN(total == (size_t)-1, false);

        size_t startpos = 0, endpos = 0;
        bool ok = Impl::dateRange(recs, total, query, startpos, endpos);

        std::atomic_thread_fence(std::memory_order_acquire);
        if (entry->seq.load(std::memory_order_relaxed) == s1) {
            HKU_IF_RETURN(!ok, false);  // 覆盖但区间为空，与服务端缓冲模式一致
            out_start = startpos;
            out_end = endpos;
            return true;
        }
    }
    return false;
}

bool KDataShmReader::tryGetKRecordList(const std::string& market_code, const KQuery& query,
                                       KRecordList& out) const {
    out.clear();
    HKU_IF_RETURN(!m_impl, false);
    std::string upper_ktype(query.kType());
    to_upper(upper_ktype);
    const Impl::KTypeIndex* idx = m_impl->findKType(upper_ktype);
    HKU_IF_RETURN(!idx, false);
    const ShmStockEntry* entry = Impl::findEntry(*idx, market_code);
    HKU_IF_RETURN(!entry, false);
    HKU_IF_RETURN(!m_impl->checkRecords(entry), false);

    bool is_date = query.queryType() == KQuery::DATE;
    if (!is_date) {
        // 负索引已由上层（Stock）解析，此处仅处理正索引并做边界钳制
        HKU_IF_RETURN(query.start() < 0 || query.end() < 0, false);
    }

    const KRecord* recs = reinterpret_cast<const KRecord*>(m_impl->base + entry->record_offset);

    // seqlock 读：区间计算与记录拷贝在同一会话内完成，保证结果一致；
    // 尾部正被镜像写入时重试，超限回退 IPC
    for (int attempt = 0; attempt < SHM_READ_MAX_RETRY; attempt++) {
        uint32_t s1 = entry->seq.load(std::memory_order_acquire);
        if (s1 & 1) {
            std::this_thread::yield();
            continue;
        }
        size_t total = Impl::readCount(entry);
        HKU_IF_RETURN(total == (size_t)-1, false);
        HKU_IF_RETURN(total == 0, false);

        size_t start_ix = 0, end_ix = 0;
        if (is_date) {
            if (!Impl::dateRange(recs, total, query, start_ix, end_ix)) {
                // 快照覆盖但区间为空，与服务端缓冲模式行为一致（out 保持空）
                std::atomic_thread_fence(std::memory_order_acquire);
                if (entry->seq.load(std::memory_order_relaxed) == s1) {
                    return true;
                }
                continue;
            }
        } else {
            int64_t startix = query.start();
            int64_t endix = query.end();
            start_ix = startix >= (int64_t)total ? total : (size_t)startix;
            end_ix = endix > (int64_t)total ? total : (size_t)endix;
            if (start_ix >= end_ix) {
                std::atomic_thread_fence(std::memory_order_acquire);
                if (entry->seq.load(std::memory_order_relaxed) == s1) {
                    return true;
                }
                continue;
            }
        }

        out.resize(end_ix - start_ix);
        // 记录区以 KRecord 二进制存放，整段 memcpy 拷出（较逐条 Datetime 构造更快）；
        // 段完整性由 magic/version 校验与 entry 级 seqlock 保证，无需逐条异常兜底。
        // out.data() 为 KRecord*（含 Datetime 致非平凡可拷贝），显式转 void* 消除 -Wnontrivial-memcall
        std::memcpy(static_cast<void*>(out.data()), recs + start_ix,
                    (end_ix - start_ix) * sizeof(KRecord));
        std::atomic_thread_fence(std::memory_order_acquire);
        if (entry->seq.load(std::memory_order_relaxed) == s1) {
            return true;
        }
        out.clear();
    }
    return false;
}

bool KDataShmReader::tryGetKRecordView(const std::string& market_code, const KQuery::KType& ktype,
                                       size_t start_ix, size_t end_ix, const KRecord*& out_data,
                                       size_t& out_count) const {
    out_data = nullptr;
    out_count = 0;
    HKU_IF_RETURN(!m_impl, false);
    std::string upper_ktype(ktype);
    to_upper(upper_ktype);
    const Impl::KTypeIndex* idx = m_impl->findKType(upper_ktype);
    HKU_IF_RETURN(!idx, false);
    const ShmStockEntry* entry = Impl::findEntry(*idx, market_code);
    HKU_IF_RETURN(!entry, false);
    HKU_IF_RETURN(!m_impl->checkRecords(entry), false);
    HKU_IF_RETURN(start_ix >= end_ix, false);

    size_t total = Impl::readCount(entry);
    HKU_IF_RETURN(total == 0 || total == (size_t)-1, false);
    HKU_IF_RETURN(start_ix >= total, false);
    if (end_ix > total) {
        end_ix = total;
    }
    HKU_IF_RETURN(start_ix >= end_ix, false);

    // 裸指针视图：不拷贝，调用方须持 reader 的 shared_ptr pin 住映射。历史区发布后不可变，
    // 仅尾部实时区由主进程 seqlock 写入，视图读取存在与主进程 KDataSharedBufferImp 同类的
    // 良性瞬时竞争（进行中末根）。
    out_data = reinterpret_cast<const KRecord*>(m_impl->base + entry->record_offset) + start_ix;
    out_count = end_ix - start_ix;
    return true;
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
