/*
 * test_SYS_margin_call.cpp
 *
 *  Created on: 2026-11-10
 *      Author: fasiondog
 *
 * The trading-system side of the margin accounting: the forced liquidation queued by a daily
 * settlement, executed at the next open, the expiry flattening at the last registered bar, and the
 * same hooks on the aggregate (the parent settlement of every close and its whole-book
 * liquidation).
 */

#include "doctest/doctest.h"
#include "test_sys.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crtTM.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_Zero.h>
#include <hikyuu/trade_manage/margin/MarginBase.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>
#include <hikyuu/trade_sys/signal/crt/SG_Manual.h>
#include <hikyuu/trade_sys/signal/crt/SG_AllwaysBuy.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>

using namespace hku;

/**
 * @defgroup test_SYS_margin_call test_SYS_margin_call
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

namespace {

/** A margin model with an independent maintenance rate. The built-in MG_Fixed requires
 *  maintain_rate <= rate and a settlement then can never call the account, so a controllable model
 *  is the only way to reach the forced liquidation path. */
class MG_TestSysCall : public MarginBase {
    MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MG_TestSysCall() : MarginBase("MG_TestSysCall") {
        setParam<double>("rate", 0.1);
        setParam<double>("maintain_rate", 0.1);
    }

    bool needBorrowAsset(const Stock& stock) const override {
        return false;
    }

    price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price, double num,
                          MarginSide side) const override {
        return getParam<double>("rate") * price * num * stock.unit();
    }

    price_t getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                              double num, MarginSide side) const override {
        return getParam<double>("maintain_rate") * price * num * stock.unit();
    }

    MarginPtr _clone() override {
        return make_shared<MG_TestSysCall>();
    }
};

/** A margin account whose maintenance requirement is the multiplier of the notional given */
TradeManagerPtr crtSysTM(price_t init_cash, double maintain_rate, bool force_liquidate) {
    auto margin = make_shared<MG_TestSysCall>();
    margin->setParam<double>("maintain_rate", maintain_rate);
    auto tm = crtTM(Datetime(199001010000LL), init_cash, TC_Zero(), "SYS", margin);
    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<bool>("force_liquidate", force_liquidate);
    return tm;
}

/** A synthetic instrument whose registered history ends exactly at its last bar, so the expiry
 *  gate is decided by the data itself instead of the security end date of the base info */
KData expiryKData() {
    Stock stock("TEST", "EXPIRY", "Expiry");
    stock.setKRecordList({KRecord(Datetime(202001020000), 10.0, 11.0, 9.0, 10.0, 100.0, 100.0),
                          KRecord(Datetime(202001030000), 10.0, 11.0, 9.0, 10.5, 100.0, 100.0),
                          KRecord(Datetime(202001060000), 10.5, 12.0, 10.0, 11.0, 100.0, 100.0)});
    return stock.getKData(KQuery(0));
}

}  // namespace

/** @par Test points: a settlement that finds the equity below the maintenance requirement queues
 *  the liquidation, and the trading system flattens the account at the open of the next bar */
