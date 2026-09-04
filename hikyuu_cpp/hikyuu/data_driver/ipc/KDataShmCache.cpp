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
#include "hikyuu/Stock.h"
#include "hikyuu/StockManager.h"
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

/** 每证券预留容量（1 个交易日）：分钟级按 A 股 240 分钟折算，日线及以上预留 2 条；
 *  预留区写满后停止镜像追加（现有部署每日定时重启，重新发布即恢复） */
size_t reservedForKType(const KQuery::KType& ktype) {
    // 分笔不在 g_ktype2min 中（由 g_ktype2sec 承载），getKTypeInMin 会打 WARN 并返回 0；
    // 行情代理也未为分笔注册处理函数（见 GlobalSpotAgent），不存在实时镜像写入，
    // 且单日分笔可达数千条，预留整日容量代价过高，故按最小值预留
    if (ktype == KQuery::TRANS) {
        return 2;
    }
    int32_t kmin = KQuery::getKTypeInMin(ktype);
    if (kmin > 0 && kmin <= 240) {
        return (size_t)(240 / kmin) + 1;
    }
    return 2;
}

/** seqlock 读重试上限，超限回退 IPC（仅在镜像写入极频繁时发生） */
constexpr int SHM_READ_MAX_RETRY = 8;

/**
 * 全局镜像注册表：读锁保护镜像写入，写锁保护发布/注销时的状态替换
 * @note 刻意堆分配且永不释放：注销可能发生在静态析构期（GlobalInitializer::clean 由最后
 * 一个 s_global_initializer 析构经 __cxa_finalize 触发），彼时本翻译单元的静态对象已被销毁，
 * 对已销毁的锁加锁会得到 EINVAL 并抛出 system_error；而调用方位于析构函数（隐式 noexcept）
 * 中，异常将直接 terminate。此处一把锁的泄露相对进程退出无实质影响。
 */
std::shared_mutex& g_mirror_mutex = *(new std::shared_mutex);
KDataShmPublisher* g_mirror_pub{nullptr};
std::atomic<bool> g_mirror_active{false};

/**
 * 发布快照的进程 id
 * @details Linux 默认 fork 的子进程会继承段映射与 g_mirror_active，若子进程也触发
 * Stock::realtimeUpdate，同一 entry 将出现第二个写者，破坏 seqlock 的单写者前提
 * 并导致读端撕裂。子进程 pid 与发布时不同，据此停用其镜像写入。
 */
std::atomic<int64_t> g_mirror_pid{0};

}  // namespace

void shmMirrorRealtimeUpdate(const std::string& market_code, const KQuery::KType& ktype,
                             const KRecord& record) {
    // 快速路径：未注册发布器（客户端进程/未启用缓存/尚未发布）时直接返回
    if (!g_mirror_active.load(std::memory_order_acquire)) {
        return;
    }
    // fork 出的子进程不是发布段的那个进程，写入会成为双写者；
    // 此处仅发布进程会执行到，getpid 的 syscall 开销相对行情更新频率可忽略
    if (g_mirror_pid.load(std::memory_order_relaxed) != (int64_t)HKU_SHM_GETPID()) {
        return;
    }
    std::shared_lock<std::shared_mutex> lock(g_mirror_mutex);
    if (g_mirror_pub) {
        g_mirror_pub->mirrorUpdate(market_code, ktype, record);
    }
}

