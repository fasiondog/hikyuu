/*
 * test_TradeManagerCurrency.cpp
 *
 *  Created on: 2026-11-12
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/config.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/Performance.h>
#include <hikyuu/trade_manage/TradeManager.h>
#include <hikyuu/trade_manage/crtTM.h>
#include <hikyuu/trade_manage/margin/crt/MG_ByType.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_HK.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_MarginInterest.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_Zero.h>
#include <hikyuu/trade_sys/allocatefunds/AllocateFundsBase.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_sys/system/TradeSuggestion.h>
#include <hikyuu/utilities/exception.h>

#if HKU_SUPPORT_SERIALIZATION
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <fstream>
#include <sstream>
#endif

using namespace hku;

/**
 * @defgroup test_TradeManagerCurrency test_TradeManagerCurrency
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

namespace {

/** Expose the warned-market table, so the one-off warning can be asserted on its state */
class WarnProbeTM final : public TradeManager {
public:
    WarnProbeTM() : TradeManager(Datetime(199001010000), 100000.0, TC_Zero(), "WARNPROBE") {}
    size_t warnedCount() const {
        return m_fx_warned.size();
    }
};

/** A cost model answering a flat fee of one unit in the security's currency on the securities
 * lending hooks, where the built-in models stay silent */
class FeeStubCost final : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FeeStubCost() : TradeCostBase("FeeStub") {}

    CostRecord getBuyCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord();
    }

    CostRecord getSellCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord();
    }

    CostRecord getBorrowStockCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord(0.0, 0.0, 0.0, 1.0, 1.0);
    }

    CostRecord getReturnStockCost(const Datetime&, const Datetime&, const Stock&, price_t,
                                  double) const override {
        return CostRecord(0.0, 0.0, 0.0, 1.0, 1.0);
    }

    TradeCostPtr _clone() override {
        return make_shared<FeeStubCost>();
    }
};

}  // namespace

/** @par Test points: an empty currency_rates keeps the books in the security's own currency, a
 * configured rate converts the notional, the cost, the realized profit and the market value into
 * the account currency, and a market missing from a configured table is billed at 1.0 */
TEST_CASE("test_TradeManager_currency_rates") {
    StockManager& sm = StockManager::instance();
    Stock us = sm.getStock("NASDAQNDX");
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!us.isNull());
    REQUIRE_UNARY(!sh.isNull());
    REQUIRE_EQ(us.market(), "NASDAQ");

    KData k = us.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);
    const price_t px = 100.0;
    const double number = 10.0;
    const price_t money = roundEx(px * number * us.unit(), 2);
    REQUIRE_UNARY(money > 0.0);

    /** @arg With no rate configured the foreign market is booked as it is (1.0) */
    TradeManagerPtr plain = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "PLAIN");
    CHECK_EQ(plain->buy(dt, us, px, number).business, BUSINESS_BUY);
    CHECK_EQ(plain->currentCash(), roundEx(100000.0 - money, 2));

    /** @arg A configured rate converts the notional of the trade */
    TradeManagerPtr rated = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "RATED");
    rated->setParam<string>("currency_rates", "nasdaq:7.0");
    CHECK_EQ(rated->buy(dt, us, px, number).business, BUSINESS_BUY);
    CHECK_EQ(rated->currentCash(), roundEx(100000.0 - money * 7.0, 2));

    /** @arg The market value of the holding is converted as well */
    price_t close = us.getMarketValue(dt, "DAY");
    REQUIRE_UNARY(close > 0.0);
    CHECK_EQ(rated->getFunds().market_value, roundEx(number * close * us.unit() * 7.0, 2));
    CHECK_EQ(plain->getFunds().market_value, roundEx(number * close * us.unit(), 2));

    /** @arg The realized profit and the cost of the sell are converted too */
    TradeManagerPtr fee_tm =
      crtTM(Datetime(199001010000), 100000.0, TC_HK(0.0, 0.0, 0.0, 0.0, 1.0), "FEE");
    fee_tm->setParam<string>("currency_rates", "NASDAQ:7.0");
    CHECK_EQ(fee_tm->buy(dt, us, px, number).business, BUSINESS_BUY);
    CHECK_EQ(fee_tm->currentCash(), roundEx(100000.0 - money * 7.0 - 7.0, 2));
    CHECK_EQ(fee_tm->sell(dt, us, 120.0, number).business, BUSINESS_SELL);
    CHECK_EQ(fee_tm->currentCash(),
             roundEx(100000.0 - money * 7.0 - 7.0 + 120.0 * number * 7.0 - 7.0, 2));

    /** @arg An unlisted market of a configured table is billed at 1.0, like an empty table */
    TradeManagerPtr partial = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "PARTIAL");
    partial->setParam<string>("currency_rates", "HK:0.92");
    CHECK_EQ(partial->buy(dt, us, px, number).business, BUSINESS_BUY);
    CHECK_EQ(partial->currentCash(), plain->currentCash());

    /** @arg An account currency security is untouched by the table */
    const Datetime sh_dt(200001040000);
    TradeManagerPtr a_plain = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "A_PLAIN");
    TradeManagerPtr a_rated = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "A_RATED");
    a_rated->setParam<string>("currency_rates", "NASDAQ:7.0");
    CHECK_EQ(a_plain->buy(sh_dt, sh, 10.0, 1000).business, BUSINESS_BUY);
    CHECK_EQ(a_rated->buy(sh_dt, sh, 10.0, 1000).business, BUSINESS_BUY);
    CHECK_EQ(a_plain->currentCash(), a_rated->currentCash());
    CHECK_EQ(a_plain->getFunds().market_value, a_rated->getFunds().market_value);

    /** @arg A replayed account reproduces the converted books */
    TradeManagerPtr replayed = crtTM(Datetime(199001010000), 0.0, TC_Zero(), "REPLAY");
    replayed->setParam<string>("currency_rates", "NASDAQ:7.0");
    for (const auto& tr : rated->getTradeList()) {
        CHECK_UNARY(replayed->addTradeRecord(tr));
    }
    CHECK_EQ(replayed->currentCash(), rated->currentCash());
    CHECK_EQ(replayed->getFunds().market_value, rated->getFunds().market_value);

    /** @arg The historical query values the holding at the same rate */
    CHECK_EQ(rated->getFunds(dt, "DAY").market_value, rated->getFunds("DAY").market_value);
}

