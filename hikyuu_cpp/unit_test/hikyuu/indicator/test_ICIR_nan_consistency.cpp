/*
 * test_ICIR_nan_consistency.cpp
 *
 * 验证 MA(ic,n) / STDEV(ic,n) 组合在含散布 NaN 时的语义一致性。
 * ICIR = MA / STDEV，分子分母必须基于完全相同的有效样本集，
 * 且任一为 NaN 时 ICIR 必须静默输出 NaN（不抛浮点除零异常）。
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

/** @par 检测点：MA/STDEV 组合 ICIR 在含 NaN 时的一致性 */
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
    // ICIR = MA / STDEV（ICIR.h 内部即此组合）
    Indicator ir = ma / std;

    CHECK_EQ(ir.size(), ma.size());
    CHECK_EQ(ir.discard(), ma.discard());

    for (size_t i = 0; i < ir.size(); ++i) {
        if (std::isnan(ma[i]) || std::isnan(std[i]) || std[i] == 0.0) {
            // 任一为 NaN 或 std=0 时，ICIR 必须为 NaN（不抛异常，不产生 Inf）
            CHECK_UNARY(std::isnan(ir[i]));
        } else {
            // 两者均有效时，ICIR = ma/std
            CHECK_EQ(ir[i], doctest::Approx(ma[i] / std[i]).epsilon(0.0001));
        }
    }
}

/** @par 检测点：count=1 时 std=NaN，ICIR 静默 NaN（不除零崩溃） */
TEST_CASE("test_ICIR_count1_silent_nan") {
    PriceList d;
    for (int i = 0; i < 7; ++i) {
        d.push_back(Null<price_t>());
    }
    d[3] = 5.0;  // 唯一有效值

    Indicator ic = PRICELIST(d);
    Indicator ma = MA(ic, 4);
    Indicator std = STDEV(ic, 4);
    Indicator ir = ma / std;

    // MA 有值（5.0），但 std=NaN（count=1），ICIR 必须静默 NaN
    for (size_t i = ir.discard(); i < ir.size(); ++i) {
        CHECK_UNARY(std::isnan(std[i]));
        // 不允许 Inf 或异常
        CHECK_UNARY(std::isnan(ir[i]));
    }
}

/** @} */
