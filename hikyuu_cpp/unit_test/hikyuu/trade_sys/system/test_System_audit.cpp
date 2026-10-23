/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-15
 *      Author: fasiondog
 *
 *  Regression tests for the System audit fixes.
 */

#include "../../test_config.h"
#include <cmath>
#include <limits>
#include <map>
#include <tuple>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>
#include <hikyuu/trade_sys/system/imp/WalkForwardSystem.h>
#include <hikyuu/trade_sys/selector/imp/optimal/OptimalSelectorBase.h>
#include <hikyuu/trade_sys/signal/crt/SG_AllwaysBuy.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/moneymanager/MoneyManagerBase.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include <hikyuu/trade_sys/profitgoal/ProfitGoalBase.h>
#include <hikyuu/trade_sys/slippage/SlippageBase.h>
#include <hikyuu/trade_sys/stoploss/StoplossBase.h>

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
    virtual price_t _getRealBuyShortPrice(const Datetime&, price_t) override {
        return raw;
    }
    virtual price_t _getRealSellShortPrice(const Datetime&, price_t) override {
        return raw;
    }
    virtual void _calculate() override {}
    virtual SlippagePtr _clone() override {
        auto p = std::make_shared<FixedRawPriceSP>();
        p->raw = raw;
        return p;
    }
};

// Reproduces the state of an archive loaded without setTO: the original kdata is gone, so every
// plan price taken from it becomes 0
class StaleSrcSystem : public System {
public:
    StaleSrcSystem() : System("StaleSrcSystem") {}

    void clearSrcKData() {
        m_src_kdata = KData();
    }
};

// Reports a Python callback without carrying a Python part of its own
class PythonAwareSystem : public System {
public:
    PythonAwareSystem() : System("PythonAwareSystem") {}

    virtual bool hasPythonPart() const override {
        return true;
    }
};

// A selector reporting a Python callback; the walk-forward aggregate drives it outside the
// sub-system list, so the sub-system recursion cannot see it
class PythonAwareSE : public OptimalSelectorBase {
public:
    PythonAwareSE() : OptimalSelectorBase("PythonAwareSE") {
        m_is_python_object = true;
    }

    virtual SelectorPtr _clone() override {
        return std::make_shared<PythonAwareSE>();
    }
};

class BuyOnFirstBarSG : public SignalBase {
public:
    BuyOnFirstBarSG() : SignalBase("BuyOnFirstBarSG") {}

    virtual SignalPtr _clone() override {
        return std::make_shared<BuyOnFirstBarSG>();
    }

    virtual void _calculate(const KData& kdata) override {
        HKU_IF_RETURN(kdata.empty(), void());
        _addBuySignal(kdata[0].datetime);
    }
};

// Emits one buy or sell signal on the given date
class SingleSignalSG : public SignalBase {
public:
    SingleSignalSG(const Datetime& date, bool buy)
    : SignalBase("SingleSignalSG"), m_date(date), m_buy(buy) {}

    virtual SignalPtr _clone() override {
        return std::make_shared<SingleSignalSG>(m_date, m_buy);
    }

    virtual void _calculate(const KData&) override {
        if (m_buy) {
            _addBuySignal(m_date);
        } else {
            _addSellSignal(m_date);
        }
    }

private:
    Datetime m_date;
    bool m_buy;
};

// Records the price each sizing interface receives and returns a fixed quantity, so a test can pin
// which price the system used, independently of any money manager formula
class PriceProbeMM : public MoneyManagerBase {
public:
    PriceProbeMM() : MoneyManagerBase("PriceProbeMM") {
        setParam<double>("n", 100.0);
    }

    price_t buy_price{0.0};
    price_t sell_price{0.0};
    price_t buy_short_price{0.0};
    price_t sell_short_price{0.0};

    virtual MoneyManagerPtr _clone() override {
        auto p = std::make_shared<PriceProbeMM>();
        p->setParam<double>("n", getParam<double>("n"));
        return p;
    }

    virtual double _getBuyNumber(const Datetime&, const Stock&, price_t price, price_t,
                                 SystemPart) override {
        buy_price = price;
        return getParam<double>("n");
    }

    virtual double _getSellNumber(const Datetime&, const Stock&, price_t price, price_t,
                                  SystemPart) override {
        sell_price = price;
        return getParam<double>("n");
    }