/** @par Test points: the currency_rates expression is validated when it is set */
TEST_CASE("test_TradeManager_currency_rates_param") {
    TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "CHECK");

    /** @arg A positive rate is accepted, whatever the case of the market code */
    CHECK_NOTHROW(tm->setParam<string>("currency_rates", "HK:0.92, nasdaq:7.15"));
    CHECK_NOTHROW(tm->setParam<string>("currency_rates", ""));
    CHECK_NOTHROW(tm->setParam<string>("currency_rates", "12:0.5"));

    /** @arg A zero or negative rate is rejected, a malformed item as well */
    CHECK_THROWS_AS(tm->setParam<string>("currency_rates", "HK:0"), hku::exception);
    CHECK_THROWS_AS(tm->setParam<string>("currency_rates", "HK:-1"), hku::exception);
    CHECK_THROWS_AS(tm->setParam<string>("currency_rates", "HK"), hku::exception);

    /** @arg A valid expression is still accepted after a rejected one */
    CHECK_NOTHROW(tm->setParam<string>("currency_rates", "HK:0.9"));
    CHECK_THROWS_AS(tm->setParam<string>("currency_rates", "HK:0"), hku::exception);
    CHECK_NOTHROW(tm->setParam<string>("currency_rates", "HK:0.9, NASDAQ:7.1"));

    /** @arg A rejected value stays in the parameter table, so it has to be replaced before the
     * account is saved */
    CHECK_THROWS_AS(tm->setParam<string>("currency_rates", "HK:0"), hku::exception);
    CHECK_EQ(tm->getParam<string>("currency_rates"), "HK:0");
    CHECK_NOTHROW(tm->setParam<string>("currency_rates", "HK:0.9"));
}

/** @par Test point: the margin gates and the historical margin replay of a foreign market are
 * judged in the account currency */
