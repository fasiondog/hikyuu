/*
 * test_TradeManagerSettlement.cpp
 *
 *  Created on: 2026-11-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/KQuery.h>
#include <hikyuu/trade_manage/crtTM.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_Zero.h>
#include <hikyuu/trade_manage/margin/MarginBase.h>

using namespace hku;

/**
 * @defgroup test_TradeManagerSettlement test_TradeManagerSettlement
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

namespace {

/** A margin model opening without borrowing, the maintenance requirement being a parameter (so a
 *  case can make the account solvent or already in a margin call without moving the price) */
class MG_TestSettleCall : public MarginBase {
    MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MG_TestSettleCall() : MarginBase("MG_TestSettleCall") {
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
        return make_shared<MG_TestSettleCall>();
    }
};

/** A margin account (Phase B accounting) whose model reports the maintenance rate given */
TradeManagerPtr crtSettleTM(const Datetime& datetime, price_t init_cash, double maintain_rate,
                            bool force_liquidate) {
    auto tm = crtTM(datetime, init_cash, TC_Zero(), "SYS", make_shared<MG_TestSettleCall>());
    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<bool>("force_liquidate", force_liquidate);
    tm->margin()->setParam<double>("maintain_rate", maintain_rate);
    return tm;
}

}  // namespace

/** @par Test points: the daily settlement marks a long holding to the settlement price, posts the
 *  mark-to-market profit into the cash and relocks the margin at that price */
TEST_CASE("test_TradeManager_settlement_long") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    KQuery query = KQueryByDate(dt, Datetime(200904100000), KQuery::DAY);
    KData bars = rb.getKData(query);
    REQUIRE_UNARY(bars.size() >= 3);
    Datetime dt2 = bars[1].datetime;
    price_t close2 = bars[1].closePrice;
    REQUIRE_UNARY(close2 > 0.0);
    REQUIRE_UNARY(close2 != close);

    double unit = rb.unit();
    double number = 10.0;
    price_t nominal1 = roundEx(close * number * unit, 2);
    price_t lock1 = roundEx(0.1 * nominal1, 2);
    price_t nominal2 = roundEx(close2 * number * unit, 2);
    price_t lock2 = roundEx(0.1 * nominal2, 2);
    REQUIRE_UNARY(lock1 > 0.0);
    REQUIRE_UNARY(lock2 > 0.0);

    auto tm = crtSettleTM(dt, 1000000.0, 0.1, false);
    REQUIRE_EQ(tm->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business, BUSINESS_BUY);

    /** @arg Nothing is settled before the call, and the basis is still the opening notional */
    CHECK_UNARY(!tm->isSettled(dt));
    CHECK_UNARY(!tm->isSettled(dt2));
    CHECK_EQ(tm->getPosition(dt, rb).settleBasis, doctest::Approx(nominal1));
    CHECK_EQ(tm->getPosition(dt, rb).margin, doctest::Approx(lock1));

    /** @arg The settlement of the second day reports success and is then idempotent */
    REQUIRE_UNARY(tm->settlement(dt2));
    CHECK_UNARY(tm->isSettled(dt2));
    CHECK_UNARY(!tm->isSettled(dt));

    /** @arg The basis moves to the settlement price and the lock is recomputed on it */
    CHECK_EQ(tm->getPosition(dt2, rb).settleBasis, doctest::Approx(nominal2));
    CHECK_EQ(tm->getPosition(dt2, rb).margin, doctest::Approx(lock2));

    /** @arg The cash carries the mark-to-market profit net of the margin difference */
    price_t expect_cash = roundEx(1000000.0 - lock1, 2);
    expect_cash = roundEx(expect_cash + (nominal2 - nominal1), 2);
    expect_cash = roundEx(expect_cash - roundEx(lock2 - lock1, 2), 2);
    CHECK_EQ(tm->currentCash(), doctest::Approx(expect_cash));

    /** @arg The record carries the settlement price, the signed quantity and the new lock */
    const TradeRecordList& list = tm->getRefTradeList();
    REQUIRE_UNARY(!list.empty());
    CHECK_EQ(list.back().business, BUSINESS_SETTLEMENT);
    CHECK_UNARY(list.back().datetime == dt2);
    CHECK_EQ(list.back().realPrice, doctest::Approx(close2));
    CHECK_EQ(list.back().number, doctest::Approx(number));
    CHECK_EQ(list.back().margin, doctest::Approx(lock2));
    CHECK_EQ(list.back().cash, doctest::Approx(tm->currentCash()));

    /** @arg The equity follows the mark-to-market profit only, the lock staying inside it */
    CHECK_EQ(tm->getFunds(dt2, KQuery::DAY).net_assets(),
             doctest::Approx(1000000.0 + nominal2 - nominal1));

    /** @arg Settling the same moment again posts nothing */
    size_t count = list.size();
    REQUIRE_UNARY(tm->settlement(dt2));
    CHECK_EQ(tm->getRefTradeList().size(), count);
    CHECK_EQ(tm->currentCash(), doctest::Approx(expect_cash));

    /** @arg A moment before the last trade is refused */
    CHECK_UNARY(!tm->settlement(dt));
}

