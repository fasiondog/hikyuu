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
#include <unordered_map>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include "hikyuu/Stock.h"
#include "hikyuu/StockManager.h"
#include "hikyuu/utilities/Log.h"

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

/** 将 K 线记录写入共享内存记录区 */
void writeShmKRecord(ShmKRecord* dst, const KRecord& k) {
    dst->datetime = k.datetime.number();
    dst->openPrice = k.openPrice;
    dst->highPrice = k.highPrice;
    dst->lowPrice = k.lowPrice;
    dst->closePrice = k.closePrice;
    dst->transAmount = k.transAmount;
    dst->transCount = k.transCount;
}

/** 从共享内存记录区还原 K 线记录 */
KRecord readShmKRecord(const ShmKRecord& src) {
    KRecord k;
    k.datetime = Datetime(src.datetime);
    k.openPrice = src.openPrice;
    k.highPrice = src.highPrice;
    k.lowPrice = src.lowPrice;
    k.closePrice = src.closePrice;
    k.transAmount = src.transAmount;
    k.transCount = src.transCount;
    return k;
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

}  // namespace

///////////////////////////////////////////////////////////////////////////////
// KDataShmPublisher
///////////////////////////////////////////////////////////////////////////////
KDataShmPublisher::KDataShmPublisher(const std::string& shm_name_prefix)
: m_prefix(shm_name_prefix) {}

KDataShmPublisher::~KDataShmPublisher() {
    removeAll();
}