TEST_CASE("test_SYS_margin_call_liquidates_at_next_open") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    KData kdata = rb.getKData(KQueryByIndex(-5, Null<int64_t>(), KQuery::DAY));
    REQUIRE_UNARY(kdata.size() >= 4);
    const size_t i = 0;
    const Datetime dt_buy = kdata[i].datetime;
    const Datetime dt_open = kdata[i + 1].datetime;
    REQUIRE_UNARY(kdata[i].closePrice > 0.0);
    REQUIRE_UNARY(kdata[i + 1].openPrice > 0.0);

    // The opening margin is a tenth of the notional while the requirement is a hundred times it:
    // the entry itself is affordable and the same day's settlement is in a margin call
    TMPtr tm = crtSysTM(10000.0, 100.0, true);
    SGPtr sg = SG_Manual();
    sg->_addBuySignal(dt_buy);

    SYSPtr sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setTM(tm);
    sys->setSG(sg);
    sys->setMM(MM_FixedCount(1));
    sys->setTO(kdata);
    sys->readyForRun();

    MomentResult r = sys->runMoment(dt_buy);
    REQUIRE_EQ(r.tradesOnClose.size(), 1u);
    CHECK_EQ(r.tradesOnClose[0].business, BUSINESS_BUY);
    CHECK_UNARY(tm->have(rb));

    /** @arg The settlement of the entry day leaves the account in a margin call */
    CHECK_UNARY(tm->isSettled(dt_buy));
    CHECK_UNARY(tm->isForcedLiquidationQueued(dt_buy));

    /** @arg The next open flattens the whole holding at the open price of that bar */
    MomentResult next = sys->runMoment(dt_open);
    REQUIRE_EQ(next.tradesOnOpen.size(), 1u);
    CHECK_EQ(next.tradesOnOpen[0].business, BUSINESS_SELL);
    CHECK_UNARY(next.tradesOnOpen[0].datetime == dt_open);
    CHECK_EQ(next.tradesOnOpen[0].realPrice, doctest::Approx(kdata[i + 1].openPrice));
    CHECK_EQ(next.tradesOnOpen[0].from, PART_INVALID);
    CHECK_EQ(next.tradesOnOpen[0].number, doctest::Approx(1.0));

    /** @arg Nothing is held afterwards, so the settlement of that close clears the queue */
    CHECK_UNARY(!tm->have(rb));
    CHECK_UNARY(!tm->isForcedLiquidationQueued(dt_open));
    CHECK_EQ(tm->getPosition(dt_open, rb).margin, doctest::Approx(0.0));
}

/** @par Test points: a delayed entry queued on the close of the entry day must not rebuild the
 *  position the forced liquidation has just closed at the next open */
TEST_CASE("test_SYS_margin_call_drops_pending_entry") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    KData kdata = rb.getKData(KQueryByIndex(-5, Null<int64_t>(), KQuery::DAY));
    REQUIRE_UNARY(kdata.size() >= 4);
    const Datetime dt_signal = kdata[0].datetime;
    const Datetime dt_fill = kdata[1].datetime;
    const Datetime dt_liquidate = kdata[2].datetime;

    TMPtr tm = crtSysTM(10000.0, 100.0, true);
    SGPtr sg = SG_Manual();
    sg->setParam<bool>("alternate", false);
    sg->_addBuySignal(dt_signal);
    // A second entry intent on the very close whose settlement calls the account
    sg->_addBuySignal(dt_fill);

    SYSPtr sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", true);
    sys->setParam<bool>("sell_delay", false);
    sys->setTM(tm);
    sys->setSG(sg);
    sys->setMM(MM_FixedCount(1));
    sys->setTO(kdata);
    sys->readyForRun();

    /** @arg The delayed entry is queued on the signal day and filled at the next open */
    CHECK_UNARY(sys->runMoment(dt_signal).tradesOnOpen.empty());
    CHECK_UNARY(!tm->have(rb));
    MomentResult filled = sys->runMoment(dt_fill);
    REQUIRE_EQ(filled.tradesOnOpen.size(), 1u);
    CHECK_EQ(filled.tradesOnOpen[0].business, BUSINESS_BUY);
    CHECK_UNARY(tm->have(rb));
    CHECK_UNARY(tm->isForcedLiquidationQueued(dt_fill));

    /** @arg The liquidation at the next open drops the queued entry instead of re-entering */
    MomentResult next = sys->runMoment(dt_liquidate);
    CHECK_UNARY(!next.tradesOnOpen.empty());
    CHECK_EQ(countBusiness(tm->getTradeList(), BUSINESS_BUY), 1);
    CHECK_EQ(countBusiness(tm->getTradeList(), BUSINESS_SELL), 1);
    CHECK_UNARY(!tm->have(rb));
}

