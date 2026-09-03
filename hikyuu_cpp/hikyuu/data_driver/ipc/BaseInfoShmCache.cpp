/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-03
 *      Author: fasiondog
 */

#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include "BaseInfoShmCache.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
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
void biPutFixedString(char* dst, size_t cap, const std::string& src) {
    std::memset(dst, 0, cap);
    size_t n = std::min(cap - 1, src.size());
    std::memcpy(dst, src.data(), n);
}

/** 定长字符串字段读出 */
std::string biGetFixedString(const char* src, size_t cap) {
    size_t n = 0;
    while (n < cap && src[n] != '\0') {
        n++;
    }
    return std::string(src, n);
}

void writeShmWeightRecord(ShmWeightRecord* dst, const StockWeight& w) {
    dst->datetime = w.datetime().number();
    dst->countAsGift = w.countAsGift();
    dst->countForSell = w.countForSell();
    dst->priceForSell = w.priceForSell();
    dst->bonus = w.bonus();
    dst->increasement = w.increasement();
    dst->totalCount = w.totalCount();
    dst->freeCount = w.freeCount();
    dst->suogu = w.suogu();
}

StockWeight readShmWeightRecord(const ShmWeightRecord& src) {
    return StockWeight(Datetime(src.datetime), src.countAsGift, src.countForSell, src.priceForSell,
                       src.bonus, src.increasement, src.totalCount, src.freeCount, src.suogu);
}

/** 段名记录文件：异常退出（崩溃/被杀）残留段由新 Master 首次发布时依此清理 */
std::string biSegmentRecordFilePath(const std::string& prefix) {
    return (std::filesystem::temp_directory_path() / (prefix + ".last")).string();
}

std::string biReadSegmentRecord(const std::string& prefix) {
    try {
        std::ifstream ifs(biSegmentRecordFilePath(prefix));
        std::string name;
        if (ifs.is_open() && std::getline(ifs, name)) {
            return name;
        }
    } catch (...) {
    }
    return "";
}

void biWriteSegmentRecord(const std::string& prefix, const std::string& name) {
    try {
        std::ofstream ofs(biSegmentRecordFilePath(prefix), std::ios::trunc);
        if (ofs.is_open()) {
            ofs << name;
        }
    } catch (...) {
    }
}

/** 待发布的一张表 */
struct BiTableData {
    std::string name;
    uint32_t value_count{0};  ///< FINANCE 使用；WEIGHT 为 0
    size_t record_size{0};    ///< 单条记录的段内步幅
    std::vector<std::pair<std::string, size_t>> entries;  // (market_code, 记录条数)，按 code 升序
    std::vector<StockWeightList> weights;                 // WEIGHT 表的记录内容
    std::vector<std::vector<HistoryFinanceInfo>> finances;  // FINANCE 表的记录内容
};

}  // namespace

///////////////////////////////////////////////////////////////////////////////
// BaseInfoShmPublisher
///////////////////////////////////////////////////////////////////////////////
BaseInfoShmPublisher::BaseInfoShmPublisher(const std::string& shm_name_prefix)
: m_prefix(shm_name_prefix) {}

BaseInfoShmPublisher::~BaseInfoShmPublisher() {
    removeAll();
}