TEST_CASE("test_TradeManager_currency_rates_margin") {
    StockManager& sm = StockManager::instance();
    Stock us = sm.getStock("NASDAQNDX");
    REQUIRE_UNARY(!us.isNull());

    KData k = us.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);

    auto make_tm = [&](const string& name) {
        TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), name);
        tm->setParam<string>("currency_rates", "NASDAQ:7.0");
        tm->setParam<bool>("margin_accounting", true);
        tm->margin(MG_ByType(std::to_string(us.type()) + ":0.5"));
        return tm;
    };

    /** @arg a short opening whose margin exceeds the cash only after the conversion is rejected */
    TradeManagerPtr gate = make_tm("GATE");
    gate->setParam<bool>("support_borrow_stock", true);
    // The notional is 40000, its margin 20000 in the security currency but 140000 in the account
    CHECK_EQ(gate->sellShort(dt, us, 100.0, 400).business, BUSINESS_INVALID);

    /** @arg without the rate the same opening passes, so the gate is what rejects it */
    TradeManagerPtr plain = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "PLAIN");
    plain->setParam<bool>("margin_accounting", true);
    plain->setParam<bool>("support_borrow_stock", true);
    plain->margin(MG_ByType(std::to_string(us.type()) + ":0.5"));
    CHECK_EQ(plain->sellShort(dt, us, 100.0, 400).business, BUSINESS_SELL_SHORT);

    /** @arg a replay from an archive without the margin field reproduces the converted margin */
    TradeManagerPtr live = make_tm("LIVE");
    CHECK_EQ(live->buy(dt, us, 100.0, 10).business, BUSINESS_BUY);
    CHECK_EQ(live->getFunds().occupied_margin, roundEx(0.5 * 100.0 * 10 * us.unit() * 7.0, 2));

    TradeManagerPtr replayed = make_tm("REPLAY");
    for (const auto& tr : live->getTradeList()) {
        TradeRecord archived(tr);
        archived.margin = 0.0;  // simulate an archive written before the field existed
        CHECK_UNARY(replayed->addTradeRecord(archived));
    }
    CHECK_EQ(replayed->getFunds(dt, "DAY").occupied_margin, live->getFunds().occupied_margin);
    CHECK_EQ(replayed->getFunds(dt, "DAY").market_value, live->getFunds().market_value);

    /** @arg the quantity limit takes the funds in the account currency */
    TradeManagerPtr lim = make_tm("LIMIT");
    double atom = us.minTradeNumber() > 0.0 ? us.minTradeNumber() : 1.0;
    double expected = std::floor((70000.0 / 7.0) / (0.5 * 100.0 * atom * us.unit()) + 1e-9);
    CHECK_EQ(lim->getMaxOpenNumber(dt, us, 100.0, 70000.0, MarginSide::LONG), expected);
}

/** @par Test points: the occupied margin and the collateral haircut that enter the available funds
 * are measured in the account currency */
TEST_CASE("test_TradeManager_currency_rates_available_margin") {
    StockManager& sm = StockManager::instance();
    Stock us = sm.getStock("NASDAQNDX");
    REQUIRE_UNARY(!us.isNull());

    KData k = us.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);
    const price_t close = us.getMarketValue(dt, "DAY");
    REQUIRE_UNARY(close > 0.0);
    // Opened at half the close, so the floating profit of the holding is not zero
    const price_t px = roundEx(close * 0.5, 2);
    REQUIRE_UNARY(px > 0.0);
    const double number = 10.0;

    TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "AVAIL");
    tm->setParam<string>("currency_rates", "NASDAQ:7.0");
    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<bool>("support_borrow_cash", true);
    tm->margin(MG_ByType(std::to_string(us.type()) + ":0.5", 0.0, 0.6,
                         std::to_string(us.type()) + ":0.8"));
    REQUIRE_EQ(tm->buy(dt, us, px, number).business, BUSINESS_BUY);

    /** @arg the occupied margin of a foreign holding is reported in the account currency */
    const price_t margin0 = 0.5 * px * number * us.unit() * 7.0;
    const price_t occupied = 0.5 * close * number * us.unit() * 7.0;
    CHECK_EQ(tm->getFunds().occupied_margin, doctest::Approx(margin0).epsilon(0.001));
    CHECK_EQ(tm->getOccupiedMargin(dt), doctest::Approx(occupied).epsilon(0.001));

    /** @arg the collateral base converts the floating profit, the locked margin is already converted
     */
    const price_t base = margin0 + (close - px) * number * us.unit() * 7.0;
    CHECK_EQ(tm->getCollateralHaircut(dt), doctest::Approx(base * 0.2).epsilon(0.001));

    /** @arg the three terms of the available funds share one currency */
    const price_t net = tm->getFunds(dt, "DAY").net_assets();
    CHECK_EQ(tm->getAvailableMargin(dt),
             doctest::Approx(net - base * 0.2 - occupied).epsilon(0.001));
}

/** @par Test points: the daily settlement of a margin-accounted foreign holding marks to the
 * market in the account currency, and a collapsed equity arms the forced liquidation */
