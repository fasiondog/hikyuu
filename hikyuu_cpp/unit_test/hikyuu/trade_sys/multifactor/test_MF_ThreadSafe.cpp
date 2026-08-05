/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-31
 *      Author: woleigegg
 *
 *  MultiFactorBase 线程安全惰性计算的白盒测试。
 *
 *   - T2 并发首次访问：barrier 齐射 32 线程混合调用 getter，实际构建恰好一次；
 *   - T3 失败重试：首次 _calculate 抛异常且恶意写坏基类派生状态，
 *     验证原异常传播、基类半成品被清理、第二次调用成功且无脏数据；
 *   - T4 reset：重置后重新计算，无旧结果残留；
 *   - T6 嵌套：一个 MF 的 _calculate 触发另一个 MF 的惰性计算，无死锁；
 *   - T7 序列化：load 后状态未发布，重新计算得到完整结果。
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/utilities/thread/algorithm.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_sys/multifactor/MultiFactorBase.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_EqualWeight.h>

using namespace hku;

/**
 * @defgroup test_MF_ThreadSafe test_MF_ThreadSafe
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** 简易 spin barrier：不依赖 C++20 latch/barrier，保证各线程同时起跑 */
class SpinBarrier {
public:
    explicit SpinBarrier(size_t n) : m_target(n) {}

    void wait() {
        m_arrived.fetch_add(1, std::memory_order_acq_rel);
        while (m_arrived.load(std::memory_order_acquire) < m_target) {
            std::this_thread::yield();
        }
    }

private:
    std::atomic<size_t> m_arrived{0};
    size_t m_target;
};

//-----------------------------------------------------------------------------
// 计数 MF：进入 _calculate 即计数并延迟，放大并发窗口
//-----------------------------------------------------------------------------

class TestCountingMF : public MultiFactorBase {
    MULTIFACTOR_IMP(TestCountingMF)

public:
    static std::atomic<size_t> s_count;

    TestCountingMF() : MultiFactorBase("TestCountingMF") {}
};

IndicatorList TestCountingMF::_calculate(const vector<IndicatorList>& all_stk_inds) {
    s_count.fetch_add(1, std::memory_order_relaxed);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    // 等权合成（与 EqualWeightMultiFactor 相同逻辑）
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_factorset.size();
    return global_parallel_for_index(0, stk_count, [&](size_t si) {
        vector<price_t> sumByDate(days_total);
        vector<size_t> countByDate(days_total);
        const auto& curStkInds = all_stk_inds[si];
        for (size_t ii = 0; ii < ind_count; ii++) {
            const auto* curInd = curStkInds[ii].data();
            for (size_t di = 0; di < days_total; di++) {
                auto value = curInd[di];
                if (!std::isnan(value)) {
                    sumByDate[di] += value;
                    countByDate[di] += 1;
                }
            }
        }
        for (size_t di = 0; di < days_total; di++) {
            sumByDate[di] =
              (countByDate[di] == 0) ? Null<price_t>() : sumByDate[di] / countByDate[di];
        }
        Indicator ret = PRICELIST(sumByDate);
        ret.updateDiscard(true);
        ret.name("IC");
        return ret;
    });
}

std::atomic<size_t> TestCountingMF::s_count{0};

/** 构造并配置一个计数 MF（两个因子确保走 _calculate 分支，而非单因子直通） */
static std::shared_ptr<TestCountingMF> makeCountingMF() {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-50);

    auto mf = std::make_shared<TestCountingMF>();
    mf->setStockList(stks);
    mf->setQuery(query);
    mf->setRefStock(ref_stk);
    IndicatorList inds{MA(CLOSE()), AMA(CLOSE())};
    mf->setRefFactorSet(FactorSet(inds, KQuery::DAY));
    mf->setParam<bool>("save_all_factors", true);
    return mf;
}