/** @par Test points: the short side settles as a liability, its profit being posted when the price
 *  falls on the settlement price */
TEST_CASE("test_TradeManager_settlement_short") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    KQuery query = KQueryByDate(dt, Datetime(200904100000), KQuery::DAY);
    KData bars = rb.getKData(query);
    REQUIRE_UNARY(bars.size() >= 3);
    Datetime dt2 = bars[1].datetime;
    price_t close2 = bars[1].closePrice;
    REQUIRE_UNARY(close2 > 0.0);

    double unit = rb.unit();
    double number = 10.0;
    price_t nominal1 = roundEx(close * number * unit, 2);
    price_t lock1 = roundEx(0.1 * nominal1, 2);
    price_t nominal2 = roundEx(close2 * number * unit, 2);
    price_t lock2 = roundEx(0.1 * nominal2, 2);

    auto tm = crtSettleTM(dt, 1000000.0, 0.1, false);
    REQUIRE_EQ(tm->sellShort(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_SELL_SHORT);

    REQUIRE_UNARY(tm->settlement(dt2));

    /** @arg The short basis is marked to the settlement price like the long one */
    CHECK_EQ(tm->getShortPosition(rb).settleBasis, doctest::Approx(nominal2));
    CHECK_EQ(tm->getShortPosition(rb).margin, doctest::Approx(lock2));

    /** @arg The cash carries the short-side profit against the margin difference */
    price_t expect_cash = roundEx(1000000.0 - lock1, 2);
    expect_cash = roundEx(expect_cash + (nominal1 - nominal2), 2);
    expect_cash = roundEx(expect_cash - roundEx(lock2 - lock1, 2), 2);
    CHECK_EQ(tm->currentCash(), doctest::Approx(expect_cash));

    /** @arg The sign of the settled quantity is the side of the holding */
    const TradeRecordList& list = tm->getRefTradeList();
    REQUIRE_UNARY(!list.empty());
    CHECK_EQ(list.back().business, BUSINESS_SETTLEMENT);
    CHECK_EQ(list.back().number, doctest::Approx(-number));

    /** @arg The equity is the capital plus the short-side profit */
    CHECK_EQ(tm->getFunds(dt2, KQuery::DAY).net_assets(),
             doctest::Approx(1000000.0 + nominal1 - nominal2));
}

/** @par Test points: the settled books are reproduced by both read-only replays and by a backfill
 *  of the same records */
TEST_CASE("test_TradeManager_settlement_replay") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    KQuery query = KQueryByDate(dt, Datetime(200904100000), KQuery::DAY);
    KData bars = rb.getKData(query);
    REQUIRE_UNARY(bars.size() >= 3);
    Datetime dt2 = bars[1].datetime;

    double unit = rb.unit();
    double number = 10.0;
    price_t nominal1 = roundEx(close * number * unit, 2);
    price_t lock1 = roundEx(0.1 * nominal1, 2);

    auto tm = crtSettleTM(dt, 1000000.0, 0.1, false);
    REQUIRE_EQ(tm->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business, BUSINESS_BUY);
    REQUIRE_UNARY(tm->settlement(dt2));

    price_t live_net = tm->getFunds(dt2, KQuery::DAY).net_assets();
    price_t live_cash = tm->currentCash();
    price_t live_basis = tm->getPosition(dt2, rb).settleBasis;
    price_t live_lock = tm->getPosition(dt2, rb).margin;

    // A later record pushes the settled moment behind lastDatetime(), so the query below is served
    // by the record replay instead of the live books -- the branch this case is about
    price_t cash_before_checkin = live_cash;
    REQUIRE_UNARY(tm->checkin(bars[2].datetime, 1000.0));
    CHECK_EQ(tm->currentCash(), doctest::Approx(cash_before_checkin + 1000.0));

    /** @arg The single-date replay of the settled moment agrees with the live books */
    Datetime settled_moment(dt2.year(), dt2.month(), dt2.day(), 23, 59);
    FundsRecord replay = tm->getFunds(settled_moment, KQuery::DAY);
    CHECK_EQ(replay.cash, doctest::Approx(live_cash));
    CHECK_EQ(replay.net_assets(), doctest::Approx(live_net));
    CHECK_EQ(replay.occupied_margin, doctest::Approx(live_lock));

    /** @arg The batch replay keeps the first day on its opening basis and the second on the
     *  settlement one */
    FundsRecordList list = tm->getFundsList({dt, dt2});
    REQUIRE_EQ(list.size(), 2u);
    CHECK_EQ(list[0].occupied_margin, doctest::Approx(lock1));
    CHECK_EQ(list[0].net_assets(), doctest::Approx(1000000.0));
    CHECK_EQ(list[1].occupied_margin, doctest::Approx(live_lock));
    CHECK_EQ(list[1].net_assets(), doctest::Approx(live_net));

    /** @arg Backfilling the records rebuilds the identical settled holding */
    auto copy = crtSettleTM(dt, 1000000.0, 0.1, false);
    for (const auto& tr : tm->getRefTradeList()) {
        if (tr.business == BUSINESS_INIT || tr.business == BUSINESS_BORROW_CASH) {
            continue;
        }
        REQUIRE_UNARY(copy->addTradeRecord(tr));
    }
    CHECK_EQ(copy->currentCash(), doctest::Approx(cash_before_checkin + 1000.0));
    CHECK_EQ(copy->getPosition(dt2, rb).settleBasis, doctest::Approx(live_basis));
    CHECK_EQ(copy->getPosition(dt2, rb).margin, doctest::Approx(live_lock));
    CHECK_EQ(copy->getFunds(dt2, KQuery::DAY).net_assets(), doctest::Approx(live_net));
}