std::string BaseInfoShmPublisher::publish(uint64_t epoch, bool include_finance) {
    // 新段名（创建成功后赋值），异常时仅清理新段，保留旧段继续服务
    std::string created_name;
    try {
        auto& sm = StockManager::instance();
        const auto& hikyuu_param = sm.getHikyuuParameter();

        // 首次发布前清理上一个异常退出的 Master 残留段（本进程已持文件锁，是唯一 Master）
        if (m_current_name.empty()) {
            std::string orphan = biReadSegmentRecord(m_prefix);
            if (!orphan.empty()) {
                removeSegment(orphan);
            }
        }

        auto stocks = sm.getStockList(nullptr);
        HKU_IF_RETURN(stocks.empty(), "");

        // 发布端须按 market_code 升序排列 entry，读端据此二分查找；
        // getStockList 的返回顺序不保证按 market_code 有序，故显式排序。
        // 权息与历史财务两张表共用同一证券顺序，排序一次即两者皆有序。
        std::sort(stocks.begin(), stocks.end(),
                  [](const Stock& a, const Stock& b) { return a.market_code() < b.market_code(); });

        std::vector<BiTableData> tables;

        // 权息：load_stock_weight 为假时主进程根本不加载，客户端也拿不到，故不建表，
        // 由客户端经 coversTable 判定后走本地驱动（本地若同样关闭则结果一致为空）
        if (hikyuu_param.tryGet<bool>("load_stock_weight", true)) {
            BiTableData td;
            td.name = SHM_BI_TABLE_WEIGHT;
            td.record_size = sizeof(ShmWeightRecord);
            for (const Stock& stk : stocks) {
                const std::string& mc = stk.market_code();
                if (mc.size() >= sizeof(ShmBiEntry::market_code)) {
                    continue;  // 定长字段放不下，截断后可能与其它证券重名，跳过
                }
                StockWeightList ws = stk.getWeight();
                if (ws.empty()) {
                    continue;  // 无权息的证券不占条目，由客户端回退（结果同样为空）
                }
                td.entries.emplace_back(mc, ws.size());
                td.weights.emplace_back(std::move(ws));
            }
            if (!td.entries.empty()) {
                tables.emplace_back(std::move(td));
            }
        }

        // 历史财务：load_history_finance 仅控制预加载，懒加载路径无配置门控；
        // 未预加载时逐证券触发驱动查询代价过高，故同样以配置为门控，未加载则不建表。
        // 发布点位于预加载线程的历史财务加载之后，此时各证券缓存已就绪；
        // 但在权息就绪、历史财务尚未预加载的提前发布点（StockManager::init 内），
        // 必须传 include_finance=false，否则会逐证券触发历史财务懒加载（见 Stock::getHistoryFinance）。
        if (include_finance && hikyuu_param.tryGet<bool>("load_history_finance", true)) {
            auto fields = sm.getHistoryFinanceAllFields();
            if (!fields.empty()) {
                BiTableData td;
                td.name = SHM_BI_TABLE_FINANCE;
                td.value_count = (uint32_t)fields.size();
                td.record_size = financeRecordStride(td.value_count);
                for (const Stock& stk : stocks) {
                    const std::string& mc = stk.market_code();
                    if (mc.size() >= sizeof(ShmBiEntry::market_code)) {
                        continue;
                    }
                    const auto& fins = stk.getHistoryFinance();
                    if (fins.empty()) {
                        continue;
                    }
                    td.entries.emplace_back(mc, fins.size());
                    td.finances.emplace_back(fins);
                    // 对齐本地驱动 ASC("report_date") 契约：列式驱动的批量导入路径
                    // (getAllHistoryFinance) 不保证顺序，发布前按 reportDate 升序稳定排序，使快照
                    // 命中的输出顺序与 SQL 驱动回退一致；同一 reportDate 可能有多条(季报/年报同日)，
                    // stable_sort 保留其相对次序，与 ASC 仅按 report_date 排序的语义相符
                    std::stable_sort(td.finances.back().begin(), td.finances.back().end(),
                                     [](const HistoryFinanceInfo& a, const HistoryFinanceInfo& b) {
                                         return a.reportDate < b.reportDate;
                                     });
                }
                if (!td.entries.empty()) {
                    tables.emplace_back(std::move(td));
                }
            }
        }

        HKU_IF_RETURN(tables.empty(), "");

        // 计算布局：[header][tables][entries][对齐填充][records]
        size_t header_size = sizeof(ShmBiHeader);
        size_t table_area = tables.size() * sizeof(ShmBiTableInfo);
        size_t entry_count_total = 0;
        size_t records_size = 0;
        for (const auto& td : tables) {
            entry_count_total += td.entries.size();
            for (size_t i = 0; i < td.entries.size(); i++) {
                records_size += td.entries[i].second * td.record_size;
            }
        }
        size_t record_base = header_size + table_area + entry_count_total * sizeof(ShmBiEntry);
        record_base = (record_base + 7) & ~(size_t)7;
        size_t total_size = record_base + records_size;

        std::string name = fmt::format("{}_{:016x}", m_prefix, epoch);
        // 段名受系统限制（POSIX 一般不超过 31 字符），超长时放弃发布
        HKU_WARN_IF_RETURN(name.size() > 30, "", "Shm segment name too long ({}), skip publish!",
                           name);
        removeSegment(name);

        bi::shared_memory_object shm(bi::create_only, name.c_str(), bi::read_write);
        created_name = name;
        shm.truncate(total_size);
        bi::mapped_region region(shm, bi::read_write);
        uint8_t* base = static_cast<uint8_t*>(region.get_address());
        std::memset(base, 0, total_size);

        ShmBiHeader* header = reinterpret_cast<ShmBiHeader*>(base);
        header->version = SHM_BI_VERSION;
        header->epoch = epoch;
        header->data_size = total_size;
        header->table_count = (uint32_t)tables.size();

        size_t entry_offset = header_size + table_area;
        size_t record_offset = record_base;
        for (size_t t = 0; t < tables.size(); t++) {
            const BiTableData& td = tables[t];
            ShmBiTableInfo* info =
              reinterpret_cast<ShmBiTableInfo*>(base + header_size + t * sizeof(ShmBiTableInfo));
            biPutFixedString(info->name, sizeof(info->name), td.name);
            info->entry_offset = entry_offset;
            info->entry_count = (uint32_t)td.entries.size();
            info->value_count = td.value_count;

            for (size_t i = 0; i < td.entries.size(); i++) {
                ShmBiEntry* se = reinterpret_cast<ShmBiEntry*>(base + entry_offset);
                biPutFixedString(se->market_code, sizeof(se->market_code), td.entries[i].first);
                se->record_offset = record_offset;
                se->record_count = td.entries[i].second;
                se->value_count = td.value_count;
                entry_offset += sizeof(ShmBiEntry);

                uint8_t* rec_base = base + record_offset;
                if (td.name == SHM_BI_TABLE_WEIGHT) {
                    const StockWeightList& ws = td.weights[i];
                    for (size_t r = 0; r < ws.size(); r++) {
                        writeShmWeightRecord(reinterpret_cast<ShmWeightRecord*>(rec_base) + r,
                                             ws[r]);
                    }
                } else {
                    const auto& fins = td.finances[i];
                    for (size_t r = 0; r < fins.size(); r++) {
                        ShmFinanceRecord* fr = reinterpret_cast<ShmFinanceRecord*>(
                          rec_base + r * td.record_size);
                        fr->reportDate = fins[r].reportDate.number();
                        fr->fileDate = fins[r].fileDate.number();
                        float* vals = reinterpret_cast<float*>(
                          rec_base + r * td.record_size + sizeof(ShmFinanceRecord));
                        // 字段数不一致（主进程数据集变更）时按表级字段数截断/补零，
                        // 避免越界写入；客户端以表级 value_count 读取，二者始终一致
                        size_t n = std::min((size_t)td.value_count, fins[r].values.size());
                        for (size_t v = 0; v < n; v++) {
                            vals[v] = fins[r].values[v];
                        }
                        for (size_t v = n; v < (size_t)td.value_count; v++) {
                            vals[v] = 0.0f;
                        }
                    }
                }
                record_offset += td.entries[i].second * td.record_size;
            }
        }

        // 数据全部写入后再落 magic（release 语义），读端以 magic + version 作为整段就绪判据
        header->magic.store(SHM_BI_MAGIC, std::memory_order_release);
        region.flush();

        std::string old_name = m_current_name;
        m_shm.swap(shm);
        m_region.swap(region);
        m_current_name = name;

        biWriteSegmentRecord(m_prefix, name);
        if (!old_name.empty() && old_name != name) {
            removeSegment(old_name);
        }

        size_t total_records = 0;
        for (const auto& td : tables) {
            for (const auto& e : td.entries) {
                total_records += e.second;
            }
        }
        HKU_INFO(
          "Published base info shm cache: {} ({} tables, {} stock entries, {} records, {:.2f} MB)",
          name, tables.size(), entry_count_total, total_records, total_size / 1048576.0);
        return name;

    } catch (const std::exception& e) {
        HKU_ERROR("Failed publish base info shm cache: {}", e.what());
        if (!created_name.empty() && created_name != m_current_name) {
            removeSegment(created_name);
        }
        return "";
    } catch (...) {
        HKU_ERROR("Failed publish base info shm cache: unknown error!");
        if (!created_name.empty() && created_name != m_current_name) {
            removeSegment(created_name);
        }
        return "";
    }
}

