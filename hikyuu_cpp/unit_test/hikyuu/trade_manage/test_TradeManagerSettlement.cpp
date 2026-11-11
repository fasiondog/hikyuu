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
#include <hikyuu/data_driver/DataDriverFactory.h>
#include <hikyuu/trade_manage/crtTM.h>
#include <hikyuu/trade_manage/margin/MarginBase.h>
#include <hikyuu/trade_manage/tradecost/TradeCostBase.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_MarginInterest.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_Zero.h>

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

/** A margin model that requires nothing at all, so a settlement relocks a holding to zero */
class MG_TestSettleZero : public MarginBase {
    MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MG_TestSettleZero() : MarginBase("MG_TestSettleZero") {}

    price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price, double num,
                          MarginSide side) const override {
        return 0.0;
    }

    double getMarginRate(const Datetime& datetime, const Stock& stock,
                         MarginSide side) const override {
        return 0.0;
    }

    MarginPtr _clone() override {
        return make_shared<MG_TestSettleZero>();
    }
};

/** A margin account whose model reports the maintenance rate given */
TradeManagerPtr crtSettleTM(const Datetime& datetime, price_t init_cash, double maintain_rate,
                            bool force_liquidate) {
    auto tm = crtTM(datetime, init_cash, TC_Zero(), "SYS", make_shared<MG_TestSettleCall>());
    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<bool>("force_liquidate", force_liquidate);
    tm->margin()->setParam<double>("maintain_rate", maintain_rate);
    return tm;
}

/** A cost model answering a fixed annualized rate for each kind of liability */
class TestInterestCost final : public TradeCostBase {
public:
    TestInterestCost(double cash_rate, double stock_rate)
    : TradeCostBase("TestInterestCost"), m_cash_rate(cash_rate), m_stock_rate(stock_rate) {}

    CostRecord getBuyCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord();
    }

    CostRecord getSellCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord();
    }

    double getBorrowCashRate(const Datetime&) const override {
        return m_cash_rate;
    }

    double getBorrowStockRate(const Datetime&, const Stock&) const override {
        return m_stock_rate;
    }

protected:
    TradeCostPtr _clone() override {
        return std::make_shared<TestInterestCost>(m_cash_rate, m_stock_rate);
    }

private:
    double m_cash_rate{0.0};
    double m_stock_rate{0.0};
};

/** A settling margin account with the given annualized liability rates */
TradeManagerPtr crtInterestTM(const Datetime& datetime, price_t init_cash, double cash_rate,
                              double stock_rate, int annual_days) {
    auto cost = std::make_shared<TestInterestCost>(cash_rate, stock_rate);
    auto tm = crtTM(datetime, init_cash, cost, "SYS", make_shared<MG_TestSettleCall>());
    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<bool>("force_liquidate", false);
    tm->setParam<int>("annual_days", annual_days);
    return tm;
}