//----------------------------------------------------------------------------
// KDataShmPublisher
//----------------------------------------------------------------------------
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
        size_t total_capacity = 0;
        bool warned_long_code = false;
        for (const auto& ktype : ktypes) {
            // 预加载数量上限（缓冲条数达到上限时可能发生了截断）
            std::string preload_key = ktype;
            to_lower(preload_key);
            preload_key += "_max";
            int64_t max_num =
              preload_param.tryGet<int64_t>(preload_key, std::numeric_limits<int64_t>::max());
            size_t reserved = reservedForKType(ktype);

            KTypeData kd;
            kd.ktype = ktype;
            for (const Stock& stk : stocks) {
                // 超长 market_code 无法存入定长字段，截断后可能与其他证券重名，
                // 进而使读端的升序校验失败、整段被拒映射（所有客户端退化为 IPC），故跳过
                const std::string& mc = stk.market_code();
                if (mc.size() >= sizeof(ShmStockEntry::market_code)) {
                    if (!warned_long_code) {
                        warned_long_code = true;
                        HKU_WARN("Stock {} market_code too long for shm cache, skip publish it!",
                                 mc);
                    }
                    continue;
                }
                // 空缓冲表示该证券未预加载此类型；条数达到上限则可能发生了截断。
                // 此类证券不发布，由客户端回退 IPC 查询；服务端同样从缓冲应答，
                // 两条路径的结果均与主进程保持一致。
                // 副本在 Stock 缓冲锁内一次性拷出，是自洽快照，无需再与缓冲条数比对
                KRecordList ks = stk.getKRecordListFromBuffer(ktype);
                if (ks.empty() || (int64_t)ks.size() >= max_num) {
                    continue;
                }
                total_records += ks.size();
                total_capacity += ks.size() + reserved;
                kd.entries.emplace_back(mc, std::move(ks));
            }
            if (!kd.entries.empty()) {
                std::sort(kd.entries.begin(), kd.entries.end(),
                          [](const auto& a, const auto& b) { return a.first < b.first; });
            }
            // entry 为空的类型同样记入 ktype 表：表中列出的是主进程“已预加载”的全部类型。
            // 主进程仅为已预加载的类型注册行情处理函数（见 GlobalSpotAgent），客户端据此
            // 判定某类型在主进程侧是否存在实时更新链路，从而决定是否直接走本地驱动。
            ktype_datas.emplace_back(std::move(kd));
        }
        HKU_IF_RETURN(total_records == 0, "");

        // 计算布局：[header][ktypes][entries][对齐填充][records]，全部相对段首字节偏移；
        // 记录区按 8 字节对齐（ShmKRecord 含 double），每证券含 1 交易日预留区
        size_t header_size = sizeof(ShmCacheHeader);
        size_t ktype_table_size = ktype_datas.size() * sizeof(ShmKTypeInfo);
        size_t entries_total = 0;
        for (const auto& kd : ktype_datas) {
            entries_total += kd.entries.size();
        }
        size_t entries_size = entries_total * sizeof(ShmStockEntry);
        size_t record_base = header_size + ktype_table_size + entries_size;
        record_base = (record_base + 7) & ~(size_t)7;
        size_t records_size = total_capacity * sizeof(ShmKRecord);
        size_t total_size = record_base + records_size;

        std::string name = fmt::format("{}_{:016x}", m_prefix, epoch);
        // 段名受系统限制（POSIX 一般不超过 31 字符），超长时放弃发布
        HKU_WARN_IF_RETURN(name.size() > 30, "", "Shm segment name too long ({}), skip publish!",
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
        header->ktype_count = (uint32_t)ktype_datas.size();

        size_t entry_offset = header_size + ktype_table_size;
        size_t record_offset = record_base;
        std::unordered_map<std::string, MirrorEntry> new_index;
        new_index.reserve(entries_total);
        for (size_t i = 0; i < ktype_datas.size(); i++) {
            ShmKTypeInfo* info =
              reinterpret_cast<ShmKTypeInfo*>(base + header_size + i * sizeof(ShmKTypeInfo));
            putFixedString(info->ktype, sizeof(info->ktype), ktype_datas[i].ktype);
            info->entry_offset = entry_offset;
            info->entry_count = (uint32_t)ktype_datas[i].entries.size();
            size_t reserved = reservedForKType(ktype_datas[i].ktype);

            for (const auto& entry : ktype_datas[i].entries) {
                ShmStockEntry* se = reinterpret_cast<ShmStockEntry*>(base + entry_offset);
                putFixedString(se->market_code, sizeof(se->market_code), entry.first);
                se->record_offset = record_offset;
                se->record_capacity = entry.second.size() + reserved;
                se->record_count.store(entry.second.size(), std::memory_order_relaxed);
                se->seq.store(0, std::memory_order_relaxed);
                entry_offset += sizeof(ShmStockEntry);

                ShmKRecord* rec = reinterpret_cast<ShmKRecord*>(base + record_offset);
                for (size_t r = 0; r < entry.second.size(); r++) {
                    writeShmKRecord(rec + r, entry.second[r]);
                }
                // 预留区已由整段 memset 清零，镜像追加时写入

                MirrorEntry me;
                me.entry = se;
                me.records = rec;
                new_index.emplace(entry.first + "|" + ktype_datas[i].ktype, me);

                record_offset += se->record_capacity * sizeof(ShmKRecord);
            }
        }

        // 数据全部写入后再落 magic，读端以 magic 校验段完整性；
        // release store 与读端 acquire load 配对才能跨进程建立 happens-before
        // （普通写 + atomic_thread_fence 无法保证），故 magic 为原子量；
        // 新段就绪后才替换镜像状态并删除旧段，保证已映射旧段的读者不受影响
        header->magic.store(SHM_CACHE_MAGIC, std::memory_order_release);
        region.flush();

        std::string old_name = m_current_name;
        {
            // 持注册表写锁原子替换镜像状态：进行中的镜像写入（读锁）完成后
            // 才会切换，旧段映射在替换后才解除，镜像写入不会触及已解除的映射
            std::unique_lock<std::shared_mutex> reg_lock(g_mirror_mutex);
            m_shm.swap(shm);
            m_region.swap(region);
            m_mirror_index.swap(new_index);
            m_current_name = name;
            g_mirror_pub = this;
            g_mirror_pid.store((int64_t)HKU_SHM_GETPID(), std::memory_order_relaxed);
            g_mirror_active.store(true, std::memory_order_release);
        }
        writeSegmentRecord(m_prefix, name);
        if (!old_name.empty() && old_name != name) {
            removeSegment(old_name);
        }
        HKU_INFO(
          "Published kdata shm cache: {} ({} preloaded ktypes, {} stocks entries, {} records, "
          "capacity {} records, {:.2f} MB)",
          name, ktype_datas.size(), entries_total, total_records, total_capacity,
          total_size / 1048576.0);
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
    {
        // 持注册表写锁注销：等待进行中的镜像写入（读锁）完成后才解除映射，
        // 后续镜像调用经快速路径/空指针判断直接返回
        std::unique_lock<std::shared_mutex> reg_lock(g_mirror_mutex);
        if (g_mirror_pub == this) {
            g_mirror_pub = nullptr;
            g_mirror_pid.store(0, std::memory_order_relaxed);
            g_mirror_active.store(false, std::memory_order_release);
        }
        m_mirror_index.clear();
        bi::mapped_region().swap(m_region);
        bi::shared_memory_object().swap(m_shm);
    }
    if (!m_current_name.empty()) {
        removeSegment(m_current_name);
        m_current_name.clear();
    }
}

