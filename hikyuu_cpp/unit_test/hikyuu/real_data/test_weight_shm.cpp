/*
 * test_weight_shm.cpp
 * 使用真实数据测试权息共享内存快照（BaseInfoShmCache 的 WEIGHT 表）的发布与只读映射查询，
 * 重点实证“SHM 命中 == IPC 回退”的同源一致性。
 *
 * 背景：历史财务路径曾因 IPC 回退（HikyuuDataServer 的 BASE_HISTORY_FINANCE）绕过缓存直查
 * 驱动、而 SHM 快照发布于 Stock 缓存，导致列式驱动下二者记录集合不一致（已修复为同源缓存）。
 * 权息路径的 IPC 回退（stk.getWeight(start, end)）与 SHM 发布源（stk.getWeight()）本就是
 * 同一方法、同一 m_weightList 缓存、同一 lower_bound 区间过滤，架构上同源；本用例在 real-test
 * 中以真实数据实证这一同源性（全量往返 + 区间过滤），确保权息不存在财务那样的异源问题。
 *
 * 真实配置可能未加载权息（load_stock_weight 关闭或数据源无权息记录），此时快照不会建
 * WEIGHT 表，测试优雅跳过（不视为失败）。
 */

#include "doctest/doctest.h"
#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <iostream>
#include <unordered_map>
#include "hikyuu/StockManager.h"
#include "hikyuu/data_driver/ipc/BaseInfoShmCache.h"

using namespace hku;
using namespace hku::ipc;

/**
 * @defgroup test_weight_shm_real test_weight_shm_real
 * @ingroup test_hikyuu_real_data
 * @{
 */

namespace {

// 逐字段比对两条权息记录（字段集与 test_BaseInfoShmCache 一致）
bool weightMatch(const StockWeight& a, const StockWeight& b) {
    return a.datetime() == b.datetime() && a.countAsGift() == b.countAsGift() &&
           a.countForSell() == b.countForSell() && a.priceForSell() == b.priceForSell() &&
           a.bonus() == b.bonus() && a.increasement() == b.increasement() &&
           a.totalCount() == b.totalCount() && a.freeCount() == b.freeCount() &&
           a.suogu() == b.suogu();
}

}  // namespace

/**
 * @par 检测点: 真实数据下权息共享内存快照与 IPC 回退源（Stock::getWeight）的同源一致性
 */
TEST_CASE("test_WeightShm_real") {
    StockManager& sm = StockManager::instance();
    sm.waitDataReady();

    // 探测真实数据是否含权息记录；getWeight() 读 m_weightList 缓存（预加载阶段已填充）
    StockList weight_stocks;
    for (const auto& stk : sm.getStockList(nullptr)) {
        if (!stk.getWeight().empty()) {
            weight_stocks.emplace_back(stk);
        }
    }
    if (weight_stocks.empty()) {
        MESSAGE("真实数据无权息记录（load_stock_weight 关闭或数据源无权息），跳过权息快照测试");
        return;
    }
    std::cout << "含权息的证券数: " << weight_stocks.size() << std::endl;

    const std::string prefix = "hkuwshm";
    BaseInfoShmPublisher publisher(prefix);
    // include_finance=false：本用例只验证权息表，不触发历史财务就绪等待
    std::string name = publisher.publish(20260904, false);
    REQUIRE_FALSE(name.empty());

    BaseInfoShmReader reader;
    REQUIRE(reader.open(name));
    if (!reader.coversTable(SHM_BI_TABLE_WEIGHT)) {
        MESSAGE("快照未建 WEIGHT 表，跳过权息比对");
        return;
    }

    // 逐证券全量比对：SHM 读端 == Stock::getWeight()（SHM 发布源，等价 getWeight(min, Null)）
    size_t checked = 0, size_mismatch = 0, content_mismatch = 0;
    const Stock* probe = &weight_stocks.front();
    size_t probe_n = 0;
    for (const auto& stk : weight_stocks) {
        const StockWeightList expect = stk.getWeight();
        StockWeightList actual;
        CHECK(reader.tryGetWeightList(stk.market_code(), Datetime::min(), Null<Datetime>(), actual));
        if (expect.size() != actual.size()) {
            size_mismatch++;
            continue;
        }
        if (expect.size() > probe_n) {
            probe_n = expect.size();
            probe = &stk;
        }
        bool bad = false;
        for (size_t i = 0; i < expect.size() && !bad; i++) {
            if (!weightMatch(expect[i], actual[i])) {
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
    CHECK_EQ(content_mismatch, 0);
    std::cout << "权息快照逐证券全量比对: checked=" << checked << "/" << weight_stocks.size()
              << std::endl;

    // 关键：区间比对 SHM 读端 == Stock::getWeight(start, end)（IPC 回退 handler 源）。
    // 财务路径的不一致正源于 IPC 回退绕过缓存直查驱动；权息 IPC 回退与 SHM 同读 m_weightList，
    // 此断言实证二者区间过滤结果逐字段一致，即“SHM 命中 == IPC 回退”。
    if (probe_n >= 3) {
        const StockWeightList full = probe->getWeight();
        Datetime sub_start = full[1].datetime();
        Datetime sub_end = full[full.size() - 1].datetime();

        const StockWeightList ipc_expect = probe->getWeight(sub_start, sub_end);  // IPC handler 源
        StockWeightList shm_actual;
        REQUIRE(reader.tryGetWeightList(probe->market_code(), sub_start, sub_end, shm_actual));
        REQUIRE_GT(ipc_expect.size(), 0);
        REQUIRE_EQ(shm_actual.size(), ipc_expect.size());
        for (size_t i = 0; i < shm_actual.size(); i++) {
            CHECK(weightMatch(ipc_expect[i], shm_actual[i]));
        }
        std::cout << "权息区间 SHM==IPC 源比对: probe=" << probe->market_code()
                  << " 区间记录数=" << shm_actual.size() << std::endl;

        // 空区间（start >= end）：与 Stock::getWeight 一致，命中但结果为空
        StockWeightList empty_range;
        CHECK(reader.tryGetWeightList(probe->market_code(), sub_end, sub_start, empty_range));
        CHECK(empty_range.empty());
        CHECK(probe->getWeight(sub_end, sub_start).empty());
    } else {
        std::cout << "样本证券权息记录数不足（<3），跳过区间比对" << std::endl;
    }

    // 未收录证券返回 false（由上层回退 IPC/本地驱动）
    StockWeightList dummy;
    CHECK_FALSE(reader.tryGetWeightList("SH999999", Datetime::min(), Null<Datetime>(), dummy));

    // 全量条目数应等于含权息记录的证券数
    std::unordered_map<std::string, StockWeightList> all;
    CHECK(reader.tryGetAllWeightList(all));
    CHECK_EQ(all.size(), weight_stocks.size());

    reader.close();
    CHECK_FALSE(reader.valid());
}

/** @} */

#else

TEST_CASE("test_WeightShm_real") {
    MESSAGE("HKU_ENABLE_NODE 未启用，跳过权息共享内存快照测试");
}

#endif  // HKU_ENABLE_NODE
