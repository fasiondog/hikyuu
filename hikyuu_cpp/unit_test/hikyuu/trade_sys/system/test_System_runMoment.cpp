/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 *
 *  Regression tests for the System::runMoment result contract.
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/crt/SG_AllwaysBuy.h>
#include <hikyuu/trade_sys/signal/crt/SG_Manual.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_manage/crtTM.h>

using namespace hku;

/**
 * @defgroup test_System_runMoment test_System_runMoment
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Check point: with buy_delay, the runMoment pending request appears in delayOnNextOpen
 * (previously this field had no filling point) */
TEST_CASE("test_System_runMoment_delay_request") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 0);

    auto sys = SYS_Simple(crtTM(Datetime(199001010000LL), 100000.0), MM_Nothing(), EnvironmentPtr(),
                          ConditionPtr(), SG_AllwaysBuy());
    sys->setParam<bool>("buy_delay", true);
    sys->setTO(kdata);
    sys->readyForRun();

    /** @arg delayed buy: no immediate trade, the pending request is visible */
    MomentResult r = sys->runMoment(kdata[0].datetime);
    CHECK_UNARY(r.tradesOnOpen.empty());
    CHECK_UNARY(r.tradesOnClose.empty());
    REQUIRE_EQ(r.delayOnNextOpen.size(), 1);
    CHECK_UNARY(r.delayOnNextOpen[0].valid);
    CHECK_EQ(r.delayOnNextOpen[0].business, BUSINESS_BUY);
}

namespace {
KData delayedTradeKData() {
    Stock stock("TEST", "DELAYED", "Delayed trades");
    stock.setKRecordList({KRecord(Datetime(202001020000), 10, 11, 9, 10, 100, 100),
                          KRecord(Datetime(202001030000), 10, 10, 10, 10, 0, 0),
                          KRecord(Datetime(202001060000), 10, 11, 9, 10, 100, 100),
                          KRecord(Datetime(202001070000), 10, 11, 9, 10, 100, 100)});
    return stock.getKData(KQuery(0));
}

// Partial exits keep both queued orders executable.
class DelayedTradeMM : public MoneyManagerBase {
public:
    DelayedTradeMM() : MoneyManagerBase("DelayedTradeMM") {
        m_support_mult_buy_sell = true;
    }

    MoneyManagerPtr _clone() override {
        return std::make_shared<DelayedTradeMM>();
    }

    double _getBuyNumber(const Datetime& datetime, const Stock&, price_t, price_t,
                         SystemPart) override {
        return datetime == Datetime(202001020000) ? 100 : 200;
    }
    double _getSellNumber(const Datetime& datetime, const Stock& stock, price_t price, price_t risk,
                          SystemPart from) override {
        return _getBuyNumber(datetime, stock, price, risk, from);
    }
    double _getSellShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                               price_t risk, SystemPart from) override {
        return _getBuyNumber(datetime, stock, price, risk, from);
    }
    double _getBuyShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                              price_t risk, SystemPart from) override {
        return _getBuyNumber(datetime, stock, price, risk, from);
    }
};