TEST_CASE("test_TradeManager_currency_rates_settlement") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());

    // Opened far above the market on the maximum quota the cash funds, so the settlement marks a
    // deep loss and the equity collapses
    KData k = sh.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);
    const price_t close = sh.getMarketValue(dt, "DAY");
    REQUIRE_UNARY(close > 0.0 && close < 100.0);
    const double number = 200.0;
    const price_t basis = roundEx(100.0 * number * sh.unit(), 2);
    const price_t margin0 = roundEx(0.5 * basis, 2) * 7.0;
    REQUIRE_UNARY(margin0 <= 100000.0);

    TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "SETTLE");
    tm->setParam<string>("currency_rates", "SH:7.0");
    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<bool>("support_borrow_cash", true);
    tm->setParam<bool>("force_liquidate", true);
    tm->margin(MG_ByType(std::to_string(sh.type()) + ":0.5"));

    CHECK_EQ(tm->buy(dt, sh, 100.0, number).business, BUSINESS_BUY);
    CHECK_EQ(tm->currentCash(), roundEx(100000.0 - margin0, 2));
    CHECK_EQ(tm->getFunds().occupied_margin, margin0);

    /** @arg the mark-to-market posting and the margin relock are converted */
    const price_t mv = roundEx(close * number * sh.unit(), 2);
    const price_t relock = roundEx(0.5 * mv, 2) * 7.0;
    REQUIRE_UNARY(tm->settlement(dt));
    price_t cash = roundEx(
      roundEx(100000.0 - margin0 + (mv - basis) * 7.0, 2) - roundEx(relock - margin0, 2), 2);
    CHECK_EQ(tm->currentCash(), doctest::Approx(cash));
    CHECK_EQ(tm->getFunds().occupied_margin, doctest::Approx(relock));

    /** @arg the collapsed equity arms the forced liquidation */
    CHECK_UNARY(tm->isForcedLiquidationQueued(dt));
}

/** @par Test points: the securities lending fee and the deposited/withdrawn aggregates of a
 * foreign market are converted into the account currency */
TEST_CASE("test_TradeManager_currency_rates_borrow_checkin") {
    StockManager& sm = StockManager::instance();
    Stock us = sm.getStock("NASDAQNDX");
    REQUIRE_UNARY(!us.isNull());

    KData k = us.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);
    const price_t notional = roundEx(100.0 * 10.0 * us.unit(), 2);

    TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, make_shared<FeeStubCost>(), "BC");
    tm->setParam<string>("currency_rates", "NASDAQ:7.0");

    /** @arg the lending fee of one unit in the security currency costs 7 in the account */
    CHECK_UNARY(tm->borrowStock(dt, us, 100.0, 10));
    CHECK_EQ(tm->currentCash(), 100000.0 - 7.0);
    CHECK_EQ(tm->getDebtNumber(dt, us), 10.0);
    CHECK_EQ(tm->getFunds().borrow_asset, notional * 7.0);

    /** @arg the returning fee is converted as well */
    CHECK_UNARY(tm->returnStock(dt, us, 100.0, 10));
    CHECK_EQ(tm->currentCash(), 100000.0 - 14.0);
    CHECK_EQ(tm->getFunds().borrow_asset, 0.0);

    /** @arg the deposited and withdrawn aggregates are converted */
    CHECK_UNARY(tm->checkinStock(dt, us, 100.0, 10));
    CHECK_EQ(tm->getFunds().base_asset, notional * 7.0);
    CHECK_UNARY(tm->checkoutStock(dt, us, 100.0, 10));
    CHECK_EQ(tm->getFunds().base_asset, 0.0);
}

/** @par Test point: a market missing from a configured table is reported once, however often it
 * is traded or valued */
TEST_CASE("test_TradeManager_currency_rates_warn_once") {
    StockManager& sm = StockManager::instance();
    Stock us = sm.getStock("NASDAQNDX");
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!us.isNull());
    REQUIRE_UNARY(!sh.isNull());

    KData k = us.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);

    WarnProbeTM tm;
    tm.setParam<string>("currency_rates", "HK:0.92");

    /** @arg the first trade of the unlisted market arms the warning */
    CHECK_EQ(tm.buy(dt, sh, 10.0, 100).business, BUSINESS_BUY);
    CHECK_EQ(tm.warnedCount(), 1);

    /** @arg further trades and valuations of the same market add no warning */
    CHECK_EQ(tm.sell(dt, sh, 11.0, 100).business, BUSINESS_SELL);
    tm.getFunds();
    tm.getFunds(dt, "DAY");
    CHECK_EQ(tm.warnedCount(), 1);

    /** @arg another unlisted market is reported once on its own */
    CHECK_EQ(tm.buy(dt, us, 100.0, 10).business, BUSINESS_BUY);
    CHECK_EQ(tm.warnedCount(), 2);
    CHECK_EQ(tm.sell(dt, us, 110.0, 10).business, BUSINESS_SELL);
    CHECK_EQ(tm.warnedCount(), 2);
}

#if HKU_SUPPORT_SERIALIZATION
/** @par Test points: the parsed rate table is rebuilt on load, an archive written before the
 * parameter existed falls back to the single-currency books, and a rejected expression must be
 * replaced before the account is saved */
