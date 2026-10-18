/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-15
 *      Author: fasiondog
 *
 *  Regression tests for the System audit fixes.
 */

#include "../../test_config.h"
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/crt/SG_AllwaysBuy.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/slippage/SlippageBase.h>

using namespace hku;

/**
 * @defgroup test_System_audit test_System_audit
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Check point: ignore_sell_sg suppresses the SG sell signal (the parameter used to be
 * registered and documented but never read) */
TEST_CASE("test_System_ignore_sell_sg") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);
    TMPtr tm = crtTM(Datetime(199001010000LL), 100000.0, TC_Zero());
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));

    // With no stop-loss / take-profit / goal set, the SG is the only exit source
    auto sg_sell_count = [&](bool ignore) {
        auto sys = SYS_Simple();
        sys->setParam<bool>("buy_delay", false);
        sys->setParam<bool>("sell_delay", false);
        sys->setParam<bool>("ignore_sell_sg", ignore);
        sys->setTM(tm->clone());
        sys->setSG(sg->clone());
        sys->setMM(MM_FixedCount(100));
        sys->run(stk, query);
        size_t cnt = 0;
        for (const auto& tr : sys->getTM()->getTradeList()) {
            if (tr.business == BUSINESS_SELL && tr.from == PART_SIGNAL) {
                cnt++;
            }
        }
        return cnt;
    };

    /** @arg default: the SG sell signal liquidates the position */
    CHECK_UNARY(sg_sell_count(false) > 0);
    /** @arg ignore_sell_sg=true: the SG sell signal is not acted on */
    CHECK_EQ(sg_sell_count(true), 0);
}

/** @par Check point: max_delay_count=0 still allows one delayed execution (the request used to be
 * dropped on the next open because the counter started at 1, so a value of 0 silently disabled the
 * delayed trading) */
TEST_CASE("test_System_runMoment_max_delay_count_zero") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 2);

    auto sys = SYS_Simple(crtTM(Datetime(199001010000LL), 100000.0), MM_FixedCount(100),
                          EnvironmentPtr(), ConditionPtr(), SG_AllwaysBuy());
    sys->setParam<bool>("buy_delay", true);
    sys->setParam<int>("max_delay_count", 0);
    sys->setTO(kdata);
    sys->readyForRun();

    /** @arg bar 0: the buy is delayed, no trade is made and one pending request is visible */
    MomentResult r0 = sys->runMoment(kdata[0].datetime);
    CHECK_UNARY(r0.tradesOnOpen.empty());
    CHECK_UNARY(r0.tradesOnClose.empty());
    REQUIRE_EQ(r0.delayOnNextOpen.size(), 1);

    /** @arg bar 1: the single allowed delay is executed at the open */
    MomentResult r1 = sys->runMoment(kdata[1].datetime);
    CHECK_UNARY(!r1.tradesOnOpen.empty());
}

namespace {
// A slippage that always returns a fixed raw price, ignoring the plan price
class FixedRawPriceSP : public SlippageBase {
public:
    FixedRawPriceSP() : SlippageBase("FixedRawPriceSP") {}
    virtual ~FixedRawPriceSP() {}

    double raw{0.0};

    virtual price_t _getRealBuyPrice(const Datetime&, price_t) override {
        return raw;
    }
    virtual price_t _getRealSellPrice(const Datetime&, price_t) override {
        return raw;
    }
    virtual void _calculate() override {}
    virtual SlippagePtr _clone() override {
        auto p = std::make_shared<FixedRawPriceSP>();
        p->raw = raw;
        return p;
    }
};
}  // namespace

/** @par Check point: System owns the price authority of the slippage result - an invalid value
 *  falls back to the plan price and an out-of-range value is clamped into the bar's range, so what
 *  reaches the trade record is always a usable price inside the bar */
TEST_CASE("test_System_slippage_price_fixup") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);
    // The slippage price is clamped against the original (unadjusted) bar, so the assertions below
    // use the same coordinate
    KQuery raw_query = query;
    raw_query.recoverType(KQuery::NO_RECOVER);
    KData raw_kdata = stk.getKData(raw_query);
    REQUIRE_UNARY(!raw_kdata.empty());

    auto run_with_raw = [&](double raw) {
        auto sp = std::make_shared<FixedRawPriceSP>();
        sp->raw = raw;
        auto sys = SYS_Simple();
        sys->setParam<bool>("buy_delay", false);
        sys->setParam<bool>("sell_delay", false);
        sys->setTM(crtTM(Datetime(199001010000LL), 100000.0, TC_Zero()));
        sys->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
        sys->setMM(MM_FixedCount(100));
        sys->setSP(sp);
        sys->run(stk, query);
        return sys->getTM()->getTradeList();
    };

    // The trade list also holds the initial deposit record of the account
    auto for_each_trade = [&](double raw, auto&& check_trade) {
        auto list = run_with_raw(raw);
        REQUIRE_UNARY(!list.empty());
        size_t count = 0;
        for (const auto& tr : list) {
            if (tr.business != BUSINESS_BUY && tr.business != BUSINESS_SELL &&
                tr.business != BUSINESS_BUY_SHORT && tr.business != BUSINESS_SELL_SHORT) {
                continue;
            }
            const KRecord& k = raw_kdata.getKRecord(tr.datetime);
            REQUIRE_UNARY(k.isValid());
            ++count;
            check_trade(tr, k);
        }
        REQUIRE_UNARY(count > 0);
    };

    /** @arg a raw price far above the bar is clamped down to the bar's high */
    for_each_trade(
      1e9, [](const TradeRecord& tr, const KRecord& k) { CHECK_EQ(tr.realPrice, k.highPrice); });

    /** @arg a positive but too low raw price is clamped up to the bar's low */
    for_each_trade(
      0.0001, [](const TradeRecord& tr, const KRecord& k) { CHECK_EQ(tr.realPrice, k.lowPrice); });

    /** @arg an invalid raw price falls back to the plan price: 0 is the market-order placeholder
     *  of the trade manager and must never come from the slippage path */
    const double invalid[] = {0.0, -1.0, std::numeric_limits<double>::quiet_NaN(),
                              std::numeric_limits<double>::infinity()};
    for (double raw : invalid) {
        for_each_trade(raw, [](const TradeRecord& tr, const KRecord& k) {
            CHECK_GT(tr.realPrice, 0.0);
            CHECK_GE(tr.realPrice, k.lowPrice - 1e-9);
            CHECK_LE(tr.realPrice, k.highPrice + 1e-9);
        });
    }
}

/** @} */