// Drive the delayed queue either in one combined moment or as separate open/close calls; both
// shapes must expose the same fills.
void checkAllDelayedTrades(bool split) {
    for (auto business : {BUSINESS_BUY, BUSINESS_SELL, BUSINESS_SELL_SHORT, BUSINESS_BUY_SHORT}) {
        CAPTURE(business);
        KData kdata = delayedTradeKData();
        Stock stock = kdata.getStock();
        auto tm = crtTM(Datetime(201901010000), 100000.0, TC_Zero());
        auto sg = SG_Manual();
        sg->setParam<bool>("alternate", false);
        const bool buy_signal = business == BUSINESS_BUY || business == BUSINESS_BUY_SHORT;
        for (size_t i = 0; i < 2; ++i) {
            sg->_addSignal(kdata[i].datetime, buy_signal ? 1.0 : -1.0);
        }
        auto sys =
          SYS_Simple(tm, std::make_shared<DelayedTradeMM>(), EnvironmentPtr(), ConditionPtr(), sg);
        sys->setParam<bool>("support_borrow_stock", true);
        sys->setParam<bool>("delay_use_current_price", false);
        sys->setTO(kdata);
        sys->readyForRun();
        if (business == BUSINESS_SELL) {
            REQUIRE_EQ(tm->buy(Datetime(201912310000), stock, 10, 400).business, BUSINESS_BUY);
        } else if (business == BUSINESS_BUY_SHORT) {
            REQUIRE_EQ(tm->sellShort(Datetime(201912310000), stock, 10, 400).business,
                       BUSINESS_SELL_SHORT);
        }

        auto drive = [&](size_t i) {
            if (!split) {
                return sys->runMoment(kdata[i].datetime);
            }
            MomentResult result = sys->runMomentOnOpen(kdata[i].datetime);
            MomentResult close = sys->runMomentOnClose(kdata[i].datetime);
            result.tradesOnClose = close.tradesOnClose;
            result.delayOnNextOpen = close.delayOnNextOpen;
            return result;
        };

        /** @arg a suspended bar preserves the first request and queues the second */
        CHECK_UNARY(drive(0).tradesOnOpen.empty());
        MomentResult suspended = drive(1);
        CHECK_UNARY(suspended.tradesOnOpen.empty());
        REQUIRE_EQ(suspended.delayOnNextOpen.size(), 2);

        /** @arg both fills are reported in ledger order, without leaking into the close */
        const size_t before = sys->getTradeRecordList().size();
        MomentResult filled = drive(2);
        const auto& ledger = sys->getTradeRecordList();
        REQUIRE_EQ(ledger.size() - before, 2);
        REQUIRE_EQ(filled.tradesOnOpen.size(), 2);
        for (size_t i = 0; i < 2; ++i) {
            CHECK_EQ(filled.tradesOnOpen[i], ledger[before + i]);
            CHECK_EQ(filled.tradesOnOpen[i].business, business);
            CHECK_EQ(filled.tradesOnOpen[i].number, 100 * (i + 1));
        }
        CHECK_UNARY(filled.tradesOnClose.empty());
        CHECK_UNARY(filled.delayOnNextOpen.empty());

        /** @arg a repeated open with no pending request must not report old fills */
        CHECK_UNARY(sys->runMomentOnOpen(kdata[2].datetime).tradesOnOpen.empty());
        CHECK_UNARY(drive(3).tradesOnOpen.empty());
    }
}
}  // namespace

TEST_CASE("test_System_runMoment_all_delayed_trades") {
    SUBCASE("combined") {
        checkAllDelayedTrades(false);
    }
    SUBCASE("open_then_close") {
        checkAllDelayedTrades(true);
    }
}

TEST_CASE("test_System_runMoment_delayed_trade_accounting") {
    KData kdata = delayedTradeKData();
    // System::reset() keeps the trade manager, so the batch path is verified on a fresh instance
    // instead of re-running the moment-driven one on an account that is not reset
    auto make_sys = [&kdata]() {
        auto sg = SG_Manual();
        sg->setParam<bool>("alternate", false);
        sg->_addBuySignal(kdata[0].datetime);
        sg->_addBuySignal(kdata[1].datetime);
        sg->_addSellSignal(kdata[2].datetime);
        auto sys = SYS_Simple(crtTM(Datetime(201901010000), 100000.0, TC_Zero()),
                              MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
        sys->setParam<bool>("sell_delay", false);
        sys->setTO(kdata);
        sys->readyForRun();
        return sys;
    };

    auto sys = make_sys();
    sys->runMoment(kdata[0].datetime);
    sys->runMoment(kdata[1].datetime);

    /** @arg two open buys and one immediate close sell belong to separate phases */
    MomentResult result = sys->runMoment(kdata[2].datetime);
    REQUIRE_EQ(result.tradesOnOpen.size(), 2);
    REQUIRE_EQ(result.tradesOnClose.size(), 1);
    CHECK_EQ(result.tradesOnClose[0].business, BUSINESS_SELL);
    CHECK_EQ(result.tradesOnClose[0].number, 200);
    CHECK_EQ(result.funds_before_open.cash, 100000);
    CHECK_EQ(result.funds_before_close.cash, 98000);
    CHECK_EQ(result.funds.cash, 100000);
    CHECK_UNARY(result.positions.empty());

    /** @arg an independent batch run reproduces the same ledger */
    const TradeRecordList moment_trades = sys->getTradeRecordList();
    auto batch = make_sys();
    batch->run(kdata);
    REQUIRE_EQ(batch->getTradeRecordList().size(), moment_trades.size());
    for (size_t i = 0; i < moment_trades.size(); ++i) {
        CHECK_EQ(batch->getTradeRecordList()[i], moment_trades[i]);
    }
}

/** @} */