TEST_CASE("test_TradeManager_currency_rates_archive") {
    StockManager& sm = StockManager::instance();
    Stock us = sm.getStock("NASDAQNDX");
    REQUIRE_UNARY(!us.isNull());
    KData k = us.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);
    const price_t money = roundEx(100.0 * 10.0 * us.unit(), 2);

    const string filename = sm.tmpdir() + "/tm_currency_rates.xml";
    auto save = [&](const TradeManagerPtr& src) {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(src);
    };
    auto load = [&]() {
        TradeManagerPtr dst;
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(dst);
        return dst;
    };

    TradeManagerPtr rated = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "ARCHIVE");
    rated->setParam<string>("currency_rates", "NASDAQ:7.0");
    CHECK_EQ(rated->buy(dt, us, 100.0, 10).business, BUSINESS_BUY);

    save(rated);
    TradeManagerPtr loaded = load();

    /** @arg the parameter and the books survive the round trip */
    CHECK_EQ(loaded->getParam<string>("currency_rates"), "NASDAQ:7.0");
    CHECK_EQ(loaded->currentCash(), rated->currentCash());
    CHECK_EQ(loaded->getFunds().market_value, rated->getFunds().market_value);

    /** @arg the rebuilt table still converts a trade issued after the load */
    CHECK_EQ(loaded->buy(dt, us, 100.0, 10).business, BUSINESS_BUY);
    CHECK_EQ(loaded->currentCash(), roundEx(rated->currentCash() - money * 7.0, 2));

    /** @arg an archive without the parameter values the holding at 1.0 */
    std::ostringstream oss;
    {
        boost::archive::xml_oarchive oa(oss);
        oa << BOOST_SERIALIZATION_NVP(rated);
    }
    string legacy = oss.str();
    const string key = "<name>currency_rates</name>";
    size_t pos = legacy.find(key);
    REQUIRE_UNARY(pos != string::npos);
    legacy.replace(pos, key.size(), "<name>currency_rates_legacy</name>");
    TradeManagerPtr upgraded;
    {
        std::istringstream iss(legacy);
        boost::archive::xml_iarchive ia(iss);
        ia >> BOOST_SERIALIZATION_NVP(upgraded);
    }
    CHECK_EQ(upgraded->getParam<string>("currency_rates"), "");
    TradeManagerPtr plain = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "LEGACY_PLAIN");
    CHECK_EQ(plain->buy(dt, us, 100.0, 10).business, BUSINESS_BUY);
    CHECK_EQ(upgraded->getFunds().market_value, plain->getFunds().market_value);

    /** @arg a rejected expression has to be replaced before the account is archived */
    TradeManagerPtr dirty = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "DIRTY");
    dirty->setParam<string>("currency_rates", "NASDAQ:7.0");
    CHECK_THROWS_AS(dirty->setParam<string>("currency_rates", "NASDAQ:0"), hku::exception);
    dirty->setParam<string>("currency_rates", "NASDAQ:7.0");
    CHECK_EQ(dirty->buy(dt, us, 100.0, 10).business, BUSINESS_BUY);
    save(dirty);
    TradeManagerPtr fixed = load();
    CHECK_EQ(fixed->getParam<string>("currency_rates"), "NASDAQ:7.0");
    CHECK_EQ(fixed->currentCash(), rated->currentCash());
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @par Test point: a clone keeps both the parameter and the parsed rate table */
TEST_CASE("test_TradeManager_currency_rates_clone") {
    StockManager& sm = StockManager::instance();
    Stock us = sm.getStock("NASDAQNDX");
    REQUIRE_UNARY(!us.isNull());
    KData k = us.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);
    const price_t money = roundEx(100.0 * 10.0 * us.unit(), 2);

    TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "SRC");
    tm->setParam<string>("currency_rates", "NASDAQ:7.0");
    CHECK_EQ(tm->buy(dt, us, 100.0, 10).business, BUSINESS_BUY);

    auto cloned = tm->clone();

    /** @arg the copy reproduces the converted books */
    CHECK_EQ(cloned->getParam<string>("currency_rates"), "NASDAQ:7.0");
    CHECK_EQ(cloned->currentCash(), tm->currentCash());
    CHECK_EQ(cloned->getFunds().market_value, tm->getFunds().market_value);

    /** @arg the parsed table of the copy is live, and the source is left alone */
    CHECK_EQ(cloned->buy(dt, us, 100.0, 10).business, BUSINESS_BUY);
    CHECK_EQ(cloned->currentCash(), roundEx(tm->currentCash() - money * 7.0, 2));
    CHECK_EQ(tm->currentCash(), roundEx(100000.0 - money * 7.0, 2));
}

/** @par Test point: the multi-date replay of getFundsList values a foreign holding at the same
 * rate as the per-date getFunds */