/** @par 并发首次访问：32 线程 barrier 齐射混合调用 getter，实际构建恰好一次 */
TEST_CASE("test_MF_thread_safe_concurrent_first_access") {
    TestCountingMF::s_count = 0;
    auto mf = makeCountingMF();

    const size_t N = 32;
    SpinBarrier barrier(N);
    std::atomic<size_t> ok_count{0};
    std::vector<std::thread> threads;
    threads.reserve(N);

    for (size_t i = 0; i < N; i++) {
        threads.emplace_back([mf, &barrier, &ok_count]() {
            barrier.wait();
            const auto& dates = mf->getDatetimeList();
            if (dates.empty()) {
                return;
            }
            auto mid = dates[dates.size() / 2];
            auto cross = mf->getScores(mid);
            const auto& all = mf->getAllScores();
            if (all.size() == dates.size() && !cross.empty()) {
                ok_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    // 完整计算只执行一次
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 1u);
    // 所有线程都拿到一致完整结果
    CHECK_EQ(ok_count.load(std::memory_order_relaxed), N);
}

//-----------------------------------------------------------------------------
// FailOnce MF：首次调用写坏基类派生状态后抛异常，第二次正常
//-----------------------------------------------------------------------------

class TestFailOnceMF : public MultiFactorBase {
    MULTIFACTOR_IMP(TestFailOnceMF)

public:
    static std::atomic<size_t> s_calls;
    static std::atomic<bool> s_saw_dirty;
    static Datetime s_dirty_date;

    TestFailOnceMF() : MultiFactorBase("TestFailOnceMF") {}
};

IndicatorList TestFailOnceMF::_calculate(const vector<IndicatorList>& all_stk_inds) {
    size_t call = s_calls.fetch_add(1, std::memory_order_relaxed);
    if (call == 0) {
        // 恶意写坏基类 protected 派生状态，模拟自定义子类遗留半成品；
        // 若 clearCalculatedData 未生效，第二次计算会看到这些脏数据
        m_date_index[s_dirty_date] = 999;
        m_ic = PRICELIST(PriceList{12345.0});
        throw std::runtime_error("simulated first-call failure");
    }

    // 第二次计算开始时检查：第一次遗留的脏状态必须已被清理
    if (m_date_index.find(s_dirty_date) != m_date_index.end()) {
        s_saw_dirty.store(true, std::memory_order_relaxed);
    }

    // 等权合成
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_factorset.size();
    return global_parallel_for_index(0, stk_count, [&](size_t si) {
        vector<price_t> sumByDate(days_total);
        vector<size_t> countByDate(days_total);
        const auto& curStkInds = all_stk_inds[si];
        for (size_t ii = 0; ii < ind_count; ii++) {
            const auto* curInd = curStkInds[ii].data();
            for (size_t di = 0; di < days_total; di++) {
                auto value = curInd[di];
                if (!std::isnan(value)) {
                    sumByDate[di] += value;
                    countByDate[di] += 1;
                }
            }
        }
        for (size_t di = 0; di < days_total; di++) {
            sumByDate[di] =
              (countByDate[di] == 0) ? Null<price_t>() : sumByDate[di] / countByDate[di];
        }
        Indicator ret = PRICELIST(sumByDate);
        ret.updateDiscard(true);
        ret.name("IC");
        return ret;
    });
}

std::atomic<size_t> TestFailOnceMF::s_calls{0};
std::atomic<bool> TestFailOnceMF::s_saw_dirty{false};
Datetime TestFailOnceMF::s_dirty_date = Datetime(20111204);

/** @par 失败重试：首次异常向上传播，半成品被清理，第二次调用成功 */
TEST_CASE("test_MF_failed_first_call_clean_retry") {
    TestFailOnceMF::s_calls = 0;
    TestFailOnceMF::s_saw_dirty = false;

    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-50);

    auto mf = std::make_shared<TestFailOnceMF>();
    mf->setStockList(stks);
    mf->setQuery(query);
    mf->setRefStock(ref_stk);
    IndicatorList inds{MA(CLOSE()), AMA(CLOSE())};
    mf->setRefFactorSet(FactorSet(inds, KQuery::DAY));
    mf->setParam<bool>("save_all_factors", true);

    // 第一次调用：原异常向上传播，不发布 Ready
    REQUIRE_THROWS_AS(mf->getDatetimeList(), std::runtime_error);
    CHECK_EQ(TestFailOnceMF::s_calls.load(std::memory_order_relaxed), 1u);

    // 第二次调用：基于干净状态重新计算成功
    const auto& dates = mf->getDatetimeList();
    CHECK_UNARY_FALSE(dates.empty());
    CHECK_EQ(TestFailOnceMF::s_calls.load(std::memory_order_relaxed), 2u);
    // 第一次遗留的脏派生状态必须已被 clearCalculatedData 清除
    CHECK_UNARY_FALSE(TestFailOnceMF::s_saw_dirty.load(std::memory_order_relaxed));

    // 结果完整可用
    auto cross = mf->getScores(dates[dates.size() / 2]);
    CHECK_UNARY_FALSE(cross.empty());
}

/** @par reset 后重新计算：无旧结果残留，结果与首次一致 */
TEST_CASE("test_MF_reset_recalculates_clean") {
    TestCountingMF::s_count = 0;
    auto mf = makeCountingMF();

    const auto& dates1 = mf->getDatetimeList();
    auto cross1 = mf->getScores(dates1[dates1.size() / 2]);
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 1u);

    mf->reset();
    // reset 本身不触发计算
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 1u);

    // 再次访问触发重算，结果一致
    const auto& dates2 = mf->getDatetimeList();
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 2u);
    CHECK_EQ(dates1, dates2);

    auto cross2 = mf->getScores(dates2[dates2.size() / 2]);
    CHECK_EQ(cross1.size(), cross2.size());
    for (size_t i = 0; i < cross1.size(); i++) {
        CHECK_EQ(cross1[i].stock, cross2[i].stock);
        CHECK_EQ(cross1[i].value, doctest::Approx(cross2[i].value).epsilon(1e-9));
    }
}

