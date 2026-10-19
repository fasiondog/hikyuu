/*
 * test_Slippage_audit.cpp
 *
 * Regression tests for the slippage component.
 */

#include "../../test_config.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <hikyuu/StockManager.h>
#include <hikyuu/KData.h>
#include <hikyuu/trade_sys/slippage/SlippageBase.h>
#include <hikyuu/trade_sys/slippage/crt/SP_Normal.h>
#include <hikyuu/trade_sys/slippage/crt/SP_Uniform.h>
#include <hikyuu/trade_sys/slippage/crt/SP_LogNormal.h>
#include <hikyuu/trade_sys/slippage/crt/SP_TruncNormal.h>
#include <hikyuu/trade_sys/slippage/crt/SP_FixedPercent.h>
#include <hikyuu/trade_sys/slippage/crt/SP_FixedValue.h>

using namespace hku;

/**
 * @defgroup test_Slippage_audit test_Slippage_audit
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

namespace {
// A slippage that always returns a fixed raw price, ignoring the plan price
class RawPriceSP : public SlippageBase {
public:
    RawPriceSP() : SlippageBase("RawPriceSP") {}
    virtual ~RawPriceSP() {}

    double raw{0.0};

    virtual price_t _getRealBuyPrice(const Datetime&, price_t) override {
        return raw;
    }
    virtual price_t _getRealSellPrice(const Datetime&, price_t) override {
        return raw;
    }
    virtual void _calculate() override {}
    virtual SlippagePtr _clone() override {
        auto p = std::make_shared<RawPriceSP>();
        p->raw = raw;
        return p;
    }
};
}  // namespace

/** @par Test points */
TEST_CASE("test_Slippage_raw_price_passed_through") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData kdata = stock.getKData(KQuery(0, 50, KQuery::DAY));
    REQUIRE_FALSE(kdata.empty());

    // SP only produces the price of the model: the validity check and the day-range clamp belong to
    // the System part, which is the only caller in the framework
    auto sp = std::make_shared<RawPriceSP>();
    sp->setTO(kdata);
    const KRecord& k = kdata[0];

    const price_t raws[] = {0.0, -1.0, 1e9, 0.0001, k.closePrice};
    for (price_t raw : raws) {
        sp->raw = raw;
        /** @arg the model price is returned unchanged, even when it is invalid or out of range */
        CHECK_EQ(sp->getRealBuyPrice(k.datetime, k.closePrice), raw);
        CHECK_EQ(sp->getRealSellPrice(k.datetime, k.closePrice), raw);
    }

    /** @arg a non-finite model price is returned unchanged as well */
    sp->raw = std::numeric_limits<double>::quiet_NaN();
    CHECK_UNARY(std::isnan(sp->getRealBuyPrice(k.datetime, k.closePrice)));
    CHECK_UNARY(std::isnan(sp->getRealSellPrice(k.datetime, k.closePrice)));
}

/** @par Test points */
TEST_CASE("test_LogNormal_param_check") {
    /** @arg a negative stddev is rejected */
    CHECK_THROWS(SP_LogNormal(0.0, -0.01));
    /** @arg a NaN mean is rejected */
    CHECK_THROWS(SP_LogNormal(std::numeric_limits<double>::quiet_NaN(), 0.05));
}

/** @par Test points */
TEST_CASE("test_random_sp_oversize_warn_only") {
    // The four random slippages share one threshold for an absurd parameter: a stddev or an
    // amplitude above 1.0 (a price move of more than 100%) only warns, it is never rejected.
    /** @arg an oversized stddev is accepted (SP_Normal / SP_LogNormal warn above 1.0) */
    CHECK_NOTHROW(SP_Normal(0.0, 2.0));
    CHECK_NOTHROW(SP_LogNormal(0.0, 2.0));
    /** @arg an oversized amplitude is accepted (SP_Uniform / SP_TruncNormal warn above 1.0) */
    CHECK_NOTHROW(SP_Uniform(-2.0, 2.0));
    CHECK_NOTHROW(SP_TruncNormal(0.0, 0.05, -2.0, 2.0));
    /** @arg a negative stddev is still rejected */
    CHECK_THROWS(SP_Normal(0.0, -0.01));
}

/** @par Test points */
TEST_CASE("test_Slippage_short_param_clone") {
    /** @arg the short parameter survives clone */
    auto sp = SP_FixedPercent(0.01, 0.05);
    SlippagePtr c = sp->clone();
    CHECK(c->getRealBuyShortPrice(Datetime(202201010930), 10.0) == doctest::Approx(10.5));

    /** @arg leaving short_value unset takes the value passed for the long side */
    auto sp2 = SP_FixedValue(0.1);
    CHECK_EQ(sp2->getParam<double>("short_value"), 0.1);
    CHECK(sp2->getRealBuyShortPrice(Datetime(202201010930), 10.0) == doctest::Approx(10.1));
}

/** @par Test points */
TEST_CASE("test_Slippage_adverse_only_param_clone") {
    auto sp = SP_Uniform(-0.05, 0.05, false);
    sp->setParam<int64_t>("seed", 42);
    sp->reset();
    SlippagePtr c = sp->clone();
    /** @arg adverse_only=false survives clone and the clone also produces symmetric offsets */
    CHECK_UNARY(!c->getParam<bool>("adverse_only"));
    bool has_below = false;
    for (int i = 0; i < 200; ++i) {
        if (c->getRealBuyPrice(Datetime(202201010930), 10.0) < 10.0) {
            has_below = true;
            break;
        }
    }
    CHECK_UNARY(has_below);
}

