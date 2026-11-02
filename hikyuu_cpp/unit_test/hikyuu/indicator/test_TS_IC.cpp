/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-11-2
 *      Author: fasiondog
 */

#include "../test_config.h"

#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/TS_IC.h>

using namespace hku;

/**
 * @defgroup test_indicator_TS_IC test_indicator_TS_IC
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test point: an invalid n or ic_n throws */
TEST_CASE("test_TS_IC_invalid_param") {
    /** @arg n < 2 (a correlation window needs at least two pairs) */
    CHECK_THROWS_AS(TS_IC(1, 1), std::exception);
    /** @arg ic_n < 1 */
    CHECK_THROWS_AS(TS_IC(30, 0), std::exception);
}

/** @par Test point: the normal spearman run and its basic invariants */
TEST_CASE("test_TS_IC_base") {
    StockManager& sm = StockManager::instance();
    KData ref_k = sm["sh000001"].getKData(KQuery(-100));

    // factor with discard 0, ic_n = 1, n = 2 -> discard = max(0+1, 1) + 2 - 1 = 2
    Indicator result = TS_IC(CLOSE(), 2, 1)(ref_k);
    CHECK_EQ(result.name(), "TS_IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), 2);

    /** @arg the values before the discard are Null */
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));

    /** @arg every computed value is a valid correlation in [-1, 1] */
    for (size_t i = result.discard(); i < result.size(); ++i) {
        if (std::isfinite(result[i])) {
            CHECK_UNARY(result[i] >= -1.0 && result[i] <= 1.0);
        }
    }
}

/** @par Test point: a constant factor gives zero variance -> pearson Null, spearman 0 */
TEST_CASE("test_TS_IC_constant_factor") {
    StockManager& sm = StockManager::instance();
    KData ref_k = sm["sh000001"].getKData(KQuery(-100));

    Indicator zero = CLOSE() - CLOSE();  // all-zero factor

    /** @arg pearson on a zero-variance factor yields Null everywhere */
    Indicator pearson = TS_IC(zero, 2, 1, false)(ref_k);
    CHECK_EQ(pearson.discard(), pearson.size());

    /** @arg spearman on tied ranks yields 0 for every computed window */
    Indicator spearman = TS_IC(zero, 2, 1, true)(ref_k);
    REQUIRE_LT(spearman.discard(), spearman.size());  // the loop below must actually run
    for (size_t i = spearman.discard(); i < spearman.size(); ++i) {
        CHECK_EQ(spearman[i], doctest::Approx(0.0));
    }
}

/** @par Test point: the correlation window larger than the data yields an all-Null result */
TEST_CASE("test_TS_IC_window_too_large") {
    StockManager& sm = StockManager::instance();
    KData ref_k = sm["sh000001"].getKData(KQuery(-10));

    Indicator result = TS_IC(MA(CLOSE()), 100, 1)(ref_k);
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), result.size());
}

/** @par Test point: empty input */
TEST_CASE("test_TS_IC_empty") {
    StockManager& sm = StockManager::instance();
    KData ref_k = sm["sh000001"].getKData(KQuery(-1));

    Indicator result = TS_IC(MA(CLOSE()), 2, 1)(ref_k);
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), result.size());
}

/** @par Test point: strict mode re-indexes by the factor date (shift left by ic_n) and blanks the
 * tail */
TEST_CASE("test_TS_IC_strict") {
    StockManager& sm = StockManager::instance();
    KData ref_k = sm["sh000001"].getKData(KQuery(-100));

    int ic_n = 3;
    int n = 2;
    Indicator normal = TS_IC(MA(CLOSE()), n, ic_n, true, false)(ref_k);
    Indicator strict = TS_IC(MA(CLOSE()), n, ic_n, true, true)(ref_k);

    /** @arg strict[i] equals normal[i + ic_n] where the strict value is finite */
    for (size_t i = 0; i + ic_n < ref_k.size(); ++i) {
        if (std::isfinite(strict[i])) {
            CHECK_EQ(strict[i], doctest::Approx(normal[i + ic_n]));
        }
    }

    /** @arg the strict discard is the normal one shifted left by ic_n (the first masked values
     *  are not hidden) */
    CHECK_EQ(strict.discard(), normal.discard() - ic_n);

    /** @arg the trailing ic_n values are Null in strict mode */
    size_t total = strict.size();
    for (size_t i = total - ic_n; i < total; ++i) {
        CHECK_UNARY(std::isnan(strict[i]));
    }
}

/** @par Test point: the incremental path produces the same result as the full path */
TEST_CASE("test_TS_IC_increment") {
    Stock stk = getStock("sh600000");
    KData k1 = stk.getKData(KQuery(100, 200));
    KData k2 = stk.getKData(KQuery(100, 300));

    auto ts_ic = TS_IC(CLOSE(), 5, 1);
    auto result_full = ts_ic(k2);
    auto result_inc = ts_ic(k1);

    result_inc.setContext(k2);

    check_indicator(result_inc, result_full);
}