/** @par Test points: a margin holding past its last registered bar is flattened at the close of
 *  that bar (G12), before the settlement, so the closing trade realizes the profit */
TEST_CASE("test_SYS_expiry_liquidates_at_last_bar") {
    KData kdata = expiryKData();
    Stock stock = kdata.getStock();
    REQUIRE_UNARY(!stock.isNull());
    REQUIRE_EQ(kdata.size(), 3u);
    const size_t last = kdata.size() - 1;
    REQUIRE_UNARY(kdata[last].datetime == stock.lastDatetime());

    // A solvent account: the expiry is a property of the data, not of the margin call state
    TMPtr tm = crtSysTM(10000.0, 0.05, false);
    SGPtr sg = SG_Manual();
    sg->_addBuySignal(kdata[0].datetime);

    SYSPtr sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setTM(tm);
    sys->setSG(sg);
    sys->setMM(MM_FixedCount(100));
    sys->setTO(kdata);
    sys->readyForRun();

    MomentResult entry = sys->runMoment(kdata[0].datetime);
    REQUIRE_EQ(entry.tradesOnClose.size(), 1u);
    CHECK_EQ(entry.tradesOnClose[0].business, BUSINESS_BUY);
    sys->runMoment(kdata[1].datetime);

    /** @arg The holding is still open on the bar before the last one */
    REQUIRE_UNARY(tm->have(stock));
    CHECK_EQ(tm->getFunds(kdata[1].datetime, KQuery::DAY).net_assets(), doctest::Approx(10050.0));

    MomentResult tail = sys->runMoment(kdata[last].datetime);
    /** @arg The last bar flattens it at that bar's close, not at a next open that never comes */
    REQUIRE_EQ(tail.tradesOnClose.size(), 1u);
    CHECK_EQ(tail.tradesOnClose[0].business, BUSINESS_SELL);
    CHECK_UNARY(tail.tradesOnClose[0].datetime == kdata[last].datetime);
    CHECK_EQ(tail.tradesOnClose[0].realPrice, doctest::Approx(kdata[last].closePrice));
    CHECK_EQ(tail.tradesOnClose[0].from, PART_INVALID);
    CHECK_EQ(tail.tradesOnClose[0].number, doctest::Approx(100.0));

    /** @arg The sale realizes the profit: nothing is held and the equity is the entry plus it */
    CHECK_UNARY(!tm->have(stock));
    CHECK_EQ(tm->getPosition(kdata[last].datetime, stock).margin, doctest::Approx(0.0));
    CHECK_EQ(tm->getFunds(kdata[last].datetime, KQuery::DAY).net_assets(),
             doctest::Approx(10100.0));
}

/** @par Test points: the aggregate settles its own account at the close of every driven bar, one
 *  posted record per holding per bar, so the mark to market and the basis roll follow the chart */