/** How many interest records the account carries, at the given moment or over all of them */
size_t countInterest(const TradeManagerPtr& tm, const Datetime& datetime = Null<Datetime>()) {
    size_t count = 0;
    for (const auto& tr : tm->getRefTradeList()) {
        if (tr.business == BUSINESS_INTEREST && (datetime.isNull() || tr.datetime == datetime)) {
            ++count;
        }
    }
    return count;
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
 *  telling the fallback, and a settlement posted later in the day does not count for an earlier
 *  moment */
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
    // The day is settled, but at a later moment than the probed one, which has not been settled
    // yet - and settling it now is refused as going back in time
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

/** @par Test points: the financing interest of a settled day is billed into the debt itself instead
 *  of being paid from the cash, one record per liability item, and the whole debt stays repayable
 */
TEST_CASE("test_TradeManager_settlement_interest_cash") {
    // A book made only of a cash debt needs no quote, so it can be dated freely
    Datetime dt(200912310000);
    price_t init_cash = 1000000.0;
    price_t borrow = 10000.0;
    // 0.365 over an annual base of 365 is exactly one thousandth of the debt per billed day
    auto tm = crtInterestTM(dt, init_cash, 0.365, 0.0, 365);
    REQUIRE_UNARY(tm->borrowCash(dt, borrow));
    price_t cash = tm->currentCash();
    REQUIRE_EQ(cash, doctest::Approx(init_cash + borrow));

    /** @arg A flat book with an outstanding debt does settle, because the bill was posted */
    CHECK_UNARY(!tm->isSettled(dt));
    REQUIRE_UNARY(tm->settlement(dt));
    CHECK_UNARY(tm->isSettled(dt));

    /** @arg The billed amount joins the principal and leaves the cash alone */
    price_t interest = roundEx(borrow * 0.365 / 365.0, 2);
    REQUIRE_EQ(interest, doctest::Approx(10.0));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow + interest));
    CHECK_EQ(tm->currentCash(), doctest::Approx(cash));

    /** @arg One record for the cash liability, with the amount and no quantity */
    size_t count = 0;
    for (const auto& tr : tm->getRefTradeList()) {
        if (tr.business != BUSINESS_INTEREST) {
            continue;
        }
        ++count;
        CHECK_EQ(tr.datetime, dt);
        CHECK_UNARY(tr.stock.isNull());
        CHECK_EQ(tr.realPrice, doctest::Approx(interest));
        CHECK_EQ(tr.number, doctest::Approx(0.0));
        CHECK_EQ(tr.margin, doctest::Approx(0.0));
    }
    CHECK_EQ(count, size_t(1));

    /** @arg The debt is a liability of the account, so the net assets shrink by the bill */
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(init_cash - interest));

    /** @arg A second call of the same moment bills nothing again */
    REQUIRE_UNARY(tm->settlement(dt));
    CHECK_EQ(countInterest(tm, dt), size_t(1));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow + interest));

    /** @arg The billed interest is repayable: returnCash still covers the whole debt */
    REQUIRE_UNARY(tm->returnCash(dt, borrow + interest));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(0.0));
    CHECK_EQ(tm->currentCash(), doctest::Approx(cash - borrow - interest));
}

/** @par Test points: a system driven by a finer K type asks for the settlement on every bar, and
 *  the day is settled once for all of them */
