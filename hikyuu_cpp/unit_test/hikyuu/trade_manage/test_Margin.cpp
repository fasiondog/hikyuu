/*
 * test_Margin.cpp
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/trade_manage/crt/MG_Fixed.h>
#include <hikyuu/trade_manage/crt/MG_None.h>
#include <hikyuu/trade_manage/crt/MG_ByType.h>

using namespace hku;

/**
 * @defgroup test_Margin test_Margin
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/* The component fixtures build the Stock explicitly, so no market data is needed.
 * unit = tickValue / tick; atom = minTradeNumber */
static Stock mg_mkA() {
    return Stock("SH", "600000", "A-share", STOCKTYPE_A, true, Datetime(199001010000),
                 Null<Datetime>(), 0.01, 0.01, 2, 100, 1000000);
}

static Stock mg_mkFut() {
    return Stock("SHF", "rb2510", "futures", 12, true, Datetime(199001010000), Null<Datetime>(),
                 1.0, 10.0, 0, 1, 100000);
}

static Stock mg_mkStock(const string& market, uint32_t type) {
    return Stock(market, "000001", "fixture", type, true, Datetime(199001010000), Null<Datetime>(),
                 0.01, 0.01, 2, 100, 1000000);
}

/** @par Test points */
TEST_CASE("test_MG_Fixed_rate_mode") {
    auto margin = MG_Fixed(0.6);
    Stock a = mg_mkA();
    Datetime dt(202001010000);

    /** @arg The name and the default parameters */
    CHECK_EQ(margin->name(), "MG_Fixed");
    CHECK_EQ(margin->getParam<double>("rate"), 0.6);

    /** @arg A-share: unit = 1, atom = 100, both sides use rate 0.6 */
    CHECK_EQ(margin->getOpenMargin(dt, a, 10.0, 100, MarginSide::LONG), doctest::Approx(600.0));
    CHECK_EQ(margin->getOpenMargin(dt, a, 10.0, 100, MarginSide::SHORT), doctest::Approx(600.0));
    CHECK_EQ(margin->getMarginRate(dt, a, MarginSide::LONG), 0.6);
    CHECK_EQ(margin->getMarginRate(dt, a, MarginSide::SHORT), 0.6);

    /** @arg With no maintain_rate set, the maintain margin equals the opening margin */
    CHECK_EQ(margin->getMaintainMargin(dt, a, 10.0, 100, MarginSide::LONG), doctest::Approx(600.0));

    /** @arg The available 10000 opens 1600 shares (16 lots) at 10.0 */
    CHECK_EQ(margin->getMaxOpenNumber(dt, a, 10.0, 10000.0, MarginSide::LONG), 1600.0);

    /** @arg The identity margin == rate * price * num * unit holds */
    CHECK_EQ(margin->getOpenMargin(dt, a, 9.75, 700, MarginSide::LONG),
             doctest::Approx(0.6 * 9.75 * 700 * a.unit()));
}

/** @par Test points */
TEST_CASE("test_MG_Fixed_short_and_maintain") {
    auto margin = MG_Fixed(0.1, 0.15, 0.08);
    Stock fut = mg_mkFut();
    Datetime dt(202001010000);

    /** @arg Futures: unit = 10, one lot */
    CHECK_EQ(margin->getOpenMargin(dt, fut, 4000.0, 1, MarginSide::LONG), doctest::Approx(4000.0));
    CHECK_EQ(margin->getOpenMargin(dt, fut, 4000.0, 1, MarginSide::SHORT), doctest::Approx(6000.0));
    CHECK_EQ(margin->getMarginRate(dt, fut, MarginSide::LONG), doctest::Approx(0.1));
    CHECK_EQ(margin->getMarginRate(dt, fut, MarginSide::SHORT), doctest::Approx(0.15));

    /** @arg The openable quantity of each side */
    CHECK_EQ(margin->getMaxOpenNumber(dt, fut, 4000.0, 100000.0, MarginSide::LONG), 25.0);
    CHECK_EQ(margin->getMaxOpenNumber(dt, fut, 4000.0, 100000.0, MarginSide::SHORT), 16.0);

    /** @arg The maintain margin scales by maintain_rate / rate */
    CHECK_EQ(margin->getMaintainMargin(dt, fut, 4000.0, 1, MarginSide::LONG),
             doctest::Approx(3200.0));

    /** @arg maxTradeNumber caps the openable quantity */
    CHECK_EQ(margin->getMaxOpenNumber(dt, fut, 4000.0, 1e9, MarginSide::LONG), 100000.0);
}