//-----------------------------------------------------------------------------
// 嵌套：一个 MF 的 _calculate 触发另一个 MF 的惰性计算
//-----------------------------------------------------------------------------

class TestNestedMFB : public MultiFactorBase {
    MULTIFACTOR_IMP(TestNestedMFB)

public:
    TestNestedMFB() : MultiFactorBase("TestNestedMFB") {}
};

IndicatorList TestNestedMFB::_calculate(const vector<IndicatorList>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    return global_parallel_for_index(0, stk_count, [&](size_t si) {
        vector<price_t> sumByDate(days_total);
        const auto& curStkInds = all_stk_inds[si];
        for (size_t di = 0; di < days_total; di++) {
            sumByDate[di] += curStkInds[0][di];
        }
        Indicator ret = PRICELIST(sumByDate);
        ret.updateDiscard(true);
        ret.name("IC");
        return ret;
    });
}

class TestNestedMFA : public MultiFactorBase {
    MULTIFACTOR_IMP(TestNestedMFA)

public:
    TestNestedMFA() : MultiFactorBase("TestNestedMFA") {}

    void setB(const std::shared_ptr<TestNestedMFB>& b) {
        m_b = b;
    }

private:
    std::shared_ptr<TestNestedMFB> m_b;
};

IndicatorList TestNestedMFA::_calculate(const vector<IndicatorList>& all_stk_inds) {
    // 在 A 的计算过程中触发 B 的惰性计算：不同实例不同 mutex，无环
    if (m_b && !m_ref_dates.empty()) {
        (void)m_b->getScores(m_ref_dates[m_ref_dates.size() / 2]);
    }
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    return global_parallel_for_index(0, stk_count, [&](size_t si) {
        vector<price_t> sumByDate(days_total);
        const auto& curStkInds = all_stk_inds[si];
        for (size_t di = 0; di < days_total; di++) {
            sumByDate[di] += curStkInds[0][di];
        }
        Indicator ret = PRICELIST(sumByDate);
        ret.updateDiscard(true);
        ret.name("IC");
        return ret;
    });
}

