/*
 * test_ICIR_nan_consistency.cpp
 *
 * Verify the semantic consistency of the MA(ic,n) / STDEV(ic,n) combination with scattered NaN.
 * ICIR = MA / STDEV: the numerator and the denominator must be based on exactly the same valid
 * sample set, and when either of them is NaN, ICIR must silently output NaN.
 */

#include "../test_config.h"
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/STDEV.h>

using namespace hku;

/**
 * @defgroup test_indicator_ICIR_nan test_indicator_ICIR_nan
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test point: the consistency of the MA/STDEV ICIR combination with NaN */
TEST_CASE("test_ICIR_nan_consistency") {
    PriceList d;
    d.push_back(1.0);
    d.push_back(2.0);
    d.push_back(3.0);
    d.push_back(Null<price_t>());  // NaN
    d.push_back(5.0);
    d.push_back(6.0);
    d.push_back(7.0);
    d.push_back(8.0);

    Indicator ic = PRICELIST(d);
    int n = 4;
    Indicator ma = MA(ic, n);
    Indicator std = STDEV(ic, n);
    // ICIR = MA / STDEV (this is the combination inside ICIR.h)
    Indicator ir = ma / std;

    CHECK_EQ(ir.size(), ma.size());
    CHECK_EQ(ir.discard(), ma.discard());

    for (size_t i = 0; i < ir.size(); ++i) {
        if (std::isnan(ma[i]) || std::isnan(std[i]) || std[i] == 0.0) {
            // When either is NaN or std=0, ICIR must be NaN (no exception, no Inf)
            CHECK_UNARY(std::isnan(ir[i]));
        } else {
            // When both are valid, ICIR = ma/std
            CHECK_EQ(ir[i], doctest::Approx(ma[i] / std[i]).epsilon(0.0001));
        }
    }
}

/** @par Test point: with count=1 std=NaN and ICIR is silently NaN (no division by 0 crash) */
TEST_CASE("test_ICIR_count1_silent_nan") {
    PriceList d;
    for (int i = 0; i < 7; ++i) {
        d.push_back(Null<price_t>());
    }
    d[3] = 5.0;  // The only valid value

    Indicator ic = PRICELIST(d);
    Indicator ma = MA(ic, 4);
    Indicator std = STDEV(ic, 4);
    Indicator ir = ma / std;

    // MA has a value (5.0) but std=NaN (count=1), so ICIR must be silently NaN
    for (size_t i = ir.discard(); i < ir.size(); ++i) {
        CHECK_UNARY(std::isnan(std[i]));
        // Inf or an exception is not allowed
        CHECK_UNARY(std::isnan(ir[i]));
    }
}

/** @} */