/** @par Test points */
TEST_CASE("test_MG_Fixed_per_lot") {
    auto margin = MG_Fixed(0.1, 0.0, 0.0, 5000.0);
    Stock fut = mg_mkFut();
    Datetime dt(202001010000);

    /** @arg In per-lot mode the margin no longer depends on the price */
    CHECK_EQ(margin->getOpenMargin(dt, fut, 1.1, 1, MarginSide::LONG), doctest::Approx(5000.0));
    CHECK_EQ(margin->getOpenMargin(dt, fut, 1.1, 3, MarginSide::LONG), doctest::Approx(15000.0));

    /** @arg The ratio view is meaningless in per-lot mode */
    CHECK_EQ(margin->getMarginRate(dt, fut, MarginSide::LONG), 1.0);

    /** @arg The available 26000 opens 5 lots */
    CHECK_EQ(margin->getMaxOpenNumber(dt, fut, 1.1, 26000.0, MarginSide::LONG), 5.0);
}

/** @par Test points */
TEST_CASE("test_MG_None") {
    auto margin = MG_None();
    Stock a = mg_mkA();
    Stock fut = mg_mkFut();
    Datetime dt(202001010000);

    /** @arg No security is covered by the margin accounting */
    CHECK_UNARY(!margin->isMarginTrading(a));
    CHECK_UNARY(!margin->isMarginTrading(fut));

    /** @arg The ratio view is 1.0 (no leverage) and the margin is the full nominal */
    CHECK_EQ(margin->getMarginRate(dt, a, MarginSide::LONG), 1.0);
    CHECK_EQ(margin->getOpenMargin(dt, a, 10.0, 100, MarginSide::LONG), doctest::Approx(1000.0));
    CHECK_EQ(margin->getMaxOpenNumber(dt, a, 10.0, 10000.0, MarginSide::LONG), 1000.0);

    /** @arg A stock short still goes through the borrow accounting */
    CHECK_UNARY(margin->needBorrowAsset(a));

    /** @arg Futures and crypto do not borrow */
    CHECK_UNARY(!margin->needBorrowAsset(fut));
    CHECK_UNARY(!margin->needBorrowAsset(mg_mkStock("BINANCE", STOCKTYPE_CRYPTO)));
}

/** @par Test points */
TEST_CASE("test_MG_Fixed_invalid_inputs") {
    auto margin = MG_Fixed(0.6);
    Stock a = mg_mkA();
    Datetime dt(202001010000);

    /** @arg A non-positive or NaN price / num gives 0 instead of throwing */
    CHECK_EQ(margin->getOpenMargin(dt, a, 0.0, 100, MarginSide::LONG), 0.0);
    CHECK_EQ(margin->getOpenMargin(dt, a, 0.0 / 0.0, 100, MarginSide::LONG), 0.0);
    CHECK_EQ(margin->getOpenMargin(dt, a, 10.0, 0.0, MarginSide::LONG), 0.0);
    CHECK_EQ(margin->getOpenMargin(dt, a, 10.0, -100.0, MarginSide::LONG), 0.0);

    /** @arg A non-positive or NaN available gives 0 as well */
    CHECK_EQ(margin->getMaxOpenNumber(dt, a, 10.0, 0.0, MarginSide::LONG), 0.0);
    CHECK_EQ(margin->getMaxOpenNumber(dt, a, 10.0, -1.0, MarginSide::LONG), 0.0);
    CHECK_EQ(margin->getMaxOpenNumber(dt, a, 10.0, 0.0 / 0.0, MarginSide::LONG), 0.0);

    /** @arg Insufficient funds opens nothing */
    CHECK_EQ(margin->getMaxOpenNumber(dt, a, 10.0, 599.0, MarginSide::LONG), 0.0);
}

/** @par Test points */
TEST_CASE("test_MG_ByType") {
    auto margin = MG_ByType("12:0.20,SHF:0.10", 0.0, 1.0);
    Datetime dt(202001010000);

    /** @arg A type key hits first, no matter which market the security is in */
    Stock futShf = mg_mkStock("SHF", 12);
    Stock futDce = mg_mkStock("DCE", 12);
    CHECK_EQ(margin->getMarginRate(dt, futShf, MarginSide::LONG), doctest::Approx(0.20));
    CHECK_EQ(margin->getMarginRate(dt, futDce, MarginSide::LONG), doctest::Approx(0.20));

    /** @arg Otherwise a market key hits */
    Stock other = mg_mkStock("SHF", 1);
    CHECK_EQ(margin->getMarginRate(dt, other, MarginSide::LONG), doctest::Approx(0.10));

    /** @arg With no key matched, default_rate is used */
    Stock us = mg_mkStock("NASDAQ", 1);
    CHECK_EQ(margin->getMarginRate(dt, us, MarginSide::LONG), 1.0);

    /** @arg The matched securities are covered by the margin accounting, the rest are not */
    CHECK_UNARY(margin->isMarginTrading(futShf));
    CHECK_UNARY(margin->isMarginTrading(futDce));
    CHECK_UNARY(margin->isMarginTrading(other));
    CHECK_UNARY(!margin->isMarginTrading(us));

    /** @arg The identity margin == rate * price * num * unit holds */
    Stock fut = mg_mkFut();
    CHECK_EQ(margin->getOpenMargin(dt, fut, 4000.0, 1, MarginSide::LONG), doctest::Approx(8000.0));
}

