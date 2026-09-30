/*
 * test_SYS_Simple_tp.cpp
 *
 * Take-profit tests of SYS_Simple on adjusted KData (ISS-081): the take-profit indicator runs on
 * the adjusted KData while the exit checks, the ratchet anchor and the TM accounting all use the
 * original price, so the indicator result must be mapped back to the original coordinate. Also
 * covers the tp_delay_n counting when the position was opened before the current KData window
 * (ISS-133).
 */

#include "doctest/doctest.h"
#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/stoploss/StoplossBase.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>

using namespace hku;

namespace {

// sh600000 went ex-dividend on 2000-07-06 (cash 0.15 per share), so in this window the FORWARD
// adjusted prices before that date are about 0.15 lower than the original ones
const Datetime BUY_DATE(200006270000LL);

// Emits one buy (or sell in the short mode) signal at a fixed date
class Iss081BuySG : public SignalBase {
public:
    explicit Iss081BuySG(const Datetime& datetime, bool shortMode = false)
    : SignalBase("ISS081_BuySG"), m_date(datetime), m_shortMode(shortMode) {
        if (shortMode) {
            setParam<bool>("support_borrow_stock", true);
        }
    }

    virtual SignalPtr _clone() override {
        return make_shared<Iss081BuySG>(m_date, m_shortMode);
    }

    virtual void _calculate(const KData& kdata) override {
        if (kdata.getPos(m_date) != Null<size_t>()) {
            if (m_shortMode) {
                _addSellSignal(m_date);
            } else {
                _addBuySignal(m_date);
            }
        }
    }

private:
    Datetime m_date;
    bool m_shortMode;
};

// A take-profit whose level is the close price of its own (adjusted) KData shifted by a fixed
// gap, exactly like an indicator-based stop-loss which ignores the passed planned price
class Iss081RecoveredGapTP : public StoplossBase {
public:
    Iss081RecoveredGapTP(price_t gap, const std::string& name = "ISS081_RecoveredGapTP")
    : StoplossBase(name), m_gap(gap) {}

    virtual StoplossPtr _clone() override {
        return make_shared<Iss081RecoveredGapTP>(m_gap, name());
    }

    virtual price_t getPrice(const Datetime& datetime, price_t) override {
        return levelOf(datetime);
    }

    virtual price_t getShortPrice(const Datetime& datetime, price_t) override {
        return levelOf(datetime);
    }

private:
    price_t levelOf(const Datetime& datetime) {
        size_t pos = m_kdata.getPos(datetime);
        if (pos == Null<size_t>()) {
            return 0.0;
        }
        price_t level = m_kdata[pos].closePrice + m_gap;
        Stock stock = m_kdata.getStock();
        return roundEx(level, stock.isNull() ? 2 : stock.precision());
    }

    price_t m_gap;
};

// A take-profit whose long-side level is scripted per date; unlisted dates return 0.0, i.e. no
// take-profit price on that bar
class Iss081ScriptedTP : public StoplossBase {
public:
    Iss081ScriptedTP() : StoplossBase("ISS081_ScriptedTP") {}

    virtual StoplossPtr _clone() override {
        return make_shared<Iss081ScriptedTP>();
    }

    virtual price_t getPrice(const Datetime& datetime, price_t) override {
        auto iter = m_script.find(datetime);
        return iter == m_script.end() ? 0.0 : iter->second;
    }

    void addScript(const Datetime& datetime, price_t price) {
        m_script[datetime] = price;
    }

private:
    std::map<Datetime, price_t> m_script;
};

KData makeWindow(const Stock& stk, uint64_t start, uint64_t end,
                 KQuery::RecoverType recover = KQuery::FORWARD) {
    return stk.getKData(KQueryByDate(Datetime(start), Datetime(end), KQuery::DAY, recover));
}

SYSPtr makeTpSys(const Stock& stk, const StoplossPtr& tp, int tp_delay_n = 1,
                 const Datetime& buyDate = BUY_DATE, bool shortMode = false) {
    SYSPtr sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<int>("tp_delay_n", tp_delay_n);
    if (shortMode) {
        sys->setParam<bool>("support_borrow_stock", true);
    }
    // The position is carried across separate run windows (as WalkForwardSystem does), so the TM
    // must be shared and not reset between runs
    sys->setParam<bool>("shared_tm", true);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(make_shared<Iss081BuySG>(buyDate, shortMode));
    sys->setMM(MM_FixedCount(100));
    sys->setTP(tp);
    return sys;
}

const TradeRecord* findTpFirstTrade(const TradeRecordList& trs, BUSINESS business) {
    auto iter = std::find_if(trs.begin(), trs.end(),
                             [&](const TradeRecord& tr) { return tr.business == business; });
    return iter == trs.end() ? nullptr : &(*iter);
}

}  // namespace

/**
 * @defgroup test_SYS_Simple_tp test_SYS_Simple_tp
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points (a take-profit just below the adjusted close must not fire against the
 *       original close, ISS-081) */