/** @par Test points: a maintenance shortfall warns, queues the forced liquidation only with
 *  force_liquidate on, and the queue clears once the account is flat */
TEST_CASE("test_TradeManager_settlement_margin_call_queue") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    KQuery query = KQueryByDate(dt, Datetime(200904100000), KQuery::DAY);
    KData bars = rb.getKData(query);
    REQUIRE_UNARY(bars.size() >= 3);
    Datetime dt2 = bars[1].datetime;

    double number = 10.0;

    /** @arg A requirement above the equity is a margin call, but nothing is queued by default */
    auto warn_only = crtSettleTM(dt, 1000000.0, 100.0, false);
    REQUIRE_EQ(warn_only->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    REQUIRE_UNARY(warn_only->settlement(dt2));
    CHECK_UNARY(!warn_only->isForcedLiquidationQueued(dt2));

    /** @arg With force_liquidate on the same shortfall arms the queue from the settlement date */
    auto queued = crtSettleTM(dt, 1000000.0, 100.0, true);
    REQUIRE_EQ(queued->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    CHECK_UNARY(!queued->isForcedLiquidationQueued(dt));
    REQUIRE_UNARY(queued->settlement(dt2));
    CHECK_UNARY(queued->isForcedLiquidationQueued(dt2));
    CHECK_UNARY(!queued->isForcedLiquidationQueued(dt));

    /** @arg A solvent account never arms the queue, even with force_liquidate on */
    auto solvent = crtSettleTM(dt, 1000000.0, 0.1, true);
    REQUIRE_EQ(solvent->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    REQUIRE_UNARY(solvent->settlement(dt2));
    CHECK_UNARY(!solvent->isForcedLiquidationQueued(dt2));

    /** @arg A flat account has nothing left to liquidate */
    REQUIRE_EQ(
      queued->sell(dt2, rb, bars[1].closePrice, number, 0.0, 0.0, bars[1].closePrice, PART_SIGNAL)
        .business,
      BUSINESS_SELL);
    CHECK_UNARY(!queued->isForcedLiquidationQueued(dt2));
}

/** @par Test points: an account that never settles (the plain cash books, or the switch turned
 *  off) reports false and posts nothing */
TEST_CASE("test_TradeManager_settlement_disabled") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    /** @arg The plain cash books have nothing to settle */
    auto plain = crtTM(dt, 1000000.0, TC_Zero());
    REQUIRE_EQ(plain->buy(dt, rb, close, 10.0, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    size_t plain_count = plain->getRefTradeList().size();
    CHECK_UNARY(!plain->settlement(dt));
    CHECK_EQ(plain->getRefTradeList().size(), plain_count);
    CHECK_UNARY(!plain->isSettled(dt));

    /** @arg A margin account with settle_with_close off keeps the live valuation (Phase A) */
    auto legacy = crtSettleTM(dt, 1000000.0, 0.1, false);
    legacy->setParam<bool>("settle_with_close", false);
    REQUIRE_EQ(legacy->buy(dt, rb, close, 10.0, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    size_t legacy_count = legacy->getRefTradeList().size();
    CHECK_UNARY(!legacy->settlement(dt));
    CHECK_EQ(legacy->getRefTradeList().size(), legacy_count);
}

/** @par Test points: a moment without a bar falls back to the latest close, the record remark
 *  telling the fallback, and the moment is settled on its own (idempotency is per moment) */
TEST_CASE("test_TradeManager_settlement_no_bar") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    double unit = rb.unit();
    double number = 10.0;
    price_t nominal = roundEx(close * number * unit, 2);
    price_t lock = roundEx(0.1 * nominal, 2);

    auto tm = crtSettleTM(dt, 1000000.0, 0.1, false);
    REQUIRE_EQ(tm->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business, BUSINESS_BUY);

    /** @arg The evening moment carries no bar: the latest close is used and the record says so */
    Datetime evening(dt.year(), dt.month(), dt.day(), 23, 59);
    REQUIRE_UNARY(tm->settlement(evening));
    CHECK_UNARY(tm->isSettled(evening));
    CHECK_UNARY(!tm->isSettled(dt));

    const TradeRecordList& list = tm->getRefTradeList();
    REQUIRE_UNARY(!list.empty());
    CHECK_EQ(list.back().business, BUSINESS_SETTLEMENT);
    CHECK_UNARY(!list.back().remark.empty());

    /** @arg The fallback price is the last close, so neither the basis nor the lock moves */
    CHECK_EQ(tm->getPosition(evening, rb).settleBasis, doctest::Approx(nominal));
    CHECK_EQ(tm->getPosition(evening, rb).margin, doctest::Approx(lock));
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 - lock));
}

/** @par Test points: the margin call is a strict shortfall, an equity exactly equal to the
 *  maintenance requirement being still acceptable */
TEST_CASE("test_TradeManager_settlement_margin_call_boundary") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    double number = 10.0;
    price_t nominal = roundEx(close * number * rb.unit(), 2);
    REQUIRE_UNARY(nominal > 0.0);

    // Settled at a price it already carries, the equity of the account is exactly the capital paid
    // in (the lock sits inside it), so the requirement equal to it is the boundary
    price_t init_cash = 1000000.0;
    double exact_rate = init_cash / nominal;

    auto at_boundary = crtSettleTM(dt, init_cash, exact_rate, true);
    REQUIRE_EQ(at_boundary->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);

    /** @arg An equity exactly equal to the requirement is not a shortfall */
    REQUIRE_UNARY(at_boundary->settlement(dt));
    CHECK_UNARY(!at_boundary->isForcedLiquidationQueued(dt));

    /** @arg A requirement above the equity is, so the check is a strict one */
    auto above = crtSettleTM(dt, init_cash, exact_rate * 1.01, true);
    REQUIRE_EQ(above->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    REQUIRE_UNARY(above->settlement(dt));
    CHECK_UNARY(above->isForcedLiquidationQueued(dt));
}

/** @par Test points: one security held on both sides settles as two records, the sign of the
 *  quantity telling them apart, and neither side swallows the other */
TEST_CASE("test_TradeManager_settlement_both_sides") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    KQuery query = KQueryByDate(dt, Datetime(200904100000), KQuery::DAY);
    KData bars = rb.getKData(query);
    REQUIRE_UNARY(bars.size() >= 3);
    Datetime dt2 = bars[1].datetime;
    price_t close2 = bars[1].closePrice;
    REQUIRE_UNARY(close2 > 0.0);

    double unit = rb.unit();
    double number = 10.0;
    price_t nominal2 = roundEx(close2 * number * unit, 2);
    price_t lock2 = roundEx(0.1 * nominal2, 2);

    auto tm = crtSettleTM(dt, 1000000.0, 0.1, false);
    REQUIRE_EQ(tm->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business, BUSINESS_BUY);
    REQUIRE_EQ(tm->sellShort(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_SELL_SHORT);
    REQUIRE_UNARY(tm->settlement(dt2));

    /** @arg One settlement record per holding, the sign carrying the side */
    int long_records = 0;
    int short_records = 0;
    for (const auto& tr : tm->getRefTradeList()) {
        if (tr.business != BUSINESS_SETTLEMENT) {
            continue;
        }
        if (tr.number > 0.0) {
            ++long_records;
        } else if (tr.number < 0.0) {
            ++short_records;
        }
    }
    CHECK_EQ(long_records, 1);
    CHECK_EQ(short_records, 1);

    /** @arg Both bases are marked to the same settlement price, both locks relocked */
    CHECK_EQ(tm->getPosition(dt2, rb).settleBasis, doctest::Approx(nominal2));
    CHECK_EQ(tm->getShortPosition(rb).settleBasis, doctest::Approx(nominal2));
    CHECK_EQ(tm->getPosition(dt2, rb).margin, doctest::Approx(lock2));
    CHECK_EQ(tm->getShortPosition(rb).margin, doctest::Approx(lock2));

    /** @arg The two sides cancel on the equity, while both locks stay occupied */
    CHECK_EQ(tm->getFunds(dt2, KQuery::DAY).occupied_margin,
             doctest::Approx(roundEx(lock2 * 2, 2)));
    CHECK_EQ(tm->getFunds(dt2, KQuery::DAY).net_assets(), doctest::Approx(1000000.0));
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 - 2 * lock2));
}

/** @} */
