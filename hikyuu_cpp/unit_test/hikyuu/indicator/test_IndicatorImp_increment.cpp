/*
 * test_IndicatorImp_increment.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2026-10-19
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/Indicator.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_IndicatorImp_increment test_indicator_IndicatorImp_increment
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

namespace {
// Drive an indicator with a first (old) context and then extend/replace it with a second (new)
// context so the framework takes the incremental path, then compare bar by bar with a plain full
// calculation on the new context.
template <typename Factory>
void check_increment_equivalence(const Factory& make, const Stock& stock, const KQuery& old_q,
                                 const KQuery& new_q, size_t expect_result_num) {
    KData k_new = stock.getKData(new_q);
    KData k_old = stock.getKData(old_q);

    Indicator expect = make();
    expect.setContext(k_new);

    Indicator got = make();
    got.setContext(k_old);
    got.setContext(k_new);

    CHECK_EQ(got.size(), expect.size());
    CHECK_EQ(got.getResultNumber(), expect_result_num);

    for (size_t r = 0; r < expect_result_num; ++r) {
        for (size_t i = 0; i < expect.size(); ++i) {
            auto a = got.get(i, r);
            auto b = expect.get(i, r);
            if (std::isnan(a) && std::isnan(b)) {
                continue;
            }
            CHECK(a == doctest::Approx(b));
        }
    }
}
}  // namespace

/**
 * @par Test points: the incremental shift must copy the overlapping old results before the
 * buffer is resized, so that a context whose front moved forward (the new window is shorter than
 * the old one, total < old_size) still shifts correctly for any value_type. This exercises the
 * resize/memmove reorder in increment_execute_leaf_or_op and increment_execute.
 */
TEST_CASE("test_increment_leaf_shrink_window") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    CHECK(!stock.isNull());

    auto ma = []() { return MA(CLOSE(), 5); };

    /** @arg old = [0,60), new = [40,70): front shifted forward, back extended, total(30) < old(60)
     */
    check_increment_equivalence(ma, stock, KQuery(0, 60), KQuery(40, 70), 1);

    /** @arg old = [0,30), new = [0,60): plain tail extension, no shrink */
    check_increment_equivalence(ma, stock, KQuery(0, 30), KQuery(0, 60), 1);

    /** @arg chained extension on top of a previous incremental update */
    {
        KData k_new = stock.getKData(KQuery(0, 60));
        Indicator expect = ma();
        expect.setContext(k_new);

        Indicator got = ma();
        got.setContext(stock.getKData(KQuery(0, 20)));
        got.setContext(stock.getKData(KQuery(0, 40)));
        got.setContext(k_new);

        CHECK_EQ(got.size(), expect.size());
        for (size_t i = 0; i < expect.size(); ++i) {
            auto a = got[i];
            auto b = expect[i];
            if (std::isnan(a) && std::isnan(b)) {
                continue;
            }
            CHECK(a == doctest::Approx(b));
        }
    }
}

/**
 * @par Test points: the binary operator incremental path (execute_add) shifts its operands
 * under the same front-shifted, shrunk window.
 */
TEST_CASE("test_increment_op_shrink_window") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    CHECK(!stock.isNull());

    auto add = []() { return CLOSE() + OPEN(); };

    check_increment_equivalence(add, stock, KQuery(0, 60), KQuery(40, 70), 1);
    check_increment_equivalence(add, stock, KQuery(0, 30), KQuery(0, 60), 1);
}

/**
 * @par Test points: the WEAVE node produces multiple result sets; the incremental path
 * reuses the existing buffers and must not write through a null data(r). This exercises the
 * result_num guard in prepareBinaryOp plus the shrink shift.
 */
TEST_CASE("test_increment_weave_shrink_window") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    CHECK(!stock.isNull());

    auto weave = []() { return WEAVE(CLOSE(), OPEN()); };

    /** @arg two result sets, front-shifted shrunk window */
    check_increment_equivalence(weave, stock, KQuery(0, 60), KQuery(40, 70), 2);

    /** @arg two result sets, plain tail extension */
    check_increment_equivalence(weave, stock, KQuery(0, 30), KQuery(0, 60), 2);
}

/** @} */