void BaseInfoShmPublisher::removeAll() {
    bi::mapped_region().swap(m_region);
    bi::shared_memory_object().swap(m_shm);
    if (!m_current_name.empty()) {
        removeSegment(m_current_name);
        m_current_name.clear();
    }
}

void BaseInfoShmPublisher::removeSegment(const std::string& name) {
    try {
        bi::shared_memory_object::remove(name.c_str());
    } catch (...) {
    }
}

///////////////////////////////////////////////////////////////////////////////
// BaseInfoShmReader
///////////////////////////////////////////////////////////////////////////////
struct BaseInfoShmReader::Impl {
    struct TableIndex {
        std::string name;
        uint32_t value_count{0};
        size_t record_size{0};
        const ShmBiEntry* entries{nullptr};
        uint32_t entry_count{0};
    };

    std::string name;
    uint64_t epoch{0};
    size_t covered{0};
    bi::shared_memory_object shm;
    bi::mapped_region region;
    const uint8_t* base{nullptr};
    size_t size{0};
    std::vector<TableIndex> tables;

    const TableIndex* findTable(const std::string& table) const {
        for (const auto& t : tables) {
            if (t.name == table) {
                return &t;
            }
        }
        return nullptr;
    }

    static const ShmBiEntry* findEntry(const TableIndex& idx, const std::string& key) {
        if (key.size() >= sizeof(ShmBiEntry::market_code)) {
            return nullptr;
        }
        char buf[sizeof(ShmBiEntry::market_code)];
        biPutFixedString(buf, sizeof(buf), key);
        auto cmp = [](const ShmBiEntry& entry, const char* k) {
            return std::memcmp(entry.market_code, k, sizeof(ShmBiEntry::market_code)) < 0;
        };
        const ShmBiEntry* first = idx.entries;
        const ShmBiEntry* it = std::lower_bound(first, first + idx.entry_count, buf, cmp);
        if (it != first + idx.entry_count &&
            std::memcmp(it->market_code, buf, sizeof(ShmBiEntry::market_code)) == 0) {
            return it;
        }
        return nullptr;
    }