TEST_CASE("test_MultiSystem_settles_parent_book_each_close") {
    Stock stk = StockManager::instance()["SHFRB0000"];
    REQUIRE_UNARY(!stk.isNull());
    KData kdata = stk.getKData(KQueryByIndex(-6, Null<int64_t>(), KQuery::DAY));
    REQUIRE_GE(kdata.size(), 4u);

    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtSysTM(100000.0, 0.05, true));
    ms->getAF()->setParam<double>("max_total_position", 0.0);
    ms->getAF()->setParam<double>("max_single_position", 0.0);
    ms->getAF()->setParam<double>("fixed_amount", 15000.0);
    auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_AllwaysBuy());
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setTO(kdata);
    ms->add(sys);
    ms->setAdjustDates({kdata[0].datetime});
    ms->run(kdata);

    TMPtr tm = ms->getTM();
    TradeRecordList trades = tm->getTradeList();
    TradeRecord entry;
    int settle_count = 0;
    for (const auto& tr : trades) {
        if (tr.business == BUSINESS_BUY) {
            entry = tr;
        }
        if (tr.business == BUSINESS_SETTLEMENT) {
            ++settle_count;
        }
    }

    /** @arg The parent bought on the only rebalancing close */
    REQUIRE_UNARY(!entry.isNull());
    CHECK_UNARY(entry.datetime == kdata[0].datetime);
    const double lots = entry.number;
    REQUIRE_GT(lots, 0.0);

    /** @arg Every driven bar posts its own settlement, marked at that bar's close */
    REQUIRE_EQ(settle_count, (int)kdata.size());
    for (size_t i = 0; i < kdata.size(); ++i) {
        TradeRecord settled;
        for (const auto& tr : trades) {
            if (tr.business == BUSINESS_SETTLEMENT && tr.datetime == kdata[i].datetime) {
                settled = tr;
            }
        }
        REQUIRE_UNARY(!settled.isNull());
        CHECK_EQ(settled.number, doctest::Approx(lots));
        CHECK_EQ(settled.realPrice, doctest::Approx(kdata[i].closePrice));
    }

    /** @arg The solvent account keeps the holding, its last close is settled and no call is armed
     */
    CHECK_UNARY(tm->isSettled(kdata.back().datetime));
    CHECK_UNARY(!tm->isForcedLiquidationQueued(kdata.back().datetime));
    CHECK_UNARY(tm->have(stk));

    /** @arg The basis is rolled to the last close and the equity is the entry plus the notional
     *  gain (zero costs) */
    PositionRecord pos = tm->getPosition(kdata.back().datetime, stk);
    CHECK_EQ(pos.number, doctest::Approx(lots));
    CHECK_GT(pos.margin, 0.0);
    CHECK_EQ(pos.settleBasis, doctest::Approx(lots * kdata.back().closePrice * stk.unit()));
    CHECK_EQ(
      tm->getFunds(kdata.back().datetime, KQuery::DAY).net_assets(),
      doctest::Approx(100000.0 + (kdata.back().closePrice - entry.realPrice) * lots * stk.unit()));
}

/** @par Test points: the margin call armed by the parent settlement of one close flattens the whole
 *  parent book at the next open, and the shadow book of the member is cleared with it */
TEST_CASE("test_MultiSystem_margin_call_liquidates_parent_book") {
    Stock stk = StockManager::instance()["SHFRB0000"];
    REQUIRE_UNARY(!stk.isNull());
    KData kdata = stk.getKData(KQueryByIndex(-6, Null<int64_t>(), KQuery::DAY));
    REQUIRE_GE(kdata.size(), 4u);

    // The maintenance requirement is the hundredfold of the notional, so the settlement of the
    // entry day is in a margin call whatever the price does
    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtSysTM(20000.0, 100.0, true));
    ms->setParam<bool>("support_borrow_cash", true);
    ms->getAF()->setParam<double>("max_total_position", 0.0);
    ms->getAF()->setParam<double>("max_single_position", 0.0);
    ms->getAF()->setParam<double>("fixed_amount", 15000.0);
    auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_AllwaysBuy());
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setTO(kdata);
    ms->add(sys);
    ms->setAdjustDates({kdata[0].datetime});
    ms->run(kdata);

    TMPtr tm = ms->getTM();
    TradeRecordList trades = tm->getTradeList();
    TradeRecord entry;
    TradeRecord sell;
    for (const auto& tr : trades) {
        if (tr.business == BUSINESS_BUY) {
            entry = tr;
        }
        if (tr.business == BUSINESS_SELL) {
            sell = tr;
        }
    }

    /** @arg The parent book was opened on the rebalancing close and its settlement posted */
    REQUIRE_UNARY(!entry.isNull());
    CHECK_EQ(countBusiness(trades, BUSINESS_BUY), 1);
    CHECK_EQ(countBusiness(trades, BUSINESS_SETTLEMENT), 1);
    const double lots = entry.number;

    /** @arg The next open flattens it at that open, and nothing re-enters afterwards */
    REQUIRE_UNARY(!sell.isNull());
    CHECK_EQ(countBusiness(trades, BUSINESS_SELL), 1);
    CHECK_UNARY(sell.datetime == kdata[1].datetime);
    CHECK_EQ(sell.realPrice, doctest::Approx(kdata[1].openPrice));
    CHECK_EQ(sell.number, doctest::Approx(lots));
    CHECK_EQ(sell.remark, std::string("MARGIN_CALL"));
    CHECK_UNARY(!tm->have(stk));
    CHECK_UNARY(!tm->isForcedLiquidationQueued(kdata.back().datetime));

    /** @arg The shadow book of the member is cleared at the same open, so no orphan target survives
     */
    TradeRecord shadow_sell;
    for (const auto& tr : sys->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_SELL) {
            shadow_sell = tr;
        }
    }
    REQUIRE_UNARY(!shadow_sell.isNull());
    CHECK_UNARY(shadow_sell.datetime == kdata[1].datetime);
    CHECK_EQ(shadow_sell.remark, std::string("MARGIN_CALL"));
}