std::string KDataShmPublisher::publish(uint64_t epoch) {
    // 新段名（创建成功后赋值），异常时仅清理新段，保留旧段继续服务
    std::string created_name;
    try {
        auto& sm = StockManager::instance();
        const auto& preload_param = sm.getPreloadParameter();

        // 首次发布前清理上一个异常退出的 Master 残留段（仅此时执行：
        // 本进程已持文件锁，是唯一 Master；已映射旧段的读者不受 remove 影响）
        if (m_current_name.empty()) {
            std::string orphan = readSegmentRecord(m_prefix);
            if (!orphan.empty()) {
                removeSegment(orphan);
            }
        }

        // 收集已预加载的 K 线类型（超长名无法存入定长字段，防御性跳过）
        std::vector<KQuery::KType> ktypes;
        for (const auto& ktype : KQuery::getBaseKTypeList()) {
            if (ktype.size() >= sizeof(ShmKTypeInfo::ktype)) {
                HKU_WARN("Ktype {} too long for shm cache, skip publish it!", ktype);
                continue;
            }
            std::string low_ktype = ktype;
            to_lower(low_ktype);
            if (preload_param.tryGet<bool>(low_ktype, false)) {
                ktypes.push_back(ktype);
            }
        }
        HKU_IF_RETURN(ktypes.empty(), "");

        auto stocks = sm.getStockList(nullptr);
        HKU_IF_RETURN(stocks.empty(), "");

        struct KTypeData {
            KQuery::KType ktype;
            std::vector<std::pair<std::string, KRecordList>> entries;  // 按 market_code 升序
        };
        std::vector<KTypeData> ktype_datas;
        ktype_datas.reserve(ktypes.size());

        size_t total_records = 0;
        for (const auto& ktype : ktypes) {
            // 预加载数量上限；缓冲条数达到上限时可能发生了截断，
            // 此类证券不发布，由客户端回退 IPC 查询，保证结果与全量数据一致
            std::string preload_key = ktype;
            to_lower(preload_key);
            preload_key += "_max";
            int64_t max_num = preload_param.tryGet<int64_t>(
              preload_key, std::numeric_limits<int64_t>::max());

            KTypeData kd;
            kd.ktype = ktype;
            for (const Stock& stk : stocks) {
                size_t buf_count = stk.getKDataBufferSize(ktype);
                if (buf_count == 0 || (int64_t)buf_count >= max_num) {
                    continue;
                }
                KRecordList ks = stk.getKRecordListFromBuffer(ktype);
                if (ks.size() != buf_count) {
                    continue;
                }
                total_records += ks.size();
                kd.entries.emplace_back(stk.market_code(), std::move(ks));
            }
            if (!kd.entries.empty()) {
                std::sort(kd.entries.begin(), kd.entries.end(),
                          [](const auto& a, const auto& b) { return a.first < b.first; });
                ktype_datas.emplace_back(std::move(kd));
            }
        }
        HKU_IF_RETURN(ktype_datas.empty() || total_records == 0, "");

        // 计算布局：[header][ktypes][entries][对齐填充][records]，全部相对段首字节偏移；
        // 记录区按 8 字节对齐（ShmKRecord 含 double）
        size_t header_size = sizeof(ShmCacheHeader);
        size_t ktype_table_size = ktype_datas.size() * sizeof(ShmKTypeInfo);
        size_t entries_total = 0;
        for (const auto& kd : ktype_datas) {
            entries_total += kd.entries.size();
        }
        size_t entries_size = entries_total * sizeof(ShmStockEntry);
        size_t record_base = header_size + ktype_table_size + entries_size;
        record_base = (record_base + 7) & ~(size_t)7;
        size_t records_size = total_records * sizeof(ShmKRecord);
        size_t total_size = record_base + records_size;

        std::string name = fmt::format("{}_{:016x}", m_prefix, epoch);
        // 段名受系统限制（POSIX 一般不超过 31 字符），超长时放弃发布
        HKU_WARN_IF_RETURN(name.size() > 30, "",
                           "Shm segment name too long ({}), skip publish!", name);
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
        header->ktype_count = (uint32_t)ktype_datas.size();

        size_t entry_offset = header_size + ktype_table_size;
        size_t record_offset = record_base;
        for (size_t i = 0; i < ktype_datas.size(); i++) {
            ShmKTypeInfo* info =
              reinterpret_cast<ShmKTypeInfo*>(base + header_size + i * sizeof(ShmKTypeInfo));
            putFixedString(info->ktype, sizeof(info->ktype), ktype_datas[i].ktype);
            info->entry_offset = entry_offset;
            info->entry_count = (uint32_t)ktype_datas[i].entries.size();

            for (const auto& entry : ktype_datas[i].entries) {
                ShmStockEntry* se = reinterpret_cast<ShmStockEntry*>(base + entry_offset);
                putFixedString(se->market_code, sizeof(se->market_code), entry.first);
                se->record_offset = record_offset;
                se->record_count = entry.second.size();
                entry_offset += sizeof(ShmStockEntry);

                ShmKRecord* rec = reinterpret_cast<ShmKRecord*>(base + record_offset);
                for (size_t r = 0; r < entry.second.size(); r++) {
                    writeShmKRecord(rec + r, entry.second[r]);
                }
                record_offset += entry.second.size() * sizeof(ShmKRecord);
            }
        }

        // 数据全部写入后再落 magic，读端以 magic 校验段完整性；
        // 新段就绪后才删除旧段，保证已映射旧段的读者不受影响（快照语义）
        std::atomic_thread_fence(std::memory_order_release);
        header->magic = SHM_CACHE_MAGIC;
        region.flush();

        std::string old_name = m_current_name;
        m_current_name = name;
        writeSegmentRecord(m_prefix, name);
        if (!old_name.empty() && old_name != name) {
            removeSegment(old_name);
        }
        HKU_INFO("Published kdata shm cache: {} ({} ktypes, {} stocks entries, {} records, "
                 "{:.2f} MB)",
                 name, ktype_datas.size(), entries_total, total_records, total_size / 1048576.0);
        return name;
    } catch (const std::exception& e) {
        HKU_ERROR("Failed publish kdata shm cache: {}", e.what());
        if (!created_name.empty() && created_name != m_current_name) {
            removeSegment(created_name);
        }
        return "";
    } catch (...) {
        HKU_ERROR("Failed publish kdata shm cache: unknown error!");
        if (!created_name.empty() && created_name != m_current_name) {
            removeSegment(created_name);
        }
        return "";
    }
}

void KDataShmPublisher::removeAll() {
    if (!m_current_name.empty()) {
        removeSegment(m_current_name);
        m_current_name.clear();
    }
}

void KDataShmPublisher::removeSegment(const std::string& name) {
    try {
        bi::shared_memory_object::remove(name.c_str());
    } catch (...) {
    }
}

