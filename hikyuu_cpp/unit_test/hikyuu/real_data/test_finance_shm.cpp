/*
 * test_finance_shm.cpp
 * 使用真实数据测试历史财务共享内存快照（BaseInfoShmCache）的发布与只读映射查询。
 *
 * 背景：unit-test 的测试数据集 HistoryFinance 表为空，无法覆盖财务记录的变长字段
 * (value_count) 段内步幅、字段往返一致性与按 reportDate 升序（对齐 SQL 驱动
 * ASC("report_date") 契约）。本用例在 real-test 中依据当前配置从真实数据获取财务记录，
 * 重点验证：
 *   1. 快照读取的财务记录与主进程缓存往返一致（reportDate/fileDate/values 逐字段）；
 *   2. 快照输出按 reportDate 升序——发布端 stable_sort 修复，列式驱动批量导入路径
 *      (getAllHistoryFinance) 本不保证顺序，故与“排序后的缓存”而非“缓存原序”比对；
 *   3. 变长字段 value_count 的段内步幅正确：快照侧恒为表级 value_count 个 float，
 *      源侧字段数不足时补零、超出时截断；
 *   4. 区间过滤 [start, end) 按 reportDate 的 ymd 比较，与驱动 getHistoryFinance 一致；
 *   5. 全市场财务条目数与含财务记录的证券数一致。
 *
 * 真实配置可能未加载历史财务（load_history_finance 关闭、字段表为空或数据源无记录），
 * 此时快照不会建 FINANCE 表，测试优雅跳过（不视为失败）。
 */

#include "doctest/doctest.h"
#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <algorithm>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>
#include "hikyuu/StockManager.h"
#include "hikyuu/HistoryFinanceInfo.h"
#include "hikyuu/data_driver/ipc/BaseInfoShmCache.h"

using namespace hku;
using namespace hku::ipc;

/**
 * @defgroup test_finance_shm_real test_finance_shm_real
 * @ingroup test_hikyuu_real_data
 * @{
 */

namespace {

// 按 (reportDate, fileDate) 规范化排序：同一 reportDate 可能有多条（季报/年报同日），
// fileDate 用于区分，规范化后消除快照（stable_sort by reportDate）与驱动两侧对同日记录
// 可能的相对次序差异，使逐位比对稳定
void canonicalSortFinance(std::vector<HistoryFinanceInfo>& v) {
    std::stable_sort(v.begin(), v.end(),
                     [](const HistoryFinanceInfo& a, const HistoryFinanceInfo& b) {
                         if (a.reportDate != b.reportDate) {
                             return a.reportDate < b.reportDate;
                         }
                         return a.fileDate < b.fileDate;
                     });
}

// 比对一条财务记录，对表级 value_count 的填充/截断鲁棒：
// 快照侧（shm）恒为 value_count 个 float；源侧（src）字段数不足时快照补零、超出时截断，
// 故仅比对重叠字段，并校验源侧不足部分在快照中确为零填充
bool financeRecordMatch(const HistoryFinanceInfo& src, const HistoryFinanceInfo& shm,
                        size_t value_count) {
    if (src.reportDate != shm.reportDate || src.fileDate != shm.fileDate) {
        return false;
    }
    if (shm.values.size() != value_count) {
        return false;
    }
    size_t m = std::min(src.values.size(), value_count);
    for (size_t j = 0; j < m; j++) {
        if (src.values[j] != shm.values[j]) {
            return false;
        }
    }
    for (size_t j = src.values.size(); j < value_count; j++) {
        if (shm.values[j] != 0.0f) {
            return false;
        }
    }
    return true;
}

}  // namespace

/**
 * @par 检测点: 真实数据下历史财务共享内存快照的往返一致性、reportDate 升序、
 *             变长字段步幅与区间过滤语义
 */