/** @par Test point: the incremental path keeps the strict shifted layout (ic_n > 1 covered) */
TEST_CASE("test_TS_IC_increment_strict") {
    Stock stk = getStock("sh600000");
    KData k1 = stk.getKData(KQuery(100, 200));
    KData k2 = stk.getKData(KQuery(100, 300));

    auto ts_ic = TS_IC(CLOSE(), 5, 2, true, true);
    auto result_full = ts_ic(k2);
    auto result_inc = ts_ic(k1);

    result_inc.setContext(k2);

    check_indicator(result_inc, result_full);

    /** @arg the trailing ic_n values stay Null after the incremental extension */
    size_t total = result_inc.size();
    for (size_t i = total - 2; i < total; ++i) {
        CHECK_UNARY(std::isnan(result_inc[i]));
    }
}

/** @par Test point: the offset incremental path carries the old head and refills the realized tail
 */
TEST_CASE("test_TS_IC_increment_offset") {
    Stock stk = getStock("sh600000");
    // the overlap is shifted by 50 bars, so the memmove path (copy_start_pos > 0) is exercised
    // instead of a plain prefix extension
    KData k1 = stk.getKData(KQuery(100, 200));
    KData k3 = stk.getKData(KQuery(150, 300));

    int n = 5;
    int ic_n = 2;
    Indicator base = TS_IC(CLOSE(), n, ic_n, true, true)(k1);
    Indicator full = TS_IC(CLOSE(), n, ic_n, true, true)(k3);

    auto ts_ic = TS_IC(CLOSE(), n, ic_n, true, true);
    Indicator inc = ts_ic(k1);
    inc.setContext(k3);

    /** @arg the head keeps the values carried over from k1, unreachable by a from-scratch run on
     *  k3 whose context starts at bar 150 */
    CHECK_UNARY(std::isnan(full[0]));
    for (size_t i = 0; i < 47; ++i) {
        CHECK_EQ(inc[i], doctest::Approx(base[i + 50]));
    }

    /** @arg the overlapping region agrees with the from-scratch computation on k3 */
    for (size_t i = full.discard(); i + ic_n < inc.size(); ++i) {
        if (std::isfinite(full[i])) {
            CHECK_EQ(inc[i], doctest::Approx(full[i]));
        }
    }

    /** @arg the trailing ic_n values stay Null */
    size_t total = inc.size();
    for (size_t i = total - ic_n; i < total; ++i) {
        CHECK_UNARY(std::isnan(inc[i]));
    }
}

/** @par Test point: a NaN pair inside the window: pearson propagates Null, spearman excludes it */
TEST_CASE("test_TS_IC_nan_pair") {
    StockManager& sm = StockManager::instance();
    KData ref_k = sm["sh000001"].getKData(KQuery(-100));

    PriceList values(ref_k.size(), 0.0);
    for (size_t i = 0; i < ref_k.size(); ++i) {
        values[i] = (price_t)i;
    }
    size_t mid = 50;
    values[mid] = Null<price_t>();
    Indicator factor = PRICELIST(values, 0);

    // f[i] = factor[i - 1], so the NaN pair enters at i = mid + 1, hitting the windows that end
    // in [mid + 1, mid + 3]
    Indicator pearson = TS_IC(factor, 3, 1, false)(ref_k);
    Indicator spearman = TS_IC(factor, 3, 1, true)(ref_k);

    /** @arg the pearson windows containing the NaN pair yield Null (no filtering) */
    for (size_t i = mid + 1; i <= mid + 3; ++i) {
        CHECK_UNARY(std::isnan(pearson[i]));
    }

    /** @arg the windows without the NaN pair stay finite for pearson */
    CHECK_UNARY(std::isfinite(pearson[mid - 1]));
    CHECK_UNARY(std::isfinite(pearson[mid]));

    /** @arg the same spearman windows stay finite (the NaN pair is excluded) */
    for (size_t i = mid + 1; i <= mid + 3; ++i) {
        CHECK_UNARY(std::isfinite(spearman[i]));
    }
}

/** @par Test point: the input factor's own KData is used when the node carries no context */
TEST_CASE("test_TS_IC_input_context_fallback") {
    StockManager& sm = StockManager::instance();
    KData ref_k = sm["sh000001"].getKData(KQuery(-100));

    Indicator factor = CLOSE()(ref_k);  // carries ref_k as its own context
    Indicator fallback = TS_IC(factor, 5, 1);
    Indicator bound = TS_IC(factor, 5, 1)(ref_k);

    /** @arg the fallback run yields a usable result with the same discard as the bound run */
    CHECK_EQ(fallback.size(), ref_k.size());
    CHECK_EQ(fallback.discard(), bound.discard());

    /** @arg the values match the explicitly bound run point by point */
    for (size_t i = fallback.discard(); i < fallback.size(); ++i) {
        if (std::isfinite(bound[i])) {
            CHECK_EQ(fallback[i], doctest::Approx(bound[i]));
        }
    }

    /** @arg a data-only factor with no KData anywhere still yields all Null */
    Indicator data_only = TS_IC(PRICELIST(PriceList(ref_k.size(), 1.0), 0), 5, 1);
    CHECK_EQ(data_only.size(), ref_k.size());
    CHECK_EQ(data_only.discard(), data_only.size());
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test point: serialize / deserialize the indicator */
TEST_CASE("test_TS_IC_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TS_IC.xml";

    KData ref_k = sm["sh000001"].getKData(KQuery(-100));
    Indicator x1 = TS_IC(MA(CLOSE()), 10, 1)(ref_k);

    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    Indicator x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    CHECK_EQ(x1.name(), x2.name());
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    for (size_t i = 0; i < x1.size(); ++i) {
        if (std::isnan(x1[i])) {
            CHECK_UNARY(std::isnan(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