TEST_CASE("test_TradeManager_settlement_interest_same_day_twice") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    price_t borrow = 10000.0;
    double number = 10.0;
    auto tm = crtInterestTM(dt, 1000000.0, 0.365, 0.365, 365);
    REQUIRE_EQ(tm->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business, BUSINESS_BUY);
    REQUIRE_UNARY(tm->borrowCash(dt, borrow));

    /** @arg The first bar of the day settles both halves in one batch */
    Datetime morning(dt.year(), dt.month(), dt.day(), 9, 31);
    REQUIRE_UNARY(tm->settlement(morning));
    CHECK_EQ(countInterest(tm, morning), size_t(1));
    price_t debt = tm->getDebtCash(morning);
    REQUIRE_UNARY(debt > borrow);

    /** @arg A later bar of the same day is told it is settled and posts neither half again */
    Datetime evening(dt.year(), dt.month(), dt.day(), 15, 0);
    REQUIRE_UNARY(tm->settlement(evening));
    CHECK_UNARY(tm->isSettled(evening));
    CHECK_EQ(countInterest(tm, evening), size_t(0));
    CHECK_EQ(countInterest(tm), size_t(1));
    CHECK_EQ(tm->getDebtCash(evening), doctest::Approx(debt));

    /** @arg A holding opened between the two calls waits for the next day to be marked, as the
     *  whole batch is a daily one */
    REQUIRE_EQ(tm->buy(morning, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    auto count_settled = [tm]() {
        size_t count = 0;
        for (const auto& tr : tm->getRefTradeList()) {
            count += (tr.business == BUSINESS_SETTLEMENT ? 1 : 0);
        }
        return count;
    };
    size_t settled_before = count_settled();
    REQUIRE_UNARY(tm->settlement(evening));
    CHECK_EQ(count_settled(), settled_before);
}

/** @par Test points: the securities lending is billed in cash, on the market value of the borrowed
 *  shares at the settlement price, while the borrowed quantity itself stays untouched */
TEST_CASE("test_TradeManager_settlement_interest_stock") {
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

    double number = 10.0;
    price_t init_cash = 1000000.0;
    auto tm = crtInterestTM(dt, init_cash, 0.0, 0.365, 365);
    REQUIRE_UNARY(tm->borrowStock(dt, rb, close, number));
    price_t cash = tm->currentCash();

    price_t interest = roundEx(roundEx(close2 * number * rb.unit(), 2) * 0.365 / 365.0, 2);
    REQUIRE_UNARY(interest > 0.0);
    REQUIRE_UNARY(tm->settlement(dt2));

    /** @arg The bill lands on the cash debt, not on the borrowed shares */
    CHECK_EQ(tm->getDebtCash(dt2), doctest::Approx(interest));
    CHECK_EQ(tm->getDebtNumber(dt2, rb), doctest::Approx(number));
    CHECK_EQ(tm->currentCash(), doctest::Approx(cash));

    /** @arg One record for the lending, carrying the security and the borrowed quantity */
    size_t count = 0;
    for (const auto& tr : tm->getRefTradeList()) {
        if (tr.business != BUSINESS_INTEREST) {
            continue;
        }
        ++count;
        CHECK_EQ(tr.stock, rb);
        CHECK_EQ(tr.realPrice, doctest::Approx(interest));
        CHECK_EQ(tr.number, doctest::Approx(number));
    }
    CHECK_EQ(count, size_t(1));

    /** @arg The borrowed shares are not an asset, so only the cash debt reduces the net assets */
    CHECK_EQ(tm->getFunds(dt2, KQuery::DAY).net_assets(), doctest::Approx(init_cash - interest));
}

/** @par Test points: past the registered last trading day of the security the lending stops being
 *  billed, because the expiry liquidation owns it and not the daily settlement */
TEST_CASE("test_TradeManager_settlement_interest_stock_expiry") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    KQuery query = KQueryByDate(Datetime(200903270000), Datetime(200904100000), KQuery::DAY);
    KData bars = rb.getKData(query);
    REQUIRE_UNARY(bars.size() >= 3);
    const Datetime& dt = bars[0].datetime;
    const Datetime& dt2 = bars[1].datetime;
    const Datetime& dt3 = bars[2].datetime;

    // The very same data registered with an expiry between the second and the third day, so the
    // stale close goes on answering a price and only the expiry can stop the billing
    Stock lending("SHF", "RB0000", "lending test contract", 12, true, dt, dt2, 1.0, 1.0, 2, 1,
                  100000);
    lending.setKDataDriver(rb.getKDataDirver());
    REQUIRE_UNARY(!lending.isNull());
    REQUIRE_EQ(lending.lastDatetime(), dt2);
    REQUIRE_EQ(lending.getMarketValue(dt, KQuery::DAY), doctest::Approx(bars[0].closePrice));

    price_t init_cash = 1000000.0;
    double number = 10.0;
    auto tm = crtInterestTM(dt, init_cash, 0.0, 0.365, 365);
    REQUIRE_UNARY(tm->borrowStock(dt, lending, bars[0].closePrice, number));

    /** @arg While the security is listed, the lending is billed on its settlement value */
    price_t debt_base = roundEx(bars[1].closePrice * number * lending.unit(), 2);
    price_t interest = roundEx(debt_base * 0.365 / 365.0, 2);
    REQUIRE_UNARY(interest > 0.0);
    REQUIRE_UNARY(tm->settlement(dt2));
    CHECK_EQ(countInterest(tm, dt2), size_t(1));
    CHECK_EQ(tm->getDebtCash(dt2), doctest::Approx(interest));

    /** @arg Past its last trading day the quote is still there, but nothing is billed any more, so
     *  the day counts as never settled and the debt stops growing */
    REQUIRE_UNARY(lending.getMarketValue(dt3, KQuery::DAY) > 0.0);
    CHECK_UNARY(!tm->settlement(dt3));
    CHECK_UNARY(!tm->isSettled(dt3));
    CHECK_EQ(countInterest(tm), size_t(1));
    CHECK_EQ(tm->getDebtCash(dt3), doctest::Approx(interest));
    CHECK_EQ(tm->getFunds(dt3, KQuery::DAY).net_assets(), doctest::Approx(init_cash - interest));
}

/** @par Test points: a lending of a security that has never been priced is reported and bills
 *  nothing, instead of silently settling at nothing */
TEST_CASE("test_TradeManager_settlement_interest_stock_no_price") {
    Stock lending("TEST", "NOPRICE", "unpriced lending contract", 1, true, Datetime(199001010000),
                  Datetime(209901010000), 1.0, 1.0, 2, 1, 100000);
    Parameter driver_param;
    driver_param.set<string>("type", "DoNothing");
    lending.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    REQUIRE_UNARY(!lending.isNull());

    Datetime dt(200903270000);
    auto tm = crtInterestTM(dt, 1000000.0, 0.0, 0.365, 365);
    REQUIRE_UNARY(tm->borrowStock(dt, lending, 10.0, 100.0));

    /** @arg Inside its registered life but with no quote at all: the lending bills nothing */
    CHECK_UNARY(!tm->settlement(dt));
    CHECK_UNARY(!tm->isSettled(dt));
    CHECK_EQ(countInterest(tm), size_t(0));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(0.0));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(1000000.0));
}