/** @par Test points: on the shared account (mode C) the liquidation of the aggregate also voids the
 *  entry request a member queued on the close that armed the call */
TEST_CASE("test_MultiSystem_margin_call_drops_shared_entry") {
    Stock stk = StockManager::instance()["SHFRB0000"];
    REQUIRE_UNARY(!stk.isNull());
    KData kdata = stk.getKData(KQueryByIndex(-6, Null<int64_t>(), KQuery::DAY));
    REQUIRE_GE(kdata.size(), 4u);

    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtSysTM(20000.0, 100.0, true));
    ms->setMode("C");
    ms->setTradeOnClose(false);
    ms->setParam<bool>("support_borrow_cash", true);
    // The member trades directly on the shared account with a delayed entry (mode C maps the delay
    // of the request onto trade_on_close)
    auto sys =
      SYS_Simple(crtTM(), MM_FixedCount(1), EnvironmentPtr(), ConditionPtr(), SG_AllwaysBuy());
    sys->setParam<bool>("sell_delay", false);
    sys->setTO(kdata);
    ms->add(sys);
    ms->setTO(kdata);
    ms->setAdjustDates({kdata[0].datetime});
    ms->readyForRun();

    /** @arg The entry queued on the first close is filled at the next open */
    CHECK_UNARY(ms->runMoment(kdata[0].datetime).tradesOnOpen.empty());
    MomentResult filled = ms->runMoment(kdata[1].datetime);
    REQUIRE_EQ(filled.tradesOnOpen.size(), 1u);
    CHECK_EQ(filled.tradesOnOpen[0].business, BUSINESS_BUY);
    REQUIRE_UNARY(ms->getTM()->have(stk));

    /** @arg The settlement of that close arms the call, and the next open flattens the shared book
     *  instead of filling the member entry queued on the same close */
    REQUIRE_UNARY(ms->getTM()->isForcedLiquidationQueued(kdata[1].datetime));
    MomentResult next = ms->runMoment(kdata[2].datetime);
    REQUIRE_EQ(next.tradesOnOpen.size(), 1u);
    CHECK_EQ(next.tradesOnOpen[0].business, BUSINESS_SELL);
    CHECK_EQ(next.tradesOnOpen[0].remark, std::string("MARGIN_CALL"));
    CHECK_EQ(next.tradesOnOpen[0].realPrice, doctest::Approx(kdata[2].openPrice));
    CHECK_UNARY(!ms->getTM()->have(stk));
    CHECK_EQ(countBusiness(ms->getTM()->getTradeList(), BUSINESS_BUY), 1);
    CHECK_UNARY(!ms->getTM()->isForcedLiquidationQueued(kdata[2].datetime));
}

/** @} */