    virtual double _getBuyShortNumber(const Datetime&, const Stock&, price_t price, price_t,
                                      SystemPart) override {
        buy_short_price = price;
        return getParam<double>("n");
    }

    virtual double _getSellShortNumber(const Datetime&, const Stock&, price_t price, price_t,
                                       SystemPart) override {
        sell_short_price = price;
        return getParam<double>("n");
    }
};

// A short-side stop-loss 10% above the entry, so the cover path gets a positive risk and reaches
// the money manager instead of falling back to covering everything
class FixedShortST : public StoplossBase {
public:
    FixedShortST() : StoplossBase("FixedShortST") {}

    virtual StoplossPtr _clone() override {
        return std::make_shared<FixedShortST>();
    }

    virtual price_t _getPrice(const Datetime&, price_t price) override {
        return price * 0.9;
    }

    virtual price_t _getShortPrice(const Datetime&, price_t price) override {
        return price * 1.1;
    }
};

// The bar whose high (above == true) or low sits farthest from its close, so a fixed raw price
// produces a fill clearly away from the plan price
static size_t widest_span_pos(const KData& kdata, bool above) {
    size_t best = 0;
    double best_span = -1.0;
    for (size_t i = 0; i < kdata.size(); ++i) {
        double span = above ? kdata[i].highPrice - kdata[i].closePrice
                            : kdata[i].closePrice - kdata[i].lowPrice;
        if (span > best_span) {
            best_span = span;
            best = i;
        }
    }
    return best;
}

// Only the given date is valid, so the market environment turns invalid on the next bar and forces
// a liquidation regardless of the money manager's price check
class EvFirstBarOnly : public EnvironmentBase {
public:
    explicit EvFirstBarOnly(const Datetime& valid_date)
    : EnvironmentBase("EvFirstBarOnly"), m_valid_date(valid_date) {}

    virtual EnvironmentPtr _clone() override {
        return std::make_shared<EvFirstBarOnly>(m_valid_date);
    }

    virtual void _calculate() override {
        _addValid(m_valid_date);
    }

private:
    Datetime m_valid_date;
};

// Emits a buy on one date and a sell on another
class FixedDatesSG : public SignalBase {
public:
    FixedDatesSG(const Datetime& buy_date, const Datetime& sell_date)
    : SignalBase("FixedDatesSG"), m_buy_date(buy_date), m_sell_date(sell_date) {}

    virtual SignalPtr _clone() override {
        return std::make_shared<FixedDatesSG>(m_buy_date, m_sell_date);
    }

    virtual void _calculate(const KData&) override {
        _addBuySignal(m_buy_date);
        _addSellSignal(m_sell_date);
    }

private:
    Datetime m_buy_date;
    Datetime m_sell_date;
};

// Counts the goal price queries per bar; the goal always stays below the current price, so every
// held bar triggers the profit goal
class CountingPG : public ProfitGoalBase {
public:
    CountingPG() : ProfitGoalBase("CountingPG") {}

    std::map<Datetime, int> calls;

    virtual ProfitGoalPtr _clone() override {
        return std::make_shared<CountingPG>();
    }

    virtual price_t _getGoal(const Datetime& datetime, price_t price) override {
        calls[datetime]++;
        return price * 0.95;
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
    const double price_factor = std::pow(10.0, stk.precision());

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
            /** @arg the fill price always lands on the minimum price tick of the market (the
             *  tolerance only absorbs the last-bit noise of the stored doubles) */
            CHECK_LE(
              std::abs(tr.realPrice * price_factor - std::round(tr.realPrice * price_factor)),
              1e-9);
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

/** @par Check point: the buy size is derived from the fill price, so a full-cash money manager is
 *  still filled when the fill price is above the plan price; sizing at the plan price overflowed
 *  the cash and the whole order was rejected */
TEST_CASE("test_System_buy_size_follows_fill_price") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);
    // The plan price comes from the original (unadjusted) bar, so the same coordinate is used here
    KQuery raw_query = query;
    raw_query.recoverType(KQuery::NO_RECOVER);
    KData raw_kdata = stk.getKData(raw_query);
    REQUIRE(raw_kdata.size() > 2);

