/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-31
 *      Author: woleigegg
 *
 *  The white box test of the thread safe lazy calculation of MultiFactorBase.
 *
 *   - T2 a concurrent first access: 32 threads released by a barrier call the getters in a mixed
 * way and the build happens exactly once;
 *   - T3 a failure retry: the first _calculate throws and maliciously corrupts the base derived
 * state; it verifies that the original exception propagates, the base half-finished product is
 * cleaned and the second call succeeds without dirty data;
 *   - T4 reset: the recalculation after the reset leaves no old result;
 *   - T6 nesting: the _calculate of one MF triggers the lazy calculation of another MF without a
 * deadlock;
 *   - T7 serialization: after load the state is not published and the recalculation is complete.
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

/** A simple spin barrier: it does not depend on the C++20 latch/barrier and all the threads start
 * together */
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
// A counting MF: it counts and delays on entering _calculate, enlarging the concurrency window
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

    // The equal weight combination (the same logic as EqualWeightMultiFactor)
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

/** Build and configure a counting MF (two factors take the _calculate branch, not the single-factor
 * pass-through) */
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

/** @par A concurrent first access: 32 threads released by a barrier call the getters in a mixed way
 * and the build happens exactly once */
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

    // The complete calculation runs once only
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 1u);
    // All the threads get a consistent complete result
    CHECK_EQ(ok_count.load(std::memory_order_relaxed), N);
}

//-----------------------------------------------------------------------------
// A FailOnce MF: the first call corrupts the base derived state and throws; the second is normal
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
        // Maliciously corrupt the base protected derived state, simulating a half-finished product
        // left by a custom subclass; if clearCalculatedData does not work, the second run sees the
        // dirty data
        m_date_index[s_dirty_date] = 999;
        m_ic = PRICELIST(PriceList{12345.0});
        throw std::runtime_error("simulated first-call failure");
    }

    // Check at the start of the second calculation: the dirty state left by the first is cleaned
    if (m_date_index.find(s_dirty_date) != m_date_index.end()) {
        s_saw_dirty.store(true, std::memory_order_relaxed);
    }

    // The equal weight combination
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

/** @par The failure retry: the first exception propagates up, the half-finished product is cleaned
 * and the second call succeeds */
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

    // The first call: the original exception propagates up and Ready is not published
    REQUIRE_THROWS_AS(mf->getDatetimeList(), std::runtime_error);
    CHECK_EQ(TestFailOnceMF::s_calls.load(std::memory_order_relaxed), 1u);

    // The second call: the recalculation based on the clean state succeeds
    const auto& dates = mf->getDatetimeList();
    CHECK_UNARY_FALSE(dates.empty());
    CHECK_EQ(TestFailOnceMF::s_calls.load(std::memory_order_relaxed), 2u);
    // The dirty derived state left by the first call must have been cleared by clearCalculatedData
    CHECK_UNARY_FALSE(TestFailOnceMF::s_saw_dirty.load(std::memory_order_relaxed));

    // The result is complete and usable
    auto cross = mf->getScores(dates[dates.size() / 2]);
    CHECK_UNARY_FALSE(cross.empty());
}

/** @par The recalculation after reset: no old result is left and it matches the first one */
TEST_CASE("test_MF_reset_recalculates_clean") {
    TestCountingMF::s_count = 0;
    auto mf = makeCountingMF();

    const auto& dates1 = mf->getDatetimeList();
    auto cross1 = mf->getScores(dates1[dates1.size() / 2]);
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 1u);

    mf->reset();
    // reset itself does not trigger the calculation
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 1u);

    // Accessing again triggers the recalculation and the result is consistent
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
// Nesting: the _calculate of one MF triggers the lazy calculation of another MF
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
    // Trigger the lazy calculation of B during the calculation of A: different mutexes, no cycle
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

/** @par The nested calculation: the lazy calculation of B is triggered inside the _calculate of A
 * and the concurrent triggering has no deadlock */
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

    // Two threads trigger the lazy calculations of A and B at the same time
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

    // No deadlock means it passes; the result is complete and usable
    CHECK_UNARY_FALSE(a->getDatetimeList().empty());
    CHECK_UNARY_FALSE(b->getDatetimeList().empty());
    CHECK_UNARY_FALSE(a->getScores(a->getDatetimeList().back()).empty());
}

/** @par The clone independent state: after the original object is Ready the clone does not share
 * the calculation state Two threads access the original object (already Ready, no recalculation)
 * and the clone (the first access triggers a recalculation) at the same time, without interfering
 * with each other and the results are equal. */
*/ TEST_CASE("test_MF_clone_independent_state") {
    TestCountingMF::s_count = 0;
    auto mfA = makeCountingMF();

    // The calculation of A is finished
    (void)mfA->getDatetimeList();
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 1u);

    // A forced recalculation after clone: the original is not recalculated, only the clone triggers
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

    // Only the clone triggered one new complete calculation
    CHECK_EQ(TestCountingMF::s_count.load(std::memory_order_relaxed), 2u);

    // The results of the two objects are consistent
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

/** @par After the serialization load the state is not published: the recalculation is complete */
#if HKU_SUPPORT_SERIALIZATION
TEST_CASE("test_MF_serialization_load_recalculates") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-50);
    IndicatorList inds{MA(CLOSE()), AMA(CLOSE())};

    auto mf1 = MF_EqualWeight(inds, stks, query, ref_stk);
    mf1->setParam<bool>("save_all_factors", true);
    (void)mf1->getDatetimeList();  // Trigger the calculation

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

    // After load m_calculated must be false: an access recalculates instead of reusing the old
    // state
    const auto& dates = mf2->getDatetimeList();
    CHECK_UNARY_FALSE(dates.empty());
    auto cross = mf2->getScores(dates[dates.size() / 2]);
    CHECK_UNARY_FALSE(cross.empty());
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