/** @par Test points: the proration base stays the same across a calendar boundary, each billed day
 *  compounds on the debt the previous one grew, and the days waited over are all billed */
TEST_CASE("test_TradeManager_settlement_interest_year_boundary") {
    Datetime dt(200912310000);
    Datetime dt2(201001010000);
    Datetime dt3(201001050000);
    price_t borrow = 10000.0;
    auto tm = crtInterestTM(dt, 1000000.0, 0.365, 0.0, 365);
    REQUIRE_UNARY(tm->borrowCash(dt, borrow));
    REQUIRE_UNARY(tm->settlement(dt));

    /** @arg The first bill covers the single day the debt was taken on */
    price_t first = roundEx(borrow * 0.365 / 365.0, 2);
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow + first));

    /** @arg The next calendar day bills on the grown debt, still over 365 */
    REQUIRE_UNARY(tm->settlement(dt2));
    price_t second = roundEx((borrow + first) * 0.365 / 365.0, 2);
    CHECK_EQ(tm->getDebtCash(dt2), doctest::Approx(borrow + first + second));
    CHECK_EQ(countInterest(tm), size_t(2));

    /** @arg After four untouched days the same rate bills all four of them at once */
    REQUIRE_UNARY(tm->settlement(dt3));
    price_t third = roundEx((borrow + first + second) * 0.365 * 4 / 365.0, 2);
    CHECK_EQ(countInterest(tm, dt3), size_t(1));
    CHECK_EQ(tm->getDebtCash(dt3), doctest::Approx(borrow + first + second + third));

    /** @arg The earlier days are still measured without the later bills */
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow + first));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).borrow_cash, doctest::Approx(borrow + first));
    CHECK_EQ(tm->getDebtCash(dt2), doctest::Approx(borrow + first + second));
}

/** @par Test points: a cost model that knows nothing about the rates keeps the accounting of the
 *  accounts written before they existed, billing nothing at all */
TEST_CASE("test_TradeManager_settlement_interest_default_zero") {
    Datetime dt(200912310000);
    price_t borrow = 10000.0;
    auto tm = crtTM(dt, 1000000.0, TC_Zero(), "SYS", make_shared<MG_TestSettleCall>());
    tm->setParam<bool>("margin_accounting", true);
    REQUIRE_UNARY(tm->borrowCash(dt, borrow));

    /** @arg The base cost model answers a zero rate for both kinds of liability */
    CHECK_EQ(tm->costFunc()->getBorrowCashRate(dt), 0.0);
    CHECK_EQ(tm->costFunc()->getBorrowStockRate(dt, Null<Stock>()), 0.0);

    /** @arg Nothing is billed, so a book with no holding is not settled by the interest */
    CHECK_UNARY(!tm->settlement(dt));
    CHECK_UNARY(!tm->isSettled(dt));
    CHECK_EQ(countInterest(tm), size_t(0));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(1000000.0));
}

/** @par Test points: the proration base is validated before anything is posted, and it is the
 *  denominator of the bill */
TEST_CASE("test_TradeManager_settlement_annual_days_guard") {
    Datetime dt(200912310000);
    price_t borrow = 10000.0;
    auto tm = crtInterestTM(dt, 1000000.0, 0.365, 0.0, 0);
    REQUIRE_UNARY(tm->borrowCash(dt, borrow));

    /** @arg A base that is not positive fails the whole settlement and posts nothing */
    CHECK_THROWS(tm->settlement(dt));
    CHECK_UNARY(!tm->isSettled(dt));
    CHECK_EQ(countInterest(tm), size_t(0));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow));

    /** @arg A base of 100 prorates the same annual rate over 100 days */
    tm->setParam<int>("annual_days", 100);
    REQUIRE_UNARY(tm->settlement(dt));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow + roundEx(borrow * 0.365 / 100.0, 2)));
}

/** @par Test points: a backfilled interest record rebuilds the same debt, counts as a settled day
 *  and is not applied twice */