    const double init_cash = 100000.0;
    // The bar with the widest high-close span, so the fill price is clearly above the plan price
    const size_t pos = widest_span_pos(raw_kdata, true);
    const price_t fill_price = raw_kdata[pos].highPrice;
    REQUIRE_LT(raw_kdata[pos].closePrice, fill_price);

    auto sp = std::make_shared<FixedRawPriceSP>();
    sp->raw = fill_price;

    auto sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setTM(crtTM(Datetime(199001010000LL), init_cash, TC_Zero()));
    sys->setSG(std::make_shared<SingleSignalSG>(raw_kdata[pos].datetime, true));
    sys->setMM(MM_Nothing());
    sys->setSP(sp);
    sys->run(stk, query);

    TradeRecord tr;
    for (const auto& record : sys->getTM()->getTradeList()) {
        if (record.business == BUSINESS_BUY) {
            tr = record;
            break;
        }
    }
    REQUIRE_UNARY(tr.business == BUSINESS_BUY);

    /** @arg the fill price is the slippage result */
    CHECK_EQ(tr.realPrice, fill_price);

    const double lot = stk.minTradeNumber();
    const double unit = stk.unit();

    /** @arg the size is the whole cash evaluated at the fill price */
    CHECK_EQ(tr.number, std::floor(init_cash / (fill_price * unit) / lot) * lot);

    /** @arg the same size evaluated at the plan price would have needed more cash than available */
    const double plan_number = std::floor(init_cash / (tr.planPrice * unit) / lot) * lot;
    CHECK_GT(plan_number * fill_price * unit, init_cash);

    /** @arg the account is not overdrawn */
    CHECK_GE(sys->getTM()->currentCash(), 0.0);
}

/** @par Check point: the short-opening size is derived from the fill price as well, so the short is
 *  opened at the quantity the fill price can afford instead of the plan price */
TEST_CASE("test_System_sellShort_size_follows_fill_price") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);
    KQuery raw_query = query;
    raw_query.recoverType(KQuery::NO_RECOVER);
    KData raw_kdata = stk.getKData(raw_query);
    REQUIRE(raw_kdata.size() > 2);

    const double init_cash = 100000.0;
    // The bar with the widest close-low span, so the fill price is clearly below the plan price
    const size_t pos = widest_span_pos(raw_kdata, false);
    const price_t fill_price = raw_kdata[pos].lowPrice;
    REQUIRE_LT(fill_price, raw_kdata[pos].closePrice);

    auto sp = std::make_shared<FixedRawPriceSP>();
    sp->raw = fill_price;

    auto tm = crtTM(Datetime(199001010000LL), init_cash, TC_Zero());
    tm->setParam<bool>("support_borrow_stock", true);

    // The alternate signal state only admits a short entry from a flat book when borrowing is
    // enabled on the signal generator as well
    auto sg = std::make_shared<SingleSignalSG>(raw_kdata[pos].datetime, false);
    sg->setParam<bool>("support_borrow_stock", true);

    auto sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("support_borrow_stock", true);
    sys->setTM(tm);
    sys->setSG(sg);
    sys->setMM(MM_Nothing());
    sys->setSP(sp);
    sys->run(stk, query);

    TradeRecord tr;
    for (const auto& record : sys->getTM()->getTradeList()) {
        if (record.business == BUSINESS_SELL_SHORT) {
            tr = record;
            break;
        }
    }
    REQUIRE_UNARY(tr.business == BUSINESS_SELL_SHORT);

    /** @arg the fill price is the slippage result */
    CHECK_EQ(tr.realPrice, fill_price);

    const double lot = stk.minTradeNumber();
    const double unit = stk.unit();

    /** @arg the size is the net assets evaluated at the fill price */
    CHECK_EQ(tr.number, std::floor(init_cash / (fill_price * unit) / lot) * lot);

    /** @arg sizing at the plan price would have opened a smaller short */
    CHECK_LT(std::floor(init_cash / (tr.planPrice * unit) / lot) * lot, tr.number);
}

/** @par Check point: the delayed buy asks the money manager for the quantity with the price it is
 *  actually filled at, not with the plan price */