void KDataShmPublisher::mirrorUpdate(const std::string& market_code, const KQuery::KType& ktype,
                                     const KRecord& record) {
    if (m_mirror_index.empty() || record.datetime.isNull()) {
        return;
    }
    std::string up_ktype(ktype);
    to_upper(up_ktype);
    auto it = m_mirror_index.find(market_code + "|" + up_ktype);
    if (it == m_mirror_index.end()) {
        return;
    }
    MirrorEntry& me = it->second;

    // 写者串行由 Stock::realtimeUpdate 的证券×ktype 写锁保证，
    // 段尾与缓冲尾同步演进，镜像规则与 realtimeUpdate 一致：
    // 末根同日则更新高/低/收/量额，更新（晚于末根）则追加，过期则忽略
    uint64_t count = me.entry->record_count.load(std::memory_order_relaxed);
    if (count == 0) {
        return;  // 防御：发布条目必含记录
    }
    ShmKRecord* recs = me.records;
    uint64_t dt = record.datetime.number();
    uint64_t last_dt = recs[count - 1].datetime;
    if (dt < last_dt) {
        return;
    }
    bool append = dt > last_dt;
    if (append && count >= me.entry->record_capacity) {
        // 1 交易日预留区写满：停止镜像追加（末根更新仍生效），
        // 客户端数据冻结在写满时刻，每日定时重启重新发布后恢复
        if (!me.overflow_warned) {
            me.overflow_warned = true;
            HKU_WARN("Shm cache reserved area is full for {} {}, stop mirror append!", market_code,
                     up_ktype);
        }
        return;
    }

    // seqlock 写入段：奇数 seq 标记写入中，完成后 release 落偶数 seq
    uint32_t seq = me.entry->seq.load(std::memory_order_relaxed);
    me.entry->seq.store(seq + 1, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_release);
    if (append) {
        writeShmKRecord(recs + count, record);
        me.entry->record_count.store(count + 1, std::memory_order_relaxed);
    } else {
        ShmKRecord& tmp = recs[count - 1];
        if (tmp.highPrice < record.highPrice) {
            tmp.highPrice = record.highPrice;
        }
        if (tmp.lowPrice > record.lowPrice) {
            tmp.lowPrice = record.lowPrice;
        }
        tmp.closePrice = record.closePrice;
        tmp.transAmount = record.transAmount;
        tmp.transCount = record.transCount;
    }
    me.entry->seq.store(seq + 2, std::memory_order_release);
}

void KDataShmPublisher::removeSegment(const std::string& name) {
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
               entry->record_offset + entry->record_capacity * sizeof(ShmKRecord) <= size;
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
    static bool dateRange(const ShmKRecord* recs, size_t total, const KQuery& query,
                          size_t& out_start, size_t& out_end) {
        if (total == 0) {
            return false;
        }
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

    const ShmKRecord* recs =
      reinterpret_cast<const ShmKRecord*>(m_impl->base + entry->record_offset);

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

    const ShmKRecord* recs =
      reinterpret_cast<const ShmKRecord*>(m_impl->base + entry->record_offset);

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
        // 段内容损坏（非法日期值）会使 Datetime 构造抛异常；读端接口非 noexcept，须就地
        // 兜底为“未命中”返回 false 回退 IPC/本地（同 BaseInfoShmCache）。datetime 为 8 字节
        // 对齐字段，x86-64/ARM64 上原子读写不会撕裂成非法值，故此处异常即段持久损坏，
        // 无需经 seqlock 重试区分并发撕裂。
        try {
            for (size_t i = start_ix; i < end_ix; i++) {
                out[i - start_ix] = readShmKRecord(recs[i]);
            }
        } catch (...) {
            return false;
        }
        std::atomic_thread_fence(std::memory_order_acquire);
        if (entry->seq.load(std::memory_order_relaxed) == s1) {
            return true;
        }
        out.clear();
    }
    return false;
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