/** @par Test points */
TEST_CASE("test_Slippage_clone_reseed") {
    auto sp = SP_Uniform(-0.05, 0.05);
    sp->setParam<int64_t>("seed", 42);
    sp->reset();
    SlippagePtr c = sp->clone();
    /** @arg clone reseeds its RNG from the seed, producing a reproducible sequence */
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(sp->getRealBuyPrice(Datetime(202201010930), 10.0),
                 c->getRealBuyPrice(Datetime(202201010930), 10.0));
        CHECK_EQ(sp->getRealSellPrice(Datetime(202201010930), 10.0),
                 c->getRealSellPrice(Datetime(202201010930), 10.0));
    }
}

/** @par Test points */
TEST_CASE("test_Slippage_seed_takes_effect_on_reset") {
    // The seed is only read in reset(): setting it through set_param does not reseed by itself
    auto sp1 = SP_Uniform(-0.05, 0.05);
    auto sp2 = SP_Uniform(-0.05, 0.05);
    sp1->setParam<int64_t>("seed", 42);
    sp2->setParam<int64_t>("seed", 42);

    /** @arg before reset() the two instances do not share a sequence */
    bool same = true;
    for (int i = 0; i < 5; ++i) {
        same = same && (sp1->getRealBuyPrice(Datetime(202201010930), 10.0) ==
                        sp2->getRealBuyPrice(Datetime(202201010930), 10.0));
    }
    CHECK_UNARY(!same);

    /** @arg after reset() both replay the same sequence */
    sp1->reset();
    sp2->reset();
    for (int i = 0; i < 5; ++i) {
        CHECK_EQ(sp1->getRealBuyPrice(Datetime(202201010930), 10.0),
                 sp2->getRealBuyPrice(Datetime(202201010930), 10.0));
        CHECK_EQ(sp1->getRealSellPrice(Datetime(202201010930), 10.0),
                 sp2->getRealSellPrice(Datetime(202201010930), 10.0));
    }
}

/** @par Test points */
TEST_CASE("test_TruncNormal_inverse_cdf") {
    auto sp = SP_TruncNormal(0.0, 0.05, -0.1, 0.1);
    double minv = 1e9, maxv = -1e9;
    for (int i = 0; i < 5000; ++i) {
        price_t buy = sp->getRealBuyPrice(Datetime(202201010930), 10.0);
        price_t off = buy - 10.0;
        /** @arg the sampled amplitude stays within [min_value, max_value] */
        CHECK_GE(off, -1e-9);
        CHECK_LE(off, 0.1 + 1e-9);
        minv = std::min(minv, off);
        maxv = std::max(maxv, off);
    }
    /** @arg the inverse-CDF sampling uses a meaningful part of the range, not a single boundary */
    CHECK_GT(maxv - minv, 0.05);
}

/** @par Test points */
TEST_CASE("test_TruncNormal_far_mean_no_degenerate") {
    // mean far below the truncation interval: rejection sampling would collapse to a boundary,
    // the inverse-CDF sampling must still spread across the interval
    auto sp = SP_TruncNormal(0.0, 0.05, 0.5, 1.0);
    double minv = 1e9, maxv = -1e9;
    for (int i = 0; i < 5000; ++i) {
        price_t buy = sp->getRealBuyPrice(Datetime(202201010930), 10.0);
        price_t off = buy - 10.0;
        CHECK_GE(off, 0.5 - 1e-9);
        CHECK_LE(off, 1.0 + 1e-9);
        minv = std::min(minv, off);
        maxv = std::max(maxv, off);
    }
    CHECK_GT(maxv - minv, 0.1);
}

/** @par Test points */
TEST_CASE("test_FixedPercent_param_and_input") {
    /** @arg p must be in [0, 1.0) */
    CHECK_THROWS(SP_FixedPercent(1.0));
    auto sp = SP_FixedPercent(0.001);
    /** @arg an invalid (non-positive) plan price is returned as-is */
    CHECK_EQ(sp->getRealBuyPrice(Datetime(202201010930), -5.0), -5.0);
    CHECK_EQ(sp->getRealSellPrice(Datetime(202201010930), -5.0), -5.0);
}

namespace {
class CalcCountSP : public SlippageBase {
public:
    CalcCountSP() : SlippageBase("CalcCountSP"), count(0) {}
    virtual ~CalcCountSP() {}
    int count;
    virtual price_t _getRealBuyPrice(const Datetime&, price_t p) override {
        return p;
    }
    virtual price_t _getRealSellPrice(const Datetime&, price_t p) override {
        return p;
    }
    virtual void _calculate() override {
        ++count;
    }
    virtual SlippagePtr _clone() override {
        return std::make_shared<CalcCountSP>();
    }
};
}  // namespace

/** @par Test points */
TEST_CASE("test_Slippage_clone_recalculates") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData kdata = stock.getKData(KQuery(0, 50, KQuery::DAY));
    REQUIRE_FALSE(kdata.empty());

    SlippagePtr sp = std::make_shared<CalcCountSP>();
    sp->setTO(kdata);
    CHECK_EQ(static_cast<CalcCountSP*>(sp.get())->count, 1);

    /** @arg the clone carries the source TO but is marked stale, so the next setTO recalculates */
    SlippagePtr c = sp->clone();
    CHECK_UNARY(!c->getTO().empty());
    c->setTO(kdata);
    CHECK_EQ(static_cast<CalcCountSP*>(c.get())->count, 1);
}

/** @} */