TEST_CASE("test_System_delay_buy_number_uses_fill_price") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);
    KQuery raw_query = query;
    raw_query.recoverType(KQuery::NO_RECOVER);
    KData raw_kdata = stk.getKData(raw_query);
    REQUIRE(raw_kdata.size() > 3);

    const price_t fill_price = raw_kdata[1].highPrice;
    auto sp = std::make_shared<FixedRawPriceSP>();
    sp->raw = fill_price;

    auto mm = std::make_shared<PriceProbeMM>();
    auto sys = SYS_Simple();
    sys->setTM(crtTM(Datetime(199001010000LL), 100000.0, TC_Zero()));
    sys->setSG(std::make_shared<SingleSignalSG>(raw_kdata[0].datetime, true));
    sys->setMM(mm);
    sys->setSP(sp);
    // buy_delay defaults to true: the signal on the first bar fills at the next bar's open
    sys->run(stk, query);

    TradeRecord tr;
    for (const auto& record : sys->getTM()->getTradeList()) {
        if (record.business == BUSINESS_BUY) {
            tr = record;
            break;
        }
    }
    REQUIRE_UNARY(tr.business == BUSINESS_BUY);

    /** @arg the delayed fill uses the slippage price */
    CHECK_EQ(tr.realPrice, fill_price);

    /** @arg the quantity was requested with the fill price, not with the plan price */
    CHECK_EQ(mm->buy_price, tr.realPrice);
    CHECK_EQ(tr.number, 100.0);
}

/** @par Check point: with delay_use_current_price=false the size decided at the signal bar's close
 *  is re-sized at the fill price when it no longer fits the cash, instead of being rejected by the
 *  trade manager as a whole */
TEST_CASE("test_System_delay_stale_size_is_resized") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);
    KQuery raw_query = query;
    raw_query.recoverType(KQuery::NO_RECOVER);
    KData raw_kdata = stk.getKData(raw_query);
    REQUIRE(raw_kdata.size() > 3);

    // The bar whose high sits farthest above the previous close, so the size decided at that close
    // can no longer afford the fill
    size_t gap_pos = 1;
    double best_gap = -1.0;
    for (size_t i = 1; i < raw_kdata.size(); ++i) {
        double gap = raw_kdata[i].highPrice - raw_kdata[i - 1].closePrice;
        if (gap > best_gap) {
            best_gap = gap;
            gap_pos = i;
        }
    }
    REQUIRE_GT(best_gap, 0.0);

    const double init_cash = 100000.0;
    const price_t fill_price = raw_kdata[gap_pos].highPrice;
    auto sp = std::make_shared<FixedRawPriceSP>();
    sp->raw = fill_price;

    auto sys = SYS_Simple();
    sys->setParam<bool>("delay_use_current_price", false);
    sys->setTM(crtTM(Datetime(199001010000LL), init_cash, TC_Zero()));
    sys->setSG(std::make_shared<SingleSignalSG>(raw_kdata[gap_pos - 1].datetime, true));
    sys->setMM(MM_Nothing());
    sys->setSP(sp);
    sys->run(stk, query);

    TradeRecord tr;
    for (const auto& record : sys->getTM()->getTradeList()) {
        if (record.business == BUSINESS_BUY) {
            tr = record;
            break;
        }
    }
    REQUIRE_UNARY(tr.business == BUSINESS_BUY);

    const double lot = stk.minTradeNumber();
    const double unit = stk.unit();
    const double stale =
      std::floor(init_cash / (raw_kdata[gap_pos - 1].closePrice * unit) / lot) * lot;

    /** @arg the stale size evaluated at the fill price would have been rejected */
    CHECK_GT(stale * fill_price * unit, init_cash);

    /** @arg the order is re-sized to what the fill price can afford */
    CHECK_EQ(tr.number, std::floor(init_cash / (fill_price * unit) / lot) * lot);
    CHECK_LT(tr.number, stale);
    CHECK_GE(sys->getTM()->currentCash(), 0.0);
}

/** @par Check point: the long sell and both short-side covers receive the price they are actually
 *  filled at */