    /** 校验记录区边界，防止段损坏导致越界读取 */
    bool checkRecords(const TableIndex& idx, const ShmBiEntry* entry) const {
        // 必须先确认 record_offset 在段内再做减法：size 与 record_offset 均为无符号，
        // 若段损坏使 record_offset > size，(size - record_offset) 会下溢成巨值而误通过校验
        return entry->record_offset % 8 == 0 && entry->record_count > 0 && idx.record_size > 0 &&
               entry->record_offset < size &&
               entry->record_count <= (size - entry->record_offset) / idx.record_size;
    }

    /** 按段内步幅解码单条历史财务记录 */
    static HistoryFinanceInfo readFinanceRecord(const uint8_t* p, uint32_t value_count) {
        const ShmFinanceRecord* fr = reinterpret_cast<const ShmFinanceRecord*>(p);
        const float* vals = reinterpret_cast<const float*>(p + sizeof(ShmFinanceRecord));
        HistoryFinanceInfo info;
        info.reportDate = Datetime(fr->reportDate);
        info.fileDate = Datetime(fr->fileDate);
        info.values.resize(value_count);
        for (uint32_t v = 0; v < value_count; v++) {
            info.values[v] = vals[v];
        }
        return info;
    }
};

BaseInfoShmReader::BaseInfoShmReader() = default;
BaseInfoShmReader::~BaseInfoShmReader() = default;

bool BaseInfoShmReader::open(const std::string& name) {
    close();
    try {
        auto impl = std::make_unique<Impl>();
        bi::shared_memory_object shm(bi::open_only, name.c_str(), bi::read_only);
        bi::mapped_region region(shm, bi::read_only);
        HKU_IF_RETURN(region.get_size() < sizeof(ShmBiHeader), false);

        const uint8_t* base = static_cast<const uint8_t*>(region.get_address());
        const ShmBiHeader* header = reinterpret_cast<const ShmBiHeader*>(base);
        HKU_IF_RETURN(header->magic.load(std::memory_order_acquire) != SHM_BI_MAGIC, false);
        HKU_IF_RETURN(header->version != SHM_BI_VERSION, false);
        HKU_IF_RETURN(header->data_size == 0 || header->data_size > region.get_size(), false);

        impl->epoch = header->epoch;
        impl->base = base;
        impl->size = header->data_size;

        for (uint32_t t = 0; t < header->table_count; t++) {
            size_t info_off = sizeof(ShmBiHeader) + t * sizeof(ShmBiTableInfo);
            HKU_IF_RETURN(info_off + sizeof(ShmBiTableInfo) > impl->size, false);
            const ShmBiTableInfo* info =
              reinterpret_cast<const ShmBiTableInfo*>(base + info_off);
            size_t entries_end = info->entry_offset + (size_t)info->entry_count * sizeof(ShmBiEntry);
            HKU_IF_RETURN(info->entry_offset % 8 != 0 ||
                            info->entry_offset < sizeof(ShmBiHeader) || entries_end > impl->size,
                          false);

            Impl::TableIndex idx;
            idx.name = biGetFixedString(info->name, sizeof(info->name));
            to_upper(idx.name);
            idx.value_count = info->value_count;
            idx.entries = reinterpret_cast<const ShmBiEntry*>(base + info->entry_offset);
            idx.entry_count = info->entry_count;
            idx.record_size = (idx.name == SHM_BI_TABLE_FINANCE)
                                ? financeRecordStride(idx.value_count)
                                : sizeof(ShmWeightRecord);
            HKU_IF_RETURN(idx.record_size == 0, false);

            // 校验发布端已按 market_code 升序排列（二分查找前提）
            for (uint32_t j = 1; j < idx.entry_count; j++) {
                HKU_IF_RETURN(std::memcmp(idx.entries[j - 1].market_code,
                                          idx.entries[j].market_code,
                                          sizeof(ShmBiEntry::market_code)) >= 0,
                              false);
            }

            impl->covered += idx.entry_count;
            impl->tables.emplace_back(std::move(idx));
        }

        impl->name = name;
        impl->shm = std::move(shm);
        impl->region = std::move(region);
        m_impl = std::move(impl);
        return true;
    } catch (const std::exception& e) {
        HKU_WARN("Failed open base info shm cache {}: {}", name, e.what());
    } catch (...) {
        HKU_WARN("Failed open base info shm cache: {}!", name);
    }
    return false;
}