TEST_CASE("test_HistoryFinanceShm_real") {
    StockManager& sm = StockManager::instance();
    sm.waitDataReady();

    // 财务字段表为空时快照不会建 FINANCE 表（publish 内 fields.empty() 门控），先行跳过；
    // 此检查不触发逐证券懒加载，代价极低
    auto fields = sm.getHistoryFinanceAllFields();
    if (fields.empty()) {
        MESSAGE("财务字段表为空（历史财务未加载），跳过财务共享内存快照测试");
        return;
    }

    // 探测真实数据是否含历史财务记录；getHistoryFinance() 返回 const 引用，不触发拷贝。
    // 注意：若配置未预加载财务，此处会逐证券触发懒加载，代价较高——real-test 默认
    // load_history_finance=true，财务已在预加载阶段就绪，故为缓存命中
    StockList finance_stocks;
    for (const auto& stk : sm.getStockList(nullptr)) {
        if (!stk.getHistoryFinance().empty()) {
            finance_stocks.emplace_back(stk);
        }
    }
    if (finance_stocks.empty()) {
        MESSAGE("真实数据无历史财务记录，跳过财务共享内存快照测试");
        return;
    }
    std::cout << "含历史财务的证券数: " << finance_stocks.size() << std::endl;

    const std::string prefix = "hkufin";
    BaseInfoShmPublisher publisher(prefix);
    std::string name = publisher.publish(20260904, true);  // include_finance = true
    REQUIRE_FALSE(name.empty());

    BaseInfoShmReader reader;
    REQUIRE(reader.open(name));
    if (!reader.coversTable(SHM_BI_TABLE_FINANCE)) {
        MESSAGE("快照未建 FINANCE 表（load_history_finance 关闭），跳过财务比对");
        return;
    }

    const size_t value_count = reader.financeValueCount();
    CHECK_EQ(value_count, fields.size());
    REQUIRE_GT(value_count, 0);
    std::cout << "FINANCE 表字段数 value_count: " << value_count << std::endl;

    // 逐证券比对：快照输出须按 reportDate 升序，且等于对主进程缓存按 reportDate 稳定升序
    // 排序后的结果（发布端 stable_sort 语义）；同时选取记录数最多者作为后续区间/驱动比对的样本
    size_t checked = 0, size_mismatch = 0, order_violation = 0, content_mismatch = 0, padded = 0;
    const Stock* probe = &finance_stocks.front();
    size_t probe_n = 0;
    for (const auto& stk : finance_stocks) {
        std::vector<HistoryFinanceInfo> expect = stk.getHistoryFinance();  // 拷贝以便排序
        std::vector<HistoryFinanceInfo> actual;
        CHECK(reader.tryGetHistoryFinance(stk.market_code(), Datetime::min(), Null<Datetime>(),
                                          actual));

        if (expect.size() != actual.size()) {
            size_mismatch++;
            continue;
        }
        if (expect.size() > probe_n) {
            probe_n = expect.size();
            probe = &stk;
        }

        // 升序不变量（独立于缓存原序，直接验证发布端 stable_sort 的效果）
        for (size_t i = 1; i < actual.size(); i++) {
            if (actual[i].reportDate < actual[i - 1].reportDate) {
                order_violation++;
                break;
            }
        }

        // 与按 reportDate 稳定升序排序后的缓存逐字段比对
        std::stable_sort(expect.begin(), expect.end(),
                         [](const HistoryFinanceInfo& a, const HistoryFinanceInfo& b) {
                             return a.reportDate < b.reportDate;
                         });
        bool bad = false;
        for (size_t i = 0; i < expect.size() && !bad; i++) {
            if (expect[i].values.size() != value_count) {
                padded++;  // 源字段数与表级 value_count 不一致（填充/截断边界）
            }
            if (!financeRecordMatch(expect[i], actual[i], value_count)) {
                bad = true;
            }
        }
        if (bad) {
            content_mismatch++;
            continue;
        }
        checked++;
    }
    CHECK_EQ(size_mismatch, 0);
    CHECK_EQ(order_violation, 0);
    CHECK_EQ(content_mismatch, 0);
    std::cout << "财务快照逐证券比对: checked=" << checked << "/" << finance_stocks.size()
              << " 填充/截断记录数=" << padded << std::endl;

    if (probe_n > 2) {
        // 区间过滤语义：与驱动对齐（按 reportDate 的 ymd，含头不含尾）
        auto full = probe->getHistoryFinance();
        canonicalSortFinance(full);
        Datetime sub_start = full[1].reportDate;
        Datetime sub_end = full[full.size() - 1].reportDate;

        std::vector<HistoryFinanceInfo> range_actual;
        CHECK(reader.tryGetHistoryFinance(probe->market_code(), sub_start, sub_end, range_actual));
        canonicalSortFinance(range_actual);
        // 期望：缓存中 reportDate.ymd() ∈ [sub_start.ymd(), sub_end.ymd()) 的记录
        std::vector<HistoryFinanceInfo> range_expect;
        for (const auto& r : full) {
            uint64_t ymd = r.reportDate.ymd();
            if (ymd >= sub_start.ymd() && ymd < sub_end.ymd()) {
                range_expect.emplace_back(r);
            }
        }
        REQUIRE_EQ(range_actual.size(), range_expect.size());
        for (size_t i = 0; i < range_actual.size(); i++) {
            CHECK(financeRecordMatch(range_expect[i], range_actual[i], value_count));
        }

        // 诊断（坐实修复效果，不断言）：列式驱动批量 getAllHistoryFinance 可能对同一
        // (reportDate,fileDate) 返回重复行（缓存），而逐证券直查（sm.getHistoryFinance）不重复。
        // 两处修复：① IPC 回退改走缓存（与 SHM 同源）；② 缓存填充 setHistoryFinance 按
        // (reportDate,fileDate) 去重。去重后缓存与逐证券直查记录集合应完全一致（size + 内容），
        // 进而 SHM 快照、IPC 回退、本地直查四路径全部归一。此处打印对比坐实去重效果。
        auto direct = sm.getHistoryFinance(*probe, Datetime::min(), Null<Datetime>());
        std::cout << "probe " << probe->market_code() << " 缓存记录数=" << full.size()
                  << " 逐证券直查记录数=" << direct.size();
        if (full.size() != direct.size()) {
            std::set<std::pair<uint64_t, uint64_t>> direct_keys;
            for (const auto& r : direct) {
                direct_keys.emplace(r.reportDate.number(), r.fileDate.number());
            }
            std::cout << " 缓存独有:";
            for (const auto& r : full) {
                if (direct_keys.find({r.reportDate.number(), r.fileDate.number()}) ==
                    direct_keys.end()) {
                    std::cout << " [report=" << r.reportDate << " file=" << r.fileDate << "]";
                }
            }
        } else {
            // 去重后 size 一致：逐字段对比（含 values），坐实缓存与逐证券直查完全一致
            auto direct_sorted = direct;
            canonicalSortFinance(direct_sorted);
            bool identical = true;
            for (size_t i = 0; i < full.size() && identical; i++) {
                if (full[i].reportDate != direct_sorted[i].reportDate ||
                    full[i].fileDate != direct_sorted[i].fileDate ||
                    full[i].values != direct_sorted[i].values) {
                    identical = false;
                }
            }
            std::cout << (identical ? " 去重后缓存与逐证券直查逐字段完全一致"
                                    : " 警告:去重后 size 相同但内容有差异");
        }
        std::cout << std::endl;

        // 空区间（start >= end）：命中但结果为空
        std::vector<HistoryFinanceInfo> empty_range;
        CHECK(reader.tryGetHistoryFinance(probe->market_code(), sub_end, sub_start, empty_range));
        CHECK(empty_range.empty());
    } else {
        std::cout << "样本证券财务记录数不足（<=2），跳过区间/驱动比对" << std::endl;
    }

    // 未收录证券返回 false（由上层回退 IPC/本地驱动）
    std::vector<HistoryFinanceInfo> dummy;
    CHECK_FALSE(reader.tryGetHistoryFinance("SH999999", Datetime::min(), Null<Datetime>(), dummy));

    // 全市场财务：条目数应等于含财务记录的证券数
    std::unordered_map<std::string, std::vector<HistoryFinanceInfo>> all;
    CHECK(reader.tryGetAllHistoryFinance(all));
    CHECK_EQ(all.size(), finance_stocks.size());

    reader.close();
    CHECK_FALSE(reader.valid());
}

/** @} */

#else

TEST_CASE("test_HistoryFinanceShm_real") {
    MESSAGE("HKU_ENABLE_NODE 未启用，跳过历史财务共享内存快照测试");
}

#endif  // HKU_ENABLE_NODE
