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

/** @par Test points */
TEST_CASE("test_Slippage_clamp_to_day_range") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData kdata = stock.getKData(KQuery(0, 50, KQuery::DAY));
    REQUIRE_FALSE(kdata.empty());

    // A wide amplitude forces the clamp to actually trigger on most bars
    auto sp = SP_Uniform(-1.0, 1.0);
    sp->setTO(kdata);

    for (size_t i = 0, total = kdata.size(); i < total; i++) {
        const KRecord& k = kdata[i];
        /** @arg buy price never exceeds the day's high */
        price_t buy = sp->getRealBuyPrice(k.datetime, k.closePrice);
        CHECK_LE(buy, k.highPrice + 1e-9);
        /** @arg sell price never drops below the day's low and stays non-negative */
        price_t sell = sp->getRealSellPrice(k.datetime, k.closePrice);
        CHECK_GE(sell, k.lowPrice - 1e-9);
        CHECK_GE(sell, 0.0);
    }
}

/** @par Test points */
TEST_CASE("test_Slippage_sell_floor_for_low_price") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData kdata = stock.getKData(KQuery(0, 1, KQuery::DAY));
    REQUIRE_FALSE(kdata.empty());

    // A fixed offset larger than the plan price would otherwise yield a negative sell price
    auto sp = SP_FixedValue(100.0);
    sp->setTO(kdata);
    const KRecord& k = kdata[0];
    /** @arg a negative raw sell price is clamped up to the day's low, never negative */
    price_t sell = sp->getRealSellPrice(k.datetime, 10.0);
    CHECK_GE(sell, k.lowPrice - 1e-9);
    CHECK_GE(sell, 0.0);
}

/** @par Test points */
TEST_CASE("test_LogNormal_param_check") {
    /** @arg a negative stddev is rejected */
    CHECK_THROWS(SP_LogNormal(0.0, -0.01));
    /** @arg a NaN mean is rejected */
    CHECK_THROWS(SP_LogNormal(std::numeric_limits<double>::quiet_NaN(), 0.05));
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

    /** @arg the clone carries no TO, so the next setTO always recalculates (lazy recalculation) */
    SlippagePtr c = sp->clone();
    CHECK_UNARY(c->getTO().empty());
    c->setTO(kdata);
    CHECK_EQ(static_cast<CalcCountSP*>(c.get())->count, 1);
}

/** @} */