TEST_CASE("test_System_sell_and_cover_number_use_fill_price") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);
    KQuery raw_query = query;
    raw_query.recoverType(KQuery::NO_RECOVER);
    KData raw_kdata = stk.getKData(raw_query);
    REQUIRE(raw_kdata.size() > 5);

    const Datetime open_date = raw_kdata[0].datetime;
    const Datetime exit_date = raw_kdata[3].datetime;
    const price_t raw_price = raw_kdata[3].lowPrice;

    // A fixed raw price keeps the fill independent of the plan price
    auto run = [&](bool short_side) {
        auto sp = std::make_shared<FixedRawPriceSP>();
        sp->raw = raw_price;

        auto mm = std::make_shared<PriceProbeMM>();
        // Long: buy on the first bar, sell on the fourth. Short: open on the first, cover on the
        // fourth, so each side exercises one entry and one exit
        auto sg = std::make_shared<FixedDatesSG>(short_side ? exit_date : open_date,
                                                 short_side ? open_date : exit_date);
        sg->setParam<bool>("support_borrow_stock", short_side);

        auto sys = SYS_Simple();
        sys->setParam<bool>("buy_delay", false);
        sys->setParam<bool>("sell_delay", false);
        sys->setParam<bool>("support_borrow_stock", short_side);
        sys->setTM(crtTM(Datetime(199001010000LL), 100000.0, TC_Zero()));
        sys->setSG(sg);
        sys->setMM(mm);
        sys->setSP(sp);
        if (short_side) {
            // A positive short-side risk makes the cover go through the money manager instead of
            // the "cover everything" fallback
            sys->setST(std::make_shared<FixedShortST>());
        }
        sys->run(stk, query);
        return std::make_pair(mm, sys->getTM()->getTradeList());
    };

    auto long_run = run(false);
    TradeRecord buy_tr, sell_tr;
    for (const auto& tr : long_run.second) {
        if (tr.business == BUSINESS_BUY) {
            buy_tr = tr;
        } else if (tr.business == BUSINESS_SELL) {
            sell_tr = tr;
        }
    }
    REQUIRE_UNARY(buy_tr.business == BUSINESS_BUY);
    REQUIRE_UNARY(sell_tr.business == BUSINESS_SELL);

    /** @arg the long entry and exit receive their own fill price */
    CHECK_EQ(long_run.first->buy_price, buy_tr.realPrice);
    CHECK_EQ(long_run.first->sell_price, sell_tr.realPrice);

    auto short_run = run(true);
    TradeRecord open_tr, cover_tr;
    for (const auto& tr : short_run.second) {
        if (tr.business == BUSINESS_SELL_SHORT) {
            open_tr = tr;
        } else if (tr.business == BUSINESS_BUY_SHORT) {
            cover_tr = tr;
        }
    }
    REQUIRE_UNARY(open_tr.business == BUSINESS_SELL_SHORT);
    REQUIRE_UNARY(cover_tr.business == BUSINESS_BUY_SHORT);

    /** @arg the short opening and the cover receive their own fill price */
    CHECK_EQ(short_run.first->sell_short_price, open_tr.realPrice);
    CHECK_EQ(short_run.first->buy_short_price, cover_tr.realPrice);
}

/** @par Check point: a stale or zero-price original kdata (plan price 0) never produces a
 *  zero-price fill. The forced liquidation asks the money manager for the whole position before it
 *  looks at the price, so without the closing-entry guard the position would be cleared at price 0
 *  and the funds would evaporate */
TEST_CASE("test_System_stale_src_no_zero_price_fill") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 2);

    // Returns: hold after the buy, cash after the buy, sell fills on bar 1, cash after bar 1,
    // hold after bar 1
    auto run_bars = [&](bool stale_src) {
        auto sys = std::make_shared<StaleSrcSystem>();
        sys->setTM(crtTM(Datetime(199001010000LL), 100000.0, TC_Zero()));
        sys->setEV(std::make_shared<EvFirstBarOnly>(kdata[0].datetime));
        sys->setSG(std::make_shared<BuyOnFirstBarSG>());
        sys->setMM(MM_FixedCount(100));
        sys->setParam<bool>("buy_delay", false);
        sys->setParam<bool>("sell_delay", false);
        sys->setTO(kdata);
        sys->readyForRun();

        sys->runMoment(kdata[0].datetime);
        const double hold = sys->getTM()->getHoldNumber(kdata[0].datetime, stk);
        const price_t cash = sys->getTM()->currentCash();

        if (stale_src) {
            sys->clearSrcKData();
        }

        MomentResult r1 = sys->runMoment(kdata[1].datetime);
        return std::make_tuple(hold, cash, r1.tradesOnClose.size(), sys->getTM()->currentCash(),
                               sys->getTM()->getHoldNumber(kdata[1].datetime, stk));
    };

    /** @arg control run with the original kdata intact: the environment turns invalid on bar 1 and
     *  the forced liquidation fills */
    auto ctrl = run_bars(false);
    REQUIRE_UNARY(std::get<0>(ctrl) > 0.0);
    CHECK_EQ(std::get<2>(ctrl), 1);
    CHECK_GT(std::get<3>(ctrl), std::get<1>(ctrl));
    CHECK_EQ(std::get<4>(ctrl), 0.0);

    /** @arg stale original kdata (plan price 0): no fill, the position and the cash are left
     *  untouched */
    auto stale = run_bars(true);
    REQUIRE_UNARY(std::get<0>(stale) > 0.0);
    CHECK_EQ(std::get<2>(stale), 0);
    CHECK_EQ(std::get<3>(stale), std::get<1>(stale));
    CHECK_EQ(std::get<4>(stale), std::get<0>(stale));
}