/** @par 嵌套计算：A 的 _calculate 内触发 B 的惰性计算，并发触发无死锁 */
TEST_CASE("test_MF_nested_calculate_no_deadlock") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-50);
    IndicatorList inds{MA(CLOSE())};

    auto b = std::make_shared<TestNestedMFB>();
    b->setStockList(stks);
    b->setQuery(query);
    b->setRefStock(ref_stk);
    b->setRefFactorSet(FactorSet(inds, KQuery::DAY));

    auto a = std::make_shared<TestNestedMFA>();
    a->setStockList(stks);
    a->setQuery(query);
    a->setRefStock(ref_stk);
    a->setRefFactorSet(FactorSet(inds, KQuery::DAY));
    a->setB(b);

    // 两个线程同时触发 A 和 B 的惰性计算
    std::atomic<bool> go{false};
    std::vector<std::thread> threads;
    threads.emplace_back([&]() {
        while (!go.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        (void)a->getDatetimeList();
        (void)a->getScores(a->getDatetimeList().back());
    });
    threads.emplace_back([&]() {
        while (!go.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        (void)b->getDatetimeList();
        (void)b->getScores(b->getDatetimeList().back());
    });

    go.store(true, std::memory_order_release);
    for (auto& t : threads) {
        t.join();
    }

    // 无死锁即通过；结果完整可用
    CHECK_UNARY_FALSE(a->getDatetimeList().empty());
    CHECK_UNARY_FALSE(b->getDatetimeList().empty());
    CHECK_UNARY_FALSE(a->getScores(a->getDatetimeList().back()).empty());
}

/** @par clone 独立状态：原对象 Ready 后 clone 不共享计算状态
 *  两个线程同时访问原对象（已 Ready，不重算）与克隆对象（首次访问触发重算），
 *  互不干扰且结果相等。
 */
TEST_CASE("test_MF_clone_independent_state") {
    TestCountingMF::s_count = 0;
    auto mfA = makeCountingMF();

    // A 计算完成
    (void)mfA->getDatetimeList();
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 1u);

    // clone 后强制重算：原对象不重算，仅克隆对象触发
    auto mfB = mfA->clone();

    std::atomic<bool> go{false};
    std::vector<std::thread> threads;
    threads.emplace_back([&]() {
        while (!go.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        (void)mfA->getDatetimeList();
        (void)mfA->getScores(mfA->getDatetimeList().back());
    });
    threads.emplace_back([&]() {
        while (!go.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        (void)mfB->getDatetimeList();
        (void)mfB->getScores(mfB->getDatetimeList().back());
    });

    go.store(true, std::memory_order_release);
    for (auto& t : threads) {
        t.join();
    }

    // 只有克隆对象触发了一次新的完整计算
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 2u);

    // 两对象结果一致
    const auto& dates_a = mfA->getDatetimeList();
    const auto& dates_b = mfB->getDatetimeList();
    CHECK_EQ(dates_a, dates_b);
    auto cross_a = mfA->getScores(dates_a[dates_a.size() / 2]);
    auto cross_b = mfB->getScores(dates_b[dates_b.size() / 2]);
    CHECK_EQ(cross_a.size(), cross_b.size());
    for (size_t i = 0; i < cross_a.size(); i++) {
        CHECK_EQ(cross_a[i].stock, cross_b[i].stock);
        CHECK_EQ(cross_a[i].value, doctest::Approx(cross_b[i].value).epsilon(1e-9));
    }
}

/** @par 序列化 load 后状态未发布：重新计算得到完整结果 */
#if HKU_SUPPORT_SERIALIZATION
TEST_CASE("test_MF_serialization_load_recalculates") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-50);
    IndicatorList inds{MA(CLOSE()), AMA(CLOSE())};

    auto mf1 = MF_EqualWeight(inds, stks, query, ref_stk);
    mf1->setParam<bool>("save_all_factors", true);
    (void)mf1->getDatetimeList();  // 触发计算

    string filename(sm.tmpdir());
    filename += "/MF_thread_safe_export.xml";
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(mf1);
    }

    MFPtr mf2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(mf2);
    }

    // load 后 m_calculated 必须为 false：访问触发重新计算而非复用旧状态
    const auto& dates = mf2->getDatetimeList();
    CHECK_UNARY_FALSE(dates.empty());
    auto cross = mf2->getScores(dates[dates.size() / 2]);
    CHECK_UNARY_FALSE(cross.empty());
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