/** @par Test points */
TEST_CASE("test_MG_ByType_duplicated_key") {
    /** @arg A duplicated key keeps the last ratio and only warns */
    auto margin = MG_ByType("SHF:0.10, SHF:0.12", 0.0, 1.0);
    CHECK_EQ(margin->getMarginRate(Datetime(202001010000), mg_mkStock("SHF", 1), MarginSide::LONG),
             doctest::Approx(0.12));
}

/** @par Test points */
TEST_CASE("test_MG_parameters_validation") {
    /** @arg rate must be in (0, 1] */
    CHECK_THROWS(MG_Fixed(0.0));
    CHECK_THROWS(MG_Fixed(1.1));

    /** @arg rate_short must also be in (0, 1] when set */
    CHECK_THROWS(MG_Fixed(0.1, 1.5));

    /** @arg maintain_rate must stay at or below the opening rate of every side (a higher one
     *     would call the account the moment a position is opened) */
    CHECK_NOTHROW(MG_Fixed(0.1, 0.0, 0.05));
    CHECK_THROWS(MG_Fixed(0.1, 0.0, 0.15));
    // The short side counts too: 0.06 is above its 0.05 opening ratio
    CHECK_THROWS(MG_Fixed(0.1, 0.05, 0.06));

    /** @arg margin_per_lot must not be negative */
    CHECK_THROWS(MG_Fixed(0.1, 0.0, 0.0, -1.0));

    /** @arg An invalid rates expression throws instead of silently falling back */
    CHECK_THROWS(MG_ByType("SHF:abc"));
    CHECK_THROWS(MG_ByType("SHF:0"));
    CHECK_THROWS(MG_ByType("SHF:1.2"));
    CHECK_THROWS(MG_ByType("SHF"));

    /** @arg default_rate must be in (0, 1] */
    CHECK_THROWS(MG_ByType("", 0.0, 0.0));

    /** @arg ByType: maintain_rate must stay at or below every keyed opening ratio */
    CHECK_NOTHROW(MG_ByType("12:0.95", 0.5, 0.6));
    CHECK_THROWS(MG_ByType("12:0.2", 0.5, 0.6));
}

/** @par Test points */
TEST_CASE("test_MG_clone") {
    /** @arg clone keeps the name and the parameters */
    auto margin = MG_ByType("12:0.20", 0.0, 0.8);
    auto copied = margin->clone();
    CHECK_EQ(copied->name(), margin->name());
    CHECK_EQ(copied->getParam<string>("rates"), "12:0.20");
    CHECK_EQ(copied->getMarginRate(Datetime(202001010000), mg_mkStock("SHF", 12), MarginSide::LONG),
             doctest::Approx(0.20));
}

/** @par Test points */
TEST_CASE("test_MG_ByType_parse_is_exception_safe") {
    Datetime dt(202001010000);
    Stock fut = mg_mkStock("SHF", 12);

    auto margin = MG_ByType("12:0.2");
    CHECK_EQ(margin->getMarginRate(dt, fut, MarginSide::LONG), doctest::Approx(0.2));

    /** @arg A rejected rates expression leaves the previous table intact */
    CHECK_THROWS(margin->setParam<string>("rates", "2:0.3, bad"));
    CHECK_EQ(margin->getMarginRate(dt, fut, MarginSide::LONG), doctest::Approx(0.2));

    /** @arg Restoring the previously valid expression is a no-op that keeps the table */
    margin->setParam<string>("rates", "12:0.2");
    CHECK_EQ(margin->getMarginRate(dt, fut, MarginSide::LONG), doctest::Approx(0.2));

    /** @arg A quota equal to exactly one unit's margin is not lost to floating-point rounding */
    auto fixed = MG_Fixed(0.6);
    Stock a = mg_mkA();
    CHECK_EQ(fixed->getMaxOpenNumber(dt, a, 1.11, 66.6, MarginSide::LONG), 100.0);
}

/** @} */