///////////////////////////////////////////////////////////////////////////////
// KDataShmReader
///////////////////////////////////////////////////////////////////////////////
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

    /** 校验记录区边界，防止段损坏导致越界读取（8 字节对齐保证 double 字段对齐） */
    bool checkRecords(const ShmStockEntry* entry) const {
        return entry->record_offset % 8 == 0 &&
               entry->record_offset + entry->record_count * sizeof(ShmKRecord) <= size;
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
        HKU_IF_RETURN(header->magic != SHM_CACHE_MAGIC, false);
        HKU_IF_RETURN(header->version != SHM_CACHE_VERSION, false);
        HKU_IF_RETURN(header->data_size == 0 || header->data_size > region.get_size(), false);

        impl->epoch = header->epoch;
        impl->base = base;
        impl->size = header->data_size;

        for (uint32_t i = 0; i < header->ktype_count; i++) {
            size_t info_off = sizeof(ShmCacheHeader) + i * sizeof(ShmKTypeInfo);
            HKU_IF_RETURN(info_off + sizeof(ShmKTypeInfo) > impl->size, false);
            const ShmKTypeInfo* info = reinterpret_cast<const ShmKTypeInfo*>(base + info_off);
            size_t entries_end = info->entry_offset + (size_t)info->entry_count *
                                                       sizeof(ShmStockEntry);
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

bool KDataShmReader::tryGetCount(const std::string& market_code, const KQuery::KType& ktype,
                                 size_t& out_count) const {
    HKU_IF_RETURN(!m_impl, false);
    std::string upper_ktype(ktype);
    to_upper(upper_ktype);
    const Impl::KTypeIndex* idx = m_impl->findKType(upper_ktype);
    HKU_IF_RETURN(!idx, false);
    const ShmStockEntry* entry = Impl::findEntry(*idx, market_code);
    HKU_IF_RETURN(!entry, false);
    out_count = entry->record_count;
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
    HKU_IF_RETURN(!entry || entry->record_count == 0, false);
    HKU_IF_RETURN(!m_impl->checkRecords(entry), false);

    const ShmKRecord* recs =
      reinterpret_cast<const ShmKRecord*>(m_impl->base + entry->record_offset);
    size_t total = entry->record_count;

    // 与主进程缓冲模式（_getIndexRangeByDateFromBuffer）语义一致：
    // 先二分查找 >= startDatetime 的首条，再查找 >= endDatetime 的首条
    size_t mid = total, low = 0, high = total - 1;
    while (low <= high) {
        if (query.startDatetime().number() > recs[high].datetime) {
            mid = high + 1;
            break;
        }
        if (recs[low].datetime >= query.startDatetime().number()) {
            mid = low;
            break;
        }
        mid = (low + high) / 2;
        if (query.startDatetime().number() > recs[mid].datetime) {
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
        if (query.endDatetime().number() > recs[high].datetime) {
            mid = high + 1;
            break;
        }
        if (recs[low].datetime >= query.endDatetime().number()) {
            mid = low;
            break;
        }
        mid = (low + high) / 2;
        if (query.endDatetime().number() > recs[mid].datetime) {
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

bool KDataShmReader::tryGetKRecordList(const std::string& market_code, const KQuery& query,
                                       KRecordList& out) const {
    out.clear();
    HKU_IF_RETURN(!m_impl, false);
    std::string upper_ktype(query.kType());
    to_upper(upper_ktype);
    const Impl::KTypeIndex* idx = m_impl->findKType(upper_ktype);
    HKU_IF_RETURN(!idx, false);
    const ShmStockEntry* entry = Impl::findEntry(*idx, market_code);
    HKU_IF_RETURN(!entry || entry->record_count == 0, false);
    HKU_IF_RETURN(!m_impl->checkRecords(entry), false);

    size_t total = entry->record_count;
    size_t start_ix = 0, end_ix = 0;
    if (query.queryType() == KQuery::DATE) {
        if (!tryGetIndexRangeByDate(market_code, query, start_ix, end_ix)) {
            return true;  // 快照覆盖但区间为空，与服务端缓冲模式行为一致
        }
    } else {
        // 负索引已由上层（Stock）解析，此处仅处理正索引并做边界钳制
        int64_t startix = query.start();
        int64_t endix = query.end();
        HKU_IF_RETURN(startix < 0 || endix < 0, false);
        start_ix = startix >= (int64_t)total ? total : (size_t)startix;
        end_ix = endix > (int64_t)total ? total : (size_t)endix;
        HKU_IF_RETURN(start_ix >= end_ix, true);
    }

    const ShmKRecord* recs =
      reinterpret_cast<const ShmKRecord*>(m_impl->base + entry->record_offset);
    out.resize(end_ix - start_ix);
    for (size_t i = start_ix; i < end_ix; i++) {
        out[i - start_ix] = readShmKRecord(recs[i]);
    }
    return true;
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
