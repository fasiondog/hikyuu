/*
 * test_kdata_shm.cpp
 * 使用真实数据测试 K 线共享内存快照（KDataShmCache）的发布与只读映射查询，
 * 重点实证“SHM 命中 == IPC 回退”的同源一致性。
 *
 * 背景：历史财务路径曾因 IPC 回退（HikyuuDataServer）绕过缓存直查驱动、与 SHM 快照
 * （发布于 Stock 缓存）异源，导致列式驱动下记录集合不一致（已修复为同源缓存）。KData 路径的
 * IPC 回退（stk.getCount / getKRecordList / getIndexRange）经 _getKRecordList 读取预加载缓冲，
 * 而 SHM 快照同样发布于该缓冲，架构上同源；本用例在 real-test 中以真实数据（day 已预加载）
 * 实证这一同源性，确保 KData 不存在财务那样的异源问题。
 *
 * 注：KData 快照体量大（真实全市场 day 约千万级记录、GB 级共享内存），逐证券全量拷贝代价高，
 * 故按抽样上限验证；发布失败（如内存不足）或未预加载 day 时优雅跳过（不视为失败）。
 */

#include "doctest/doctest.h"
#include "hikyuu/utilities/config.h"

#if HKU_ENABLE_NODE

#include <iostream>
#include "hikyuu/StockManager.h"
#include "hikyuu/data_driver/ipc/KDataShmCache.h"

using namespace hku;
using namespace hku::ipc;

/**
 * @defgroup test_kdata_shm_real test_kdata_shm_real
 * @ingroup test_hikyuu_real_data
 * @{
 */

namespace {

// 逐字段比对两条 K 线记录（KRecord 全字段）
bool krecordMatch(const KRecord& a, const KRecord& b) {
    return a.datetime == b.datetime && a.openPrice == b.openPrice && a.highPrice == b.highPrice &&
           a.lowPrice == b.lowPrice && a.closePrice == b.closePrice &&
           a.transAmount == b.transAmount && a.transCount == b.transCount;
}

}  // namespace

/**
 * @par 检测点: 真实数据下 K 线共享内存快照与 IPC 回退源（Stock::getKRecordList 等）的同源一致性
 */
TEST_CASE("test_KDataShm_real") {
    StockManager& sm = StockManager::instance();
    sm.waitDataReady();

    const std::string prefix = "hkukshm";
    KDataShmPublisher publisher(prefix);
    std::string name = publisher.publish(20260904);
    if (name.empty()) {
        MESSAGE("KData 共享内存快照发布失败（可能内存不足），跳过 KData 快照测试");
        return;
    }

    KDataShmReader reader;
    REQUIRE(reader.open(name));
    CHECK_EQ(reader.epoch(), 20260904);
    if (!reader.coversKType(KQuery::DAY)) {
        MESSAGE("快照未覆盖 DAY 类型（未预加载 day），跳过 KData 比对");
        return;
    }
    std::cout << "KData 快照覆盖证券条目数: " << reader.coveredCount() << std::endl;

    // 逐证券抽样比对：SHM 读端 == Stock 的 IPC 回退源（getCount / getKRecordList）。
    // 显式 int64_t 避免 Null<int64_t> 隐式转换选中 KQuery 的日期重载。
    const KQuery query((int64_t)0, (int64_t)Null<int64_t>(), KQuery::DAY);
    const size_t sample_limit = 200;
    size_t checked = 0, count_mismatch = 0, content_mismatch = 0;
    std::string sample_mc;
    for (const auto& stk : sm.getStockList(nullptr)) {
        if (stk.getKDataBufferSize(KQuery::DAY) == 0) {
            continue;  // 未预加载 day 缓冲的证券不在快照中
        }
        const std::string mc = stk.market_code();

        // 条数：SHM tryGetCount == Stock::getCount（IPC KDATA_COUNT 源）
        size_t shm_count = 0;
        CHECK(reader.tryGetCount(mc, KQuery::DAY, shm_count));
        if (shm_count != stk.getCount(KQuery::DAY)) {
            count_mismatch++;
            continue;
        }

        // 全量记录：SHM tryGetKRecordList == Stock::getKRecordList（IPC KDATA_GET_KRECORD_LIST 源）
        KRecordList ipc_ks = stk.getKRecordList(query);
        KRecordList shm_ks;
        CHECK(reader.tryGetKRecordList(mc, query, shm_ks));
        if (shm_ks.size() != ipc_ks.size()) {
            content_mismatch++;
            continue;
        }
        bool bad = false;
        for (size_t i = 0; i < shm_ks.size() && !bad; i++) {
            if (!krecordMatch(shm_ks[i], ipc_ks[i])) {
                bad = true;
            }
        }
        if (bad) {
            content_mismatch++;
            continue;
        }
        if (sample_mc.empty() && shm_ks.size() > 3) {
            sample_mc = mc;
        }
        checked++;
        if (checked >= sample_limit) {
            break;
        }
    }
    CHECK_EQ(count_mismatch, 0);
    CHECK_EQ(content_mismatch, 0);
    std::cout << "KData 快照逐证券比对(抽样上限 " << sample_limit << "): checked=" << checked
              << std::endl;

    // 日期区间与索引区间：SHM == Stock 的 IPC 回退源（经缓冲日期索引，语义一致）
    if (!sample_mc.empty()) {
        Stock sample_stk = sm.getStock(sample_mc);
        auto full = sample_stk.getKRecordList(query);
        REQUIRE_GT(full.size(), 3);
        Datetime mid_date = full[full.size() / 2].datetime;
        KQuery qbd = KQueryByDate(mid_date, Null<Datetime>(), KQuery::DAY);

        KRecordList shm_by_date, ipc_by_date;
        CHECK(reader.tryGetKRecordList(sample_mc, qbd, shm_by_date));
        ipc_by_date = sample_stk.getKRecordList(qbd);
        REQUIRE_EQ(shm_by_date.size(), ipc_by_date.size());
        if (!shm_by_date.empty()) {
            CHECK(krecordMatch(shm_by_date.front(), ipc_by_date.front()));
            CHECK(krecordMatch(shm_by_date.back(), ipc_by_date.back()));
        }

        // 索引区间：SHM tryGetIndexRangeByDate == Stock::getIndexRange（IPC KDATA_INDEX_RANGE_BY_DATE 源）
        size_t shm_start = 0, shm_end = 0, ipc_start = 0, ipc_end = 0;
        CHECK(reader.tryGetIndexRangeByDate(sample_mc, qbd, shm_start, shm_end));
        CHECK(sample_stk.getIndexRange(qbd, ipc_start, ipc_end));
        CHECK_EQ(shm_start, ipc_start);
        CHECK_EQ(shm_end, ipc_end);
        std::cout << "KData 日期区间 SHM==IPC 源: probe=" << sample_mc << " 索引区间=[" << shm_start
                  << "," << shm_end << ") 记录数=" << shm_by_date.size() << std::endl;
    } else {
        std::cout << "无记录数 >3 的抽样证券，跳过日期/索引区间比对" << std::endl;
    }

    // 未覆盖证券返回 false（由上层回退 IPC/本地驱动）
    size_t dummy_count = 0;
    CHECK_FALSE(reader.tryGetCount("SH999999", KQuery::DAY, dummy_count));

    reader.close();
    CHECK_FALSE(reader.valid());
}

/** @} */

#else

TEST_CASE("test_KDataShm_real") {
    MESSAGE("HKU_ENABLE_NODE 未启用，跳过 K 线共享内存快照测试");
}

#endif  // HKU_ENABLE_NODE