TEST_CASE("test_SYS_Simple_tp_no_false_trigger_under_backward_recover") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    // BACKWARD window straddling the 2000-07-06 ex-date: after that date the adjusted prices are
    // about 0.15 higher than the original ones (the adjustment is anchored at the window start).
    // The window ends on 07-12 while the raw price rises monotonically, so a level mapped back to
    // sit 0.05 below each raw close can never be reached
    KData kdata = makeWindow(stk, 200006270000LL, 200007130000LL, KQuery::BACKWARD);

    // The level stays 0.05 below the adjusted close on every bar
    SYSPtr sys =
      makeTpSys(stk, make_shared<Iss081RecoveredGapTP>(-0.05), 1, Datetime(200007100000LL));
    sys->run(stk, kdata.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    REQUIRE_NE(findTpFirstTrade(tr_list, BUSINESS_BUY), nullptr);

    /** @arg after mapping back, the level is 0.05 below the original close so no take-profit can
     *       fire; comparing the original close directly with the adjusted level (0.10 above the
     *       original close) would produce a false sale on the bar after the buy */
    CHECK_EQ(findTpFirstTrade(tr_list, BUSINESS_SELL), nullptr);
    CHECK_UNARY(sys->getTM()->have(stk));
}

/** @par Test points (a short take-profit just above the adjusted close must not fire against the
 *       original close, ISS-081) */
TEST_CASE("test_SYS_Simple_short_tp_no_false_trigger_under_forward_recover") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    // FORWARD window straddling the 2000-07-06 ex-date: before that date the adjusted prices are
    // about 0.15 lower than the original ones (the adjustment is anchored at the window end)
    KData kdata = makeWindow(stk, 200006270000LL, 200007120000LL, KQuery::FORWARD);

    // The short is opened on 07-03 (raw close 23.23); the level stays 0.05 above the adjusted
    // close on every bar, i.e. 0.05 above the original close after mapping back
    SYSPtr sys =
      makeTpSys(stk, make_shared<Iss081RecoveredGapTP>(0.05), 1, Datetime(200007030000LL), true);
    sys->run(stk, kdata.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    REQUIRE_NE(findTpFirstTrade(tr_list, BUSINESS_SELL_SHORT), nullptr);

    /** @arg no take-profit cover is triggered; comparing the original close directly with the
     *       adjusted level (0.10 below the original close) would produce a false cover on 07-05,
     *       when the raw price dips slightly below the opening price */
    CHECK_EQ(findTpFirstTrade(tr_list, BUSINESS_BUY_SHORT), nullptr);
    CHECK_UNARY(sys->getTM()->haveShort(stk));
}

/** @par Test points (a take-profit above the close fires at the first profitable eligible bar in
 *       the original coordinate, ISS-081) */
TEST_CASE("test_SYS_Simple_tp_trigger_under_forward_recover") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData kdata = makeWindow(stk, 200006200000LL, 200007200000LL);

    // The level is 0.10 above the adjusted close: eligible every bar, but the profit gate blocks
    // the first two bars (original close below the entry 23.42), so the cover happens on 06-30
    SYSPtr sys = makeTpSys(stk, make_shared<Iss081RecoveredGapTP>(0.10));
    sys->run(stk, kdata.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* buy_tr = findTpFirstTrade(tr_list, BUSINESS_BUY);
    const TradeRecord* sell_tr = findTpFirstTrade(tr_list, BUSINESS_SELL);
    REQUIRE_NE(buy_tr, nullptr);
    REQUIRE_NE(sell_tr, nullptr);

    /** @arg the cover is marked PART_TAKEPROFIT and happens on 2000-06-30 */
    CHECK_EQ(buy_tr->datetime, BUY_DATE);
    CHECK_EQ(sell_tr->from, PART_TAKEPROFIT);
    CHECK_EQ(sell_tr->datetime, Datetime(200006300000LL));
    CHECK_EQ(sell_tr->number, buy_tr->number);
    CHECK_UNARY_FALSE(sys->getTM()->have(stk));
}

/** @par Test points (the tp_delay_n gate counts from the window start when the position was opened
 *       in an earlier KData window, ISS-133) */
TEST_CASE("test_SYS_Simple_tp_delay_when_opened_before_window") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];

    // First window: buy on 2000-06-27; with tp_delay_n=5 the take-profit cannot trigger before the
    // window ends
    SYSPtr sys = makeTpSys(stk, make_shared<Iss081RecoveredGapTP>(0.10), 5);
    KData kdata1 = makeWindow(stk, 200006200000LL, 200007010000LL, KQuery::NO_RECOVER);
    sys->run(stk, kdata1.getQuery());

    TradeRecordList tr_list1 = sys->getTM()->getTradeList();
    const TradeRecord* buy_tr = findTpFirstTrade(tr_list1, BUSINESS_BUY);
    REQUIRE_NE(buy_tr, nullptr);
    CHECK_UNARY(sys->getTM()->have(stk));

    // Second window starts after the opening date: the opening date is outside this KData, so the
    // delay is counted from the window start instead of underflowing size_t
    KData kdata2 = makeWindow(stk, 200007100000LL, 200007200000LL, KQuery::NO_RECOVER);
    sys->run(stk, kdata2.getQuery(), false, false);

    // Keep a copy: getTradeList() may return a temporary, so the pointer must not outlive it
    TradeRecordList tr_list2 = sys->getTM()->getTradeList();
    const TradeRecord* sell_tr = findTpFirstTrade(tr_list2, BUSINESS_SELL);
    REQUIRE_NE(sell_tr, nullptr);

    /** @arg the cover waits until the 5th bar of the new window (2000-07-14), not the 1st */
    CHECK_EQ(sell_tr->from, PART_TAKEPROFIT);
    CHECK_EQ(sell_tr->datetime, Datetime(200007140000LL));
    CHECK_UNARY_FALSE(sys->getTM()->have(stk));
}