/** @par Check point: the forced liquidation on an invalid environment follows sell_delay - delayed
 *  to the next open when true, filled at the invalidating close when false */
TEST_CASE("test_System_ev_force_sell_follows_sell_delay") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 3);

    // Returns: hold after the buy, fills on the invalidating close, pending delays, fills on the
    // next open, hold after the next bar
    auto run_bars = [&](bool sell_delay) {
        auto sys = SYS_Simple();
        sys->setTM(crtTM(Datetime(199001010000LL), 100000.0, TC_Zero()));
        sys->setEV(std::make_shared<EvFirstBarOnly>(kdata[0].datetime));
        sys->setSG(std::make_shared<BuyOnFirstBarSG>());
        sys->setMM(MM_FixedCount(100));
        sys->setParam<bool>("buy_delay", false);
        sys->setParam<bool>("sell_delay", sell_delay);
        sys->setTO(kdata);
        sys->readyForRun();

        sys->runMoment(kdata[0].datetime);
        const double hold = sys->getTM()->getHoldNumber(kdata[0].datetime, stk);
        MomentResult r1 = sys->runMoment(kdata[1].datetime);
        MomentResult r2 = sys->runMoment(kdata[2].datetime);
        return std::make_tuple(hold, r1.tradesOnClose.size(), r1.delayOnNextOpen.size(),
                               r2.tradesOnOpen.size(),
                               sys->getTM()->getHoldNumber(kdata[2].datetime, stk));
    };

    /** @arg sell_delay=true (the default): nothing fills on the invalidating close, the request is
     *  exposed as pending and fills at the next open */
    auto delayed = run_bars(true);
    REQUIRE_UNARY(std::get<0>(delayed) > 0.0);
    CHECK_EQ(std::get<1>(delayed), 0);
    CHECK_EQ(std::get<2>(delayed), 1);
    CHECK_EQ(std::get<3>(delayed), 1);
    CHECK_EQ(std::get<4>(delayed), 0.0);

    /** @arg sell_delay=false: the liquidation fills at the invalidating close itself */
    auto immediate = run_bars(false);
    REQUIRE_UNARY(std::get<0>(immediate) > 0.0);
    CHECK_EQ(std::get<1>(immediate), 1);
    CHECK_EQ(std::get<2>(immediate), 0);
    CHECK_EQ(std::get<4>(immediate), 0.0);
}

/** @par Check point: the delayed requests are served cash generating side first - a pending sell is
 *  executed before a pending buy on the same open */