TEST_CASE("test_TradeManager_currency_rates_funds_list") {
    StockManager& sm = StockManager::instance();
    Stock us = sm.getStock("NASDAQNDX");
    REQUIRE_UNARY(!us.isNull());
    KData k = us.getKData(KQuery(-3));
    REQUIRE(k.size() >= 2);
    DatetimeList dates;
    for (size_t i = 0; i < k.size(); ++i) {
        dates.push_back(k[i].datetime);
    }
    const Datetime dt = dates.front();

    auto make = [&](const string& name, bool rated) {
        TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), name);
        if (rated) {
            tm->setParam<string>("currency_rates", "NASDAQ:7.0");
        }
        CHECK_EQ(tm->buy(dt, us, 100.0, 10).business, BUSINESS_BUY);
        return tm;
    };

    TradeManagerPtr rated = make("LIST_RATED", true);
    TradeManagerPtr plain = make("LIST_PLAIN", false);
    FundsList rated_list = rated->getFundsList(dates);
    FundsList plain_list = plain->getFundsList(dates);
    REQUIRE_EQ(rated_list.size(), dates.size());
    REQUIRE_EQ(plain_list.size(), dates.size());

    for (size_t i = 0; i < dates.size(); ++i) {
        /** @arg each date agrees with the single-date valuation */
        FundsRecord expect = rated->getFunds(dates[i]);
        CHECK_EQ(rated_list[i].market_value, expect.market_value);
        CHECK_EQ(rated_list[i].base_asset, expect.base_asset);
        CHECK_EQ(rated_list[i].borrow_asset, expect.borrow_asset);
        /** @arg every date carries the converted market value, not the raw one */
        CHECK_EQ(rated_list[i].market_value, doctest::Approx(plain_list[i].market_value * 7.0));
    }
}

/** @par Test points: a dividend keeps its amount in the record while the cash posting is
 * converted into the account currency */
TEST_CASE("test_TradeManager_currency_rates_dividend") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");
    REQUIRE_UNARY(!stk.isNull());

    TradeManagerPtr plain = crtTM(Datetime(199305010000), 100000.0, TC_Zero(), "DIV_PLAIN");
    TradeManagerPtr rated = crtTM(Datetime(199305010000), 100000.0, TC_Zero(), "DIV_RATED");
    rated->setParam<string>("currency_rates", std::to_string(stk.type()) + ":7.0");

    CHECK_EQ(plain->buy(Datetime(199305200000L), stk, 55.7, 100).business, BUSINESS_BUY);
    CHECK_EQ(rated->buy(Datetime(199305200000L), stk, 55.7, 100).business, BUSINESS_BUY);
    CHECK_EQ(plain->buy(Datetime(199305250000L), stk, 27.5, 100).business, BUSINESS_BUY);
    CHECK_EQ(rated->buy(Datetime(199305250000L), stk, 27.5, 100).business, BUSINESS_BUY);

    TradeRecordList plain_list = plain->getTradeList();
    TradeRecordList rated_list = rated->getTradeList();
    const TradeRecord* bp = nullptr;
    const TradeRecord* br = nullptr;
    for (const auto& tr : plain_list) {
        if (tr.business == BUSINESS_BONUS) {
            bp = &tr;
            break;
        }
    }
    for (const auto& tr : rated_list) {
        if (tr.business == BUSINESS_BONUS) {
            br = &tr;
            break;
        }
    }
    REQUIRE_UNARY(bp != nullptr);
    REQUIRE_UNARY(br != nullptr);

    /** @arg the record stays in the security's currency */
    CHECK_EQ(br->realPrice, bp->realPrice);
    CHECK_EQ(br->realPrice, 30.0);
    /** @arg the cash snapshot is converted */
    CHECK_EQ(bp->cash, 94460.0);
    CHECK_EQ(br->cash, 61220.0);
    CHECK_EQ(rated->cash(Datetime(199305240000L)), br->cash);

    /** @arg the dividend compensation paid on a borrowed stock is converted as well */
    TradeManagerPtr borrowed = crtTM(Datetime(199305010000), 100000.0, TC_Zero(), "DIV_BORROW");
    borrowed->setParam<string>("currency_rates", std::to_string(stk.type()) + ":7.0");
    borrowed->setParam<bool>("support_borrow_stock", true);
    CHECK_UNARY(borrowed->borrowStock(Datetime(199305200000L), stk, 55.7, 100));
    CHECK_EQ(borrowed->buy(Datetime(199305250000L), stk, 27.5, 100).business, BUSINESS_BUY);
    TradeRecordList borrowed_list = borrowed->getTradeList();
    const TradeRecord* bc = nullptr;
    for (const auto& tr : borrowed_list) {
        if (tr.business == BUSINESS_DIVIDEND_COMPENSATION) {
            bc = &tr;
            break;
        }
    }
    REQUIRE_UNARY(bc != nullptr);
    CHECK_EQ(bc->realPrice, 30.0);
    CHECK_EQ(bc->cash, 99790.0);

    /** @arg the reported dividend is in the account currency */
    Performance plain_perf, rated_perf;
    plain_perf.statistics(plain);
    rated_perf.statistics(rated);
    REQUIRE_UNARY(plain_perf["Total Dividends"] > 0.0);
    CHECK_EQ(rated_perf["Total Dividends"],
             doctest::Approx(plain_perf["Total Dividends"] * 7.0).epsilon(0.001));
}

