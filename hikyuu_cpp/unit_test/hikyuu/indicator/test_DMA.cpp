/*
 * test_DMA.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created for: the regression of the out-of-bounds write of IDma::_calculate when the input is
 *  fully discarded
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/DMA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_DMA test_indicator_DMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/**
 * @par Test points
 * The input is fully discarded (discard == size, a regular reachable state because setDiscard
 * clamps to size, and most other imps guard it with "if (m_discard >= total)"). Before the fix,
 * IDma::_calculate wrote y[m_discard] unconditionally, i.e. one double past the end of the
 * result buffer.
 *
 * @note This case locks the contract (discard == size and no valid output). The out-of-bounds
 * write itself hits heap slack and is only guaranteed to be detected by a sanitizer build
 * (xmake f --leak_check=y); do not treat a green release run as proof that the write is gone.
 */
TEST_CASE("test_DMA_fully_discarded_input") {
    PriceList a{1.0, 2.0, 3.0, 4.0, 5.0};
    PriceList w{0.5, 0.5, 0.5, 0.5, 0.5};
    DatetimeList ds{Datetime(20240101150000), Datetime(20240102150000), Datetime(20240103150000),
                    Datetime(20240104150000), Datetime(20240105150000)};

    Indicator x = PRICELIST(a, ds, 5);  // fully discarded: discard clamped to size
    Indicator weights = PRICELIST(w, ds);
    CHECK_EQ(x.discard(), x.size());

    Indicator result = DMA(x, weights);
    CHECK_EQ(result.size(), 5);
    CHECK_EQ(result.discard(), 5);
    for (size_t i = 0; i < result.size(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    /** @arg The normal case is not affected by the guard */
    x = PRICELIST(a, ds);
    result = DMA(x, PRICELIST(PriceList{0.2, 0.4, 0.6, 0.8, 1.0}, ds));
    CHECK_EQ(result.size(), 5);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 1.0);
    CHECK_EQ(result[1], doctest::Approx(0.4 * 2.0 + 0.6 * 1.0));
    CHECK_EQ(result[2], doctest::Approx(0.6 * 3.0 + 0.4 * result[1]));
}

/** @par Test points: NaN in x or a holds the previous DMA output */
TEST_CASE("test_DMA_nan_hold") {
    price_t nan = Null<price_t>();
    Indicator x = PRICELIST({1.0, 2.0, nan, 4.0});
    Indicator a = PRICELIST({0.5, 0.5, 0.5, 0.5});
    Indicator ret = DMA(x, a);
    // ret[0]=x[0]=1.0, ret[1]=0.5*2+0.5*1=1.5
    // ret[2]: x=NaN -> hold -> 1.5
    // ret[3]: 0.5*4+0.5*1.5 = 2.75
    CHECK_EQ(ret[0], 1.0);
    CHECK_EQ(ret[1], doctest::Approx(1.5));
    CHECK_EQ(ret[2], doctest::Approx(1.5));
    CHECK_EQ(ret[3], doctest::Approx(2.75));

    /** @arg NaN in the weight a also holds */
    Indicator a2 = PRICELIST({0.5, 0.5, nan, 0.5});
    Indicator ret2 = DMA(PRICELIST({1.0, 2.0, 3.0, 4.0}), a2);
    CHECK_EQ(ret2[2], ret2[1]);
    CHECK_UNARY(!std::isnan(ret2[3]));

    /** @arg Inf in x or a is also held (AGENTS.md contract: no Inf poisoning) */
    price_t inf_v = std::numeric_limits<price_t>::infinity();
    Indicator ret3 = DMA(PRICELIST({1.0, inf_v, 3.0, 4.0}), PRICELIST({0.5, 0.5, 0.5, 0.5}));
    CHECK_EQ(ret3[1], ret3[0]);
    for (size_t i = 0; i < ret3.size(); i++) {
        CHECK_UNARY(std::isfinite(ret3[i]));
    }
}

/** @} */