TEST_CASE("test_System_delay_request_priority") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(201112060000LL), KQuery::DAY);
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 10);

    // A suspended bar (no volume) keeps both pending requests queued: the delayed buy is not
    // executed there and the sell request raised on its close joins the queue
    size_t suspended = Null<size_t>();
    for (size_t i = 2; i + 2 < kdata.size(); i++) {
        if (kdata[i].transAmount == 0.0 || kdata[i].transCount == 0.0) {
            suspended = i;
            break;
        }
    }
    REQUIRE_UNARY(suspended != Null<size_t>());

    auto tm = crtTM(Datetime(199001010000LL), 100000.0, TC_Zero());
    REQUIRE_UNARY(
      tm->buy(kdata[suspended - 2].datetime, stk, kdata[suspended - 2].closePrice, 100.0)
        .business == BUSINESS_BUY);

    auto sys = SYS_Simple();
    sys->setTM(tm);
    sys->setSG(
      std::make_shared<FixedDatesSG>(kdata[suspended - 1].datetime, kdata[suspended].datetime));
    sys->setMM(MM_FixedCount(100));
    sys->setParam<bool>("buy_delay", true);
    sys->setParam<bool>("sell_delay", true);
    sys->setTO(kdata);
    sys->readyForRun();

    sys->runMoment(kdata[suspended - 1].datetime);
    MomentResult rs = sys->runMoment(kdata[suspended].datetime);

    /** @arg the suspended bar fills nothing and leaves both requests pending */
    CHECK_UNARY(rs.tradesOnOpen.empty());
    REQUIRE_EQ(rs.delayOnNextOpen.size(), 2);

    MomentResult r1 = sys->runMoment(kdata[suspended + 1].datetime);

    /** @arg the sell is served first: it fills and the buy keeps waiting */
    REQUIRE_EQ(r1.tradesOnOpen.size(), 1);
    CHECK_EQ(r1.tradesOnOpen[0].business, BUSINESS_SELL);
    CHECK_EQ(sys->getTM()->getHoldNumber(kdata[suspended + 1].datetime, stk), 0.0);
    CHECK_UNARY(!r1.delayOnNextOpen.empty());

    /** @arg the buy fills on the following open */
    MomentResult r2 = sys->runMoment(kdata[suspended + 2].datetime);
    REQUIRE_EQ(r2.tradesOnOpen.size(), 1);
    CHECK_EQ(r2.tradesOnOpen[0].business, BUSINESS_BUY);
    CHECK_GT(sys->getTM()->getHoldNumber(kdata[suspended + 2].datetime, stk), 0.0);
}

/** @par Check point: the profit goal price is queried once per bar even on the bar where it
 *  triggers - the exit check and the trade record share the same value */
TEST_CASE("test_System_profit_goal_called_once_per_bar") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200012310000LL), KQuery::DAY);

    auto pg = std::make_shared<CountingPG>();
    auto sys = SYS_Simple();
    sys->setTM(crtTM(Datetime(199001010000LL), 100000.0, TC_Zero()));
    sys->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
    sys->setMM(MM_FixedCount(100));
    sys->setPG(pg);
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->run(stk, query);

    REQUIRE_UNARY(!pg->calls.empty());
    for (const auto& item : pg->calls) {
        CHECK_EQ(item.second, 1);
    }
}

/** @par Check point: an aggregate reports a Python callback held by any sub-system, at any depth */
TEST_CASE("test_System_hasPythonPart_recurses_into_subsystems") {
    /** @arg a system with C++ parts only reports none */
    CHECK_UNARY_FALSE(SYS_Simple()->hasPythonPart());

    /** @arg one sub-system reporting a Python callback is enough */
    auto ms = std::make_shared<MultiSystem>(
      SystemList{SYS_Simple(), std::make_shared<PythonAwareSystem>()}, "MS");
    CHECK_UNARY(ms->hasPythonPart());

    /** @arg the report reaches an aggregate nested in an aggregate */
    auto nested =
      std::make_shared<MultiSystem>(SystemList{std::make_shared<MultiSystem>(
                                      SystemList{std::make_shared<PythonAwareSystem>()}, "INNER")},
                                    "OUTER");
    CHECK_UNARY(nested->hasPythonPart());

    /** @arg an aggregate of C++ systems only still reports none */
    auto clean = std::make_shared<MultiSystem>(SystemList{SYS_Simple(), SYS_Simple()}, "CLEAN");
    CHECK_UNARY_FALSE(clean->hasPythonPart());

    /** @arg the walk-forward form also reports its selector and driven candidate */
    auto candidate = SYS_Simple();
    candidate->setTM(crtTM());
    candidate->setMM(MM_FixedCount(100));
    candidate->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
    auto fw = std::make_shared<WalkForwardSystem>(SystemList{candidate},
                                                  std::make_shared<PythonAwareSE>(), crtTM());
    CHECK_UNARY(fw->hasPythonPart());
}

/** @} */