/** @par Test points: the short proceeds, the buyback notional and its cash gate are all converted
 * into the account currency */
TEST_CASE("test_TradeManager_currency_rates_short_cash") {
    StockManager& sm = StockManager::instance();
    Stock us = sm.getStock("NASDAQNDX");
    REQUIRE_UNARY(!us.isNull());
    KData k = us.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);
    const price_t money = roundEx(100.0 * 10.0 * us.unit(), 2);

    auto make = [&](const string& name, bool rated) {
        TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), name);
        tm->margin(nullptr);
        tm->setParam<bool>("support_borrow_stock", true);
        if (rated) {
            tm->setParam<string>("currency_rates", "NASDAQ:7.0");
        }
        return tm;
    };

    TradeManagerPtr rated = make("SHORT_RATED", true);
    TradeManagerPtr plain = make("SHORT_PLAIN", false);
    CHECK_EQ(rated->sellShort(dt, us, 100.0, 10).business, BUSINESS_SELL_SHORT);
    CHECK_EQ(plain->sellShort(dt, us, 100.0, 10).business, BUSINESS_SELL_SHORT);

    /** @arg the proceeds and the buyback of a foreign short are converted */
    CHECK_EQ(rated->currentCash(), roundEx(100000.0 + money * 7.0, 2));
    CHECK_EQ(plain->currentCash(), roundEx(100000.0 + money, 2));
    CHECK_EQ(rated->buyShort(dt, us, 100.0, 10).business, BUSINESS_BUY_SHORT);
    CHECK_EQ(rated->currentCash(), 100000.0);

    /** @arg a buyback the plain books can afford is rejected once converted */
    TradeManagerPtr rated_gate = make("SHORT_GATE_RATED", true);
    TradeManagerPtr plain_gate = make("SHORT_GATE_PLAIN", false);
    CHECK_EQ(rated_gate->sellShort(dt, us, 100.0, 10).business, BUSINESS_SELL_SHORT);
    CHECK_EQ(plain_gate->sellShort(dt, us, 100.0, 10).business, BUSINESS_SELL_SHORT);
    CHECK_EQ(rated_gate->buyShort(dt, us, 5000.0, 10).business, BUSINESS_INVALID);
    CHECK_EQ(plain_gate->buyShort(dt, us, 5000.0, 10).business, BUSINESS_BUY_SHORT);
}

/** @par Test point: the securities lending interest is billed on the market value converted into
 * the account currency */
TEST_CASE("test_TradeManager_currency_rates_borrow_interest") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());

    KData k = sh.getKData(KQuery(-2));
    REQUIRE_EQ(k.size(), 2);
    const Datetime dt(k[0].datetime);
    const Datetime dt2(k[1].datetime);

    auto make = [&](const string& name, bool rated) {
        TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0,
                                   TC_MarginInterest(TC_Zero(), 0.0, "SH:0.36", 0.0), name);
        if (rated) {
            tm->setParam<string>("currency_rates", "SH:7.0");
        }
        tm->setParam<bool>("margin_accounting", true);
        tm->setParam<bool>("support_borrow_stock", true);
        return tm;
    };

    TradeManagerPtr plain = make("INT_PLAIN", false);
    TradeManagerPtr rated = make("INT_RATED", true);
    CHECK_UNARY(plain->borrowStock(dt, sh, 10.0, 100));
    CHECK_UNARY(rated->borrowStock(dt, sh, 10.0, 100));

    /** @arg the settlement bills both accounts, and the rated one on the converted base */
    plain->settlement(dt2);
    rated->settlement(dt2);
    const price_t plain_interest = plain->getFunds().borrow_cash;
    const price_t rated_interest = rated->getFunds().borrow_cash;
    REQUIRE_UNARY(plain_interest > 0.0);
    CHECK_EQ(rated_interest, doctest::Approx(plain_interest * 7.0).epsilon(0.01));
}

/** @par Test points: the account-level performance metrics are reported in the account currency,
 * so the closed-trade cost, the net profit and the cash usage all scale with the rate */