void BaseInfoShmReader::close() {
    m_impl.reset();
}

const std::string& BaseInfoShmReader::name() const noexcept {
    static const std::string empty;
    return m_impl ? m_impl->name : empty;
}

uint64_t BaseInfoShmReader::epoch() const noexcept {
    return m_impl ? m_impl->epoch : 0;
}

size_t BaseInfoShmReader::coveredCount() const noexcept {
    return m_impl ? m_impl->covered : 0;
}

bool BaseInfoShmReader::coversTable(const std::string& table) const {
    HKU_IF_RETURN(!m_impl, false);
    std::string upper(table);
    to_upper(upper);
    return m_impl->findTable(upper) != nullptr;
}

size_t BaseInfoShmReader::financeValueCount() const noexcept {
    HKU_IF_RETURN(!m_impl, 0);
    const Impl::TableIndex* idx = m_impl->findTable(SHM_BI_TABLE_FINANCE);
    return idx ? idx->value_count : 0;
}

bool BaseInfoShmReader::tryGetWeightList(const std::string& market_code, const Datetime& start,
                                         const Datetime& end, StockWeightList& out) const {
    out.clear();
    HKU_IF_RETURN(!m_impl, false);
    const Impl::TableIndex* idx = m_impl->findTable(SHM_BI_TABLE_WEIGHT);
    HKU_IF_RETURN(!idx, false);
    const ShmBiEntry* entry = Impl::findEntry(*idx, market_code);
    HKU_IF_RETURN(!entry, false);
    HKU_IF_RETURN(!m_impl->checkRecords(*idx, entry), false);

    const uint8_t* recs = m_impl->base + entry->record_offset;
    uint64_t start_num = start.isNull() ? 0 : start.number();
    uint64_t end_num = end.isNull() ? (std::numeric_limits<uint64_t>::max)() : end.number();

    // 记录按权息日期升序，二分定位 [start, end)
    size_t low = 0, high = (size_t)entry->record_count;
    while (low < high) {
        size_t mid = (low + high) / 2;
        uint64_t dt =
          reinterpret_cast<const ShmWeightRecord*>(recs + mid * idx->record_size)->datetime;
        if (dt < start_num) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    // 段内容损坏（如非法日期值）会使 Datetime 构造抛异常；读端接口非 noexcept，
    // 须就地兜底为“未命中”返回 false，交由调用方回退 IPC/本地驱动（与 open() 一致）
    try {
        out.reserve((size_t)entry->record_count - low);
        for (size_t i = low; i < (size_t)entry->record_count; i++) {
            const ShmWeightRecord* rec =
              reinterpret_cast<const ShmWeightRecord*>(recs + i * idx->record_size);
            if (rec->datetime >= end_num) {
                break;
            }
            out.emplace_back(readShmWeightRecord(*rec));
        }
    } catch (...) {
        return false;
    }
    return true;
}

bool BaseInfoShmReader::tryGetAllWeightList(
  std::unordered_map<std::string, StockWeightList>& out) const {
    out.clear();
    HKU_IF_RETURN(!m_impl, false);
    const Impl::TableIndex* idx = m_impl->findTable(SHM_BI_TABLE_WEIGHT);
    HKU_IF_RETURN(!idx, false);

    // 段内容损坏（如非法日期值）会使 Datetime 构造抛异常，兜底为未命中回退（同 tryGetWeightList）
    try {
        for (uint32_t i = 0; i < idx->entry_count; i++) {
            const ShmBiEntry* entry = idx->entries + i;
            HKU_IF_RETURN(!m_impl->checkRecords(*idx, entry), false);
            std::string mc = biGetFixedString(entry->market_code, sizeof(entry->market_code));
            StockWeightList ws;
            ws.reserve((size_t)entry->record_count);
            const uint8_t* recs = m_impl->base + entry->record_offset;
            for (uint64_t r = 0; r < entry->record_count; r++) {
                ws.emplace_back(readShmWeightRecord(
                  *reinterpret_cast<const ShmWeightRecord*>(recs + r * idx->record_size)));
            }
            out.emplace(std::move(mc), std::move(ws));
        }
    } catch (...) {
        return false;
    }
    return true;
}

bool BaseInfoShmReader::tryGetHistoryFinance(const std::string& market_code, const Datetime& start,
                                             const Datetime& end,
                                             std::vector<HistoryFinanceInfo>& out) const {
    out.clear();
    HKU_IF_RETURN(!m_impl, false);
    const Impl::TableIndex* idx = m_impl->findTable(SHM_BI_TABLE_FINANCE);
    HKU_IF_RETURN(!idx, false);
    const ShmBiEntry* entry = Impl::findEntry(*idx, market_code);
    HKU_IF_RETURN(!entry, false);
    HKU_IF_RETURN(!m_impl->checkRecords(*idx, entry), false);

    // 与本地驱动的 SQL 过滤语义保持一致（report_date >= start.ymd() and < end.ymd()），
    // 否则快照命中会比服务端 IPC 应答多返回区间外的记录
    HKU_IF_RETURN(start >= end, true);
    uint64_t start_ymd = start.isNull() ? Datetime::min().ymd() : start.ymd();
    uint64_t end_ymd = end.isNull() ? Datetime::max().ymd() : end.ymd();

    const uint8_t* recs = m_impl->base + entry->record_offset;
    // 段内容损坏（如非法日期值）会使 Datetime 构造抛异常，兜底为未命中回退（同 tryGetWeightList）
    try {
        out.reserve((size_t)entry->record_count);
        for (uint64_t r = 0; r < entry->record_count; r++) {
            const uint8_t* p = recs + r * idx->record_size;
            // 逐条判定区间而不依赖升序提前终止：同一报告日可能有多条（季报/年报同日），
            // 且列式批量导入路径不保证缓存内的排序
            const ShmFinanceRecord* fr = reinterpret_cast<const ShmFinanceRecord*>(p);
            uint64_t report_ymd = Datetime(fr->reportDate).ymd();
            if (report_ymd < start_ymd || report_ymd >= end_ymd) {
                continue;
            }
            out.emplace_back(Impl::readFinanceRecord(p, idx->value_count));
        }
    } catch (...) {
        return false;
    }
    return true;
}

bool BaseInfoShmReader::tryGetAllHistoryFinance(
  std::unordered_map<std::string, std::vector<HistoryFinanceInfo>>& out) const {
    out.clear();
    HKU_IF_RETURN(!m_impl, false);
    const Impl::TableIndex* idx = m_impl->findTable(SHM_BI_TABLE_FINANCE);
    HKU_IF_RETURN(!idx, false);

    // 段内容损坏（如非法日期值）会使 Datetime 构造抛异常，兜底为未命中回退（同 tryGetWeightList）
    try {
        for (uint32_t i = 0; i < idx->entry_count; i++) {
            const ShmBiEntry* entry = idx->entries + i;
            HKU_IF_RETURN(!m_impl->checkRecords(*idx, entry), false);
            std::string mc = biGetFixedString(entry->market_code, sizeof(entry->market_code));
            std::vector<HistoryFinanceInfo> fins;
            fins.reserve((size_t)entry->record_count);
            const uint8_t* recs = m_impl->base + entry->record_offset;
            for (uint64_t r = 0; r < entry->record_count; r++) {
                fins.emplace_back(
                  Impl::readFinanceRecord(recs + r * idx->record_size, idx->value_count));
            }
            out.emplace(std::move(mc), std::move(fins));
        }
    } catch (...) {
        return false;
    }
    return true;
}

}  // namespace ipc
}  // namespace hku

#endif  // HKU_ENABLE_NODE