TEST_CASE("test_TradeManager_settlement_interest_replay") {
    Datetime dt(200912310000);
    price_t borrow = 10000.0;

    auto src = crtInterestTM(dt, 1000000.0, 0.365, 0.0, 365);
    REQUIRE_UNARY(src->borrowCash(dt, borrow));
    REQUIRE_UNARY(src->settlement(dt));

    // The record as the persistence layer hands it back
    TradeRecord interest;
    for (const auto& tr : src->getRefTradeList()) {
        if (tr.business == BUSINESS_INTEREST) {
            interest = tr;
        }
    }
    REQUIRE_EQ(interest.business, BUSINESS_INTEREST);
    REQUIRE_UNARY(interest.realPrice > 0.0);

    auto tm = crtInterestTM(dt, 1000000.0, 0.365, 0.0, 365);
    REQUIRE_UNARY(tm->borrowCash(dt, borrow));

    /** @arg The rebuilt account carries the same debt and the same cash as the settled one */
    REQUIRE_UNARY(tm->addTradeRecord(interest));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow + interest.realPrice));
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 + borrow));

    /** @arg The replayed bill is a settled day, so the settlement does not bill it again */
    CHECK_UNARY(tm->isSettled(dt));
    REQUIRE_UNARY(tm->settlement(dt));
    CHECK_EQ(countInterest(tm, dt), size_t(1));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow + interest.realPrice));

    /** @arg Adding the very same record once more is a no-op */
    REQUIRE_UNARY(tm->addTradeRecord(interest));
    CHECK_EQ(countInterest(tm, dt), size_t(1));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow + interest.realPrice));
}

/** @par Test points: a book carrying both a cash debt and a lending bills the two items in the same
 *  batch, one record each, and neither of them twice */
TEST_CASE("test_TradeManager_settlement_interest_mixed") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);
    KData bars = rb.getKData(KQueryByDate(dt, Datetime(200904100000), KQuery::DAY));
    REQUIRE_UNARY(bars.size() >= 2);
    Datetime dt2 = bars[1].datetime;
    price_t close2 = bars[1].closePrice;
    REQUIRE_UNARY(close2 > 0.0);

    price_t borrow = 10000.0;
    double number = 10.0;
    auto tm = crtInterestTM(dt, 1000000.0, 0.365, 0.365, 365);
    REQUIRE_UNARY(tm->borrowCash(dt, borrow));
    REQUIRE_UNARY(tm->borrowStock(dt, rb, close, number));
    price_t cash = tm->currentCash();

    REQUIRE_UNARY(tm->settlement(dt2));

    /** @arg Each item bills on its own base, and both amounts land on the single cash debt */
    price_t cash_interest = roundEx(borrow * 0.365 / 365.0, 2);
    price_t stock_interest = roundEx(roundEx(close2 * number * rb.unit(), 2) * 0.365 / 365.0, 2);
    REQUIRE_UNARY(cash_interest > 0.0 && stock_interest > 0.0);
    CHECK_EQ(tm->getDebtCash(dt2), doctest::Approx(borrow + cash_interest + stock_interest));
    CHECK_EQ(tm->currentCash(), doctest::Approx(cash));

    /** @arg One record per liability item, the cash one first and carrying no security */
    CHECK_EQ(countInterest(tm, dt2), size_t(2));
    size_t lending = 0;
    for (const auto& tr : tm->getRefTradeList()) {
        if (tr.business != BUSINESS_INTEREST) {
            continue;
        }
        if (tr.stock.isNull()) {
            CHECK_EQ(tr.realPrice, doctest::Approx(cash_interest));
            CHECK_EQ(tr.number, doctest::Approx(0.0));
        } else {
            ++lending;
            CHECK_EQ(tr.stock, rb);
            CHECK_EQ(tr.realPrice, doctest::Approx(stock_interest));
            CHECK_EQ(tr.number, doctest::Approx(number));
        }
    }
    CHECK_EQ(lending, size_t(1));

    /** @arg A second call of the same moment bills neither item again */
    REQUIRE_UNARY(tm->settlement(dt2));
    CHECK_EQ(countInterest(tm, dt2), size_t(2));
    CHECK_EQ(tm->getDebtCash(dt2), doctest::Approx(borrow + cash_interest + stock_interest));

    /** @arg The whole grown debt stays repayable */
    REQUIRE_UNARY(tm->returnCash(dt2, borrow + cash_interest + stock_interest));
    CHECK_EQ(tm->getDebtCash(dt2), doctest::Approx(0.0));
}

/** @par Test points: the replay of getFundsList carries the bill from its own day on, exactly like
 *  the single date query does */