TEST_CASE("test_TradeManager_currency_rates_performance") {
    StockManager& sm = StockManager::instance();
    Stock us = sm.getStock("NASDAQNDX");
    REQUIRE_UNARY(!us.isNull());

    const Datetime dt_buy(201601040000L);
    const Datetime dt_sell(201701030000L);
    const TradeCostPtr cost = TC_HK(0.001, 0.0, 0.0, 0.0, 0.0);

    auto make = [&](const string& name, bool rated) {
        TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, cost, name);
        if (rated) {
            tm->setParam<string>("currency_rates", "NASDAQ:7.0");
        }
        return tm;
    };

    TradeManagerPtr plain = make("PERF_PLAIN", false);
    TradeManagerPtr rated = make("PERF_RATED", true);
    CHECK_EQ(plain->buy(dt_buy, us, 100.0, 10).business, BUSINESS_BUY);
    CHECK_EQ(rated->buy(dt_buy, us, 100.0, 10).business, BUSINESS_BUY);
    CHECK_EQ(plain->sell(dt_sell, us, 120.0, 10).business, BUSINESS_SELL);
    CHECK_EQ(rated->sell(dt_sell, us, 120.0, 10).business, BUSINESS_SELL);

    Performance plain_perf, rated_perf;
    plain_perf.statistics(plain);
    rated_perf.statistics(rated);

    /** @arg the closed-trade cost and the net profit are converted */
    const double plain_cost = plain_perf["Total Cost of Closed Trades"];
    const double rated_cost = rated_perf["Total Cost of Closed Trades"];
    REQUIRE_UNARY(plain_cost > 0.0);
    CHECK_EQ(rated_cost, doctest::Approx(plain_cost * 7.0).epsilon(0.001));

    const double plain_profit = plain_perf["Total Net Profit of Closed Trades"];
    const double rated_profit = rated_perf["Total Net Profit of Closed Trades"];
    REQUIRE_UNARY(plain_profit > 0.0);
    CHECK_EQ(rated_profit, doctest::Approx(plain_profit * 7.0).epsilon(0.001));

    /** @arg the cash usage keeps one currency on both sides: without the conversion the mixed
     * hold_cash / cash would give a ratio that is neither */
    const double plain_usage = plain_perf["Max Cash Usage per Trade %"];
    const double rated_usage = rated_perf["Max Cash Usage per Trade %"];
    REQUIRE_UNARY(plain_usage > 0.0);
    CHECK_EQ(rated_usage, doctest::Approx(plain_usage * 7.0).epsilon(0.001));
}

/** @par Test point: the money manager sizes a foreign buy by the account-currency unit price */
TEST_CASE("test_MoneyManager_currency_rates_buy_number") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    KData k = sh.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);

    auto buy_number = [&](bool rated) {
        TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "MM");
        if (rated) {
            tm->setParam<string>("currency_rates", "SH:7.0");
        }
        MoneyManagerPtr mm = MM_Nothing();
        mm->setTM(tm);
        return mm->getBuyNumber(dt, sh, 10.0, 5.0, PART_SYSTEM);
    };

    const double plain = buy_number(false);
    const double rated = buy_number(true);
    REQUIRE_UNARY(plain > 0.0);
    const double lot = sh.minTradeNumber();
    REQUIRE_UNARY(lot > 0.0);
    /** @arg the rated size is the plain size divided by the rate, floored to a board lot */
    CHECK_EQ(rated, doctest::Approx(std::floor((plain / 7.0) / lot) * lot).epsilon(0.001));
}

/** @par Test point: the AF mode A target shares are converted by the account-currency unit price */
TEST_CASE("test_AllocateFunds_currency_rates_modeA") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    KData k = sh.getKData(KQuery(-1));
    REQUIRE_UNARY(!k.empty());
    const Datetime dt(k.back().datetime);

    auto target_shares = [&](bool rated) {
        TradeManagerPtr tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero(), "AF");
        if (rated) {
            tm->setParam<string>("currency_rates", "SH:7.0");
        }
        TradeSuggestionList suggestions;
        TradeSuggestion s;
        s.stock = sh;
        s.plan_price = 10.0;
        s.target_position_ratio = 0.5;
        s.type = SuggestionType::BUY;
        suggestions.push_back(s);
        AF_EqualWeight()->allocateTargets(dt, tm, suggestions, AllocateFundsBase::Weights(),
                                          KQuery(), "A");
        REQUIRE_EQ(suggestions.size(), 1);
        return suggestions[0].number;
    };

    const double plain = target_shares(false);
    const double rated = target_shares(true);

    /** @arg the 50% target is 5000 shares on the plain books and 1/7 of that once converted */
    REQUIRE_UNARY(plain > 0.0);
    CHECK_EQ(plain, doctest::Approx(5000.0).epsilon(0.001));
    CHECK_EQ(rated, doctest::Approx(plain / 7.0).epsilon(0.001));
}

/** @} */