/** @par Test points (tp_monotonic=true keeps the highest prior level while tp_monotonic=false uses
 *       the current bar level, ISS-081) */
TEST_CASE("test_SYS_Simple_tp_monotonic_clamp_switch") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    // Raw prices (NO_RECOVER): buy 07-10 at 23.11; closes 07-11 23.46, 07-12 23.60, 07-13 23.44
    KData kdata = makeWindow(stk, 200007100000LL, 200007150000LL, KQuery::NO_RECOVER);

    auto makeScriptedTp = []() {
        auto tp = make_shared<Iss081ScriptedTP>();
        // Just below each close while the price rises, then far below on the 07-13 pullback
        tp->addScript(Datetime(200007110000LL), 23.45);
        tp->addScript(Datetime(200007120000LL), 23.59);
        tp->addScript(Datetime(200007130000LL), 23.00);
        return tp;
    };

    SYSPtr sys_clamp = makeTpSys(stk, makeScriptedTp(), 1, Datetime(200007100000LL));
    sys_clamp->run(stk, kdata.getQuery());
    const TradeRecordList& clamp_list = sys_clamp->getTM()->getTradeList();
    const TradeRecord* clamp_sell = findTpFirstTrade(clamp_list, BUSINESS_SELL);
    REQUIRE_NE(clamp_sell, nullptr);

    /** @arg with the default tp_monotonic=true the 23.59 level is kept on 07-13 and the pullback
     *       to 23.44 triggers the take-profit */
    CHECK_EQ(clamp_sell->from, PART_TAKEPROFIT);
    CHECK_EQ(clamp_sell->datetime, Datetime(200007130000LL));

    SYSPtr sys_free = makeTpSys(stk, makeScriptedTp(), 1, Datetime(200007100000LL));
    sys_free->setParam<bool>("tp_monotonic", false);
    sys_free->run(stk, kdata.getQuery());
    const TradeRecordList& free_list = sys_free->getTM()->getTradeList();

    /** @arg with tp_monotonic=false the lowered 23.00 level of 07-13 is used, so no take-profit
     *       fires and the position is kept */
    CHECK_EQ(findTpFirstTrade(free_list, BUSINESS_SELL), nullptr);
    CHECK_UNARY(sys_free->getTM()->have(stk));
}

/** @par Test points (the buy-price ratchet anchor and the take-profit interplay across the
 *       ex-dividend date, ISS-081) */
TEST_CASE("test_SYS_Simple_tp_ratchet_anchor_across_ex_date") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    // FORWARD window straddling the 2000-07-06 ex-date (cash 0.15): before that date the adjusted
    // prices are about 0.15 lower than the original ones. Raw closes: entry 06-30 at 23.55, then
    // a pullback, and 07-11 23.46 / 07-12 23.60 after the ex-date
    KData kdata = makeWindow(stk, 200006200000LL, 200007150000LL, KQuery::FORWARD);

    // The trailing level stays 0.05 below the close of its own (adjusted) coordinate; the
    // ex-dividend lowers the position cost to 23.40, so the profit gate passes from 07-11 on.
    // The buy-price anchor (23.55) dominates the ratchet because no bar level exceeds it
    SYSPtr sys =
      makeTpSys(stk, make_shared<Iss081RecoveredGapTP>(-0.05), 1, Datetime(200006300000LL));
    sys->run(stk, kdata.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* buy_tr = findTpFirstTrade(tr_list, BUSINESS_BUY);
    const TradeRecord* sell_tr = findTpFirstTrade(tr_list, BUSINESS_SELL);
    REQUIRE_NE(buy_tr, nullptr);
    REQUIRE_NE(sell_tr, nullptr);

    /** @arg the anchor stays at the entry price 23.55 in the original coordinate across the
     *       ex-date (the indicator levels never exceed it), and the recovery to 23.46 with the
     *       ex-dividend-lowered cost 23.40 triggers the take-profit on 2000-07-11 */
    CHECK_EQ(sell_tr->from, PART_TAKEPROFIT);
    CHECK_EQ(sell_tr->datetime, Datetime(200007110000LL));
    CHECK_EQ(sell_tr->number, buy_tr->number);
    CHECK_UNARY_FALSE(sys->getTM()->have(stk));
}

/** @} */