TEST_CASE("test_TradeManager_settlement_interest_funds_list") {
    Datetime dt(200912310000);
    Datetime dt2(201001040000);
    price_t borrow = 10000.0;
    price_t init_cash = 1000000.0;
    auto tm = crtInterestTM(dt, init_cash, 0.365, 0.0, 365);
    REQUIRE_UNARY(tm->borrowCash(dt, borrow));
    REQUIRE_UNARY(tm->settlement(dt));
    price_t first = roundEx(borrow * 0.365 / 365.0, 2);

    // A later trade makes the billed day a past one, i.e. the dates are answered by the replay;
    // the four calendar days in between are all billed
    REQUIRE_UNARY(tm->borrowCash(dt2, borrow));
    REQUIRE_UNARY(tm->settlement(dt2));
    price_t second = roundEx((2 * borrow + first) * 0.365 * 4 / 365.0, 2);

    /** @arg The earlier day knows nothing about the later bill, and the later one holds both */
    FundsList list = tm->getFundsList(DatetimeList{dt, dt2});
    REQUIRE_EQ(list.size(), size_t(2));
    CHECK_EQ(list[0].borrow_cash, doctest::Approx(borrow + first));
    CHECK_EQ(list[1].borrow_cash, doctest::Approx(2 * borrow + first + second));

    /** @arg The per date query agrees with the batch one on both moments */
    CHECK_EQ(tm->getFunds(dt).borrow_cash, doctest::Approx(list[0].borrow_cash));
    CHECK_EQ(tm->getFunds(dt2).borrow_cash, doctest::Approx(list[1].borrow_cash));
}

/** @par Test points: the built-in wrapping part feeds the very same settlement, so no test stub is
 *  needed to bill the interest of a real account */
TEST_CASE("test_TradeManager_settlement_interest_part") {
    Datetime dt(200912310000);
    price_t borrow = 10000.0;
    auto cost = TC_MarginInterest(TC_Zero(), 0.365);
    auto tm = crtTM(dt, 1000000.0, cost, "SYS", make_shared<MG_TestSettleCall>());
    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<int>("annual_days", 365);

    /** @arg The account reads the rates through its cost model */
    CHECK_EQ(tm->costFunc()->getBorrowCashRate(dt), doctest::Approx(0.365));
    CHECK_EQ(tm->costFunc()->getBorrowStockRate(dt, Null<Stock>()), 0.0);

    /** @arg The bill lands the same way as the one of a hand written model */
    REQUIRE_UNARY(tm->borrowCash(dt, borrow));
    REQUIRE_UNARY(tm->settlement(dt));
    CHECK_EQ(countInterest(tm, dt), size_t(1));
    CHECK_EQ(tm->getDebtCash(dt), doctest::Approx(borrow + roundEx(borrow * 0.365 / 365.0, 2)));
}

/** @par Test points: a settlement relocking a holding to zero demotes it to the cash books on every
 *  replay path, so the per date query and the batch one do not diverge on the zero lock snapshot */
TEST_CASE("test_TradeManager_settlement_relock_zero_replay") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    Datetime dt2(200001050000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    auto tm = crtSettleTM(dt, 1000000.0, 0.1, false);
    REQUIRE_EQ(tm->buy(dt, stock, close, 100.0, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    REQUIRE_UNARY(tm->getFunds(dt, KQuery::DAY).occupied_margin > 0.0);

    /** @arg A model requiring nothing relocks the holding to zero and books it in the record */
    tm->margin(make_shared<MG_TestSettleZero>());
    REQUIRE_UNARY(tm->settlement(dt2));
    REQUIRE_EQ(tm->getRefTradeList().back().business, BUSINESS_SETTLEMENT);
    CHECK_EQ(tm->getRefTradeList().back().margin, 0.0);

    /** @arg The per date query reads the zero lock as a plain cash holding */
    FundsRecord live = tm->getFunds(dt2, KQuery::DAY);
    CHECK_EQ(live.occupied_margin, 0.0);

    /** @arg The batch replay answers the same cash, market value and net assets */
    FundsRecordList list = tm->getFundsList({dt2});
    REQUIRE_EQ(list.size(), size_t(1));
    CHECK_EQ(list[0].occupied_margin, 0.0);
    CHECK_EQ(list[0].cash, doctest::Approx(live.cash));
    CHECK_EQ(list[0].market_value, doctest::Approx(live.market_value));
    CHECK_EQ(list[0].net_assets(), doctest::Approx(live.net_assets()));
}

/** @} */
