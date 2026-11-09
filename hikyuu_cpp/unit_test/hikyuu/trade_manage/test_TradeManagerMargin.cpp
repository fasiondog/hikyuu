/*
 * test_TradeManagerMargin.cpp
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <hikyuu/config.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crtTM.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_Zero.h>
#include <hikyuu/trade_manage/margin/crt/MG_Fixed.h>
#include <hikyuu/trade_manage/margin/crt/MG_None.h>
#include <hikyuu/trade_manage/margin/crt/MG_ByType.h>

#if HKU_SUPPORT_SERIALIZATION
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_TradeManagerMargin test_TradeManagerMargin
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** A margin model that opens a short without borrowing, like a futures account */
class MG_TestNoBorrow : public MarginBase {
    MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MG_TestNoBorrow() : MarginBase("MG_TestNoBorrow") {
        setParam<double>("rate", 0.1);
    }

    bool needBorrowAsset(const Stock& stock) const override {
        return false;
    }

    price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price, double num,
                          MarginSide side) const override {
        return getParam<double>("rate") * price * num * stock.unit();
    }

    double getMarginRate(const Datetime& datetime, const Stock& stock,
                         MarginSide side) const override {
        return getParam<double>("rate");
    }

    MarginPtr _clone() override {
        return make_shared<MG_TestNoBorrow>();
    }
};

/** A margin model that keeps the A-share securities lending semantics for a short opening */
class MG_TestSecuritiesLending : public MarginBase {
    MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MG_TestSecuritiesLending() : MarginBase("MG_TestSecuritiesLending") {
        setParam<double>("rate", 0.1);
    }

    price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price, double num,
                          MarginSide side) const override {
        return getParam<double>("rate") * price * num * stock.unit();
    }

    double getMarginRate(const Datetime& datetime, const Stock& stock,
                         MarginSide side) const override {
        return getParam<double>("rate");
    }

    MarginPtr _clone() override {
        return make_shared<MG_TestSecuritiesLending>();
    }
};

/** A margin model standing in for one defined in Python, which the account must report upward */
class MG_TestPython : public MarginBase {
    MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MG_TestPython() : MarginBase("MG_TestPython") {
        m_is_python_object = true;
    }

    price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price, double num,
                          MarginSide side) const override {
        return nominal(stock, price, num);
    }

    MarginPtr _clone() override {
        return make_shared<MG_TestPython>();
    }
};

/** A margin model that reports no requirement at all for the securities it covers */
class MG_TestZeroRequirement : public MarginBase {
    MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MG_TestZeroRequirement() : MarginBase("MG_TestZeroRequirement") {}

    price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price, double num,
                          MarginSide side) const override {
        return 0.0;
    }

    double getMarginRate(const Datetime& datetime, const Stock& stock,
                         MarginSide side) const override {
        return 0.0;
    }

    MarginPtr _clone() override {
        return make_shared<MG_TestZeroRequirement>();
    }
};

/** An account that books the margin securities by margin (Phase B accounting) */
static TradeManagerPtr crtMarginAccountTM(const Datetime& datetime, price_t init_cash) {
    auto tm = crtTM(datetime, init_cash, TC_Zero(), "SYS", make_shared<MG_TestNoBorrow>());
    tm->setParam<bool>("margin_accounting", true);
    return tm;
}

namespace {

/**
 * Overrides the variety-level contract multiplier of a loaded security for the duration of a case
 * (the runtime override of Stock::tickValue; the fixture keeps the neutral type-level unit of 1, so
 * without it a missing multiplication would stay invisible). A case using it must write its
 * expectations with the multiplier as a literal instead of reading stock.unit() back. It changes
 * the data shared by the whole process, so the cases may not run in parallel
 */
class ScopedMultiplier {
public:
    ScopedMultiplier(Stock stock, double multiplier)
    : m_stock(stock), m_tickValue(stock.tickValue()) {
        m_stock.tickValue(m_tickValue * multiplier);
    }
    ~ScopedMultiplier() {
        m_stock.tickValue(m_tickValue);
    }

private:
    Stock m_stock;
    price_t m_tickValue;
};

}  // namespace

/** @par Test points */
TEST_CASE("test_TradeManager_margin_borrowing_power") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    auto tm = crtTM(dt, 4000.0, TC_Zero());
    tm->setParam<bool>("support_borrow_cash", true);

    /** @arg An empty account has the whole capital available and no occupation */
    CHECK_EQ(tm->getOccupiedMargin(dt), 0.0);
    CHECK_EQ(tm->getAvailableMargin(dt), 4000.0);

    /** @arg MG_Fixed(0.6) turns the available funds into a quantity aligned to the trade unit */
    double expect = std::floor(4000.0 / (0.6 * close * stock.unit()) / stock.minTradeNumber()) *
                    stock.minTradeNumber();
    CHECK_EQ(tm->getMaxOpenNumber(dt, stock, close, tm->getAvailableMargin(dt), MarginSide::LONG),
             expect);

    /** @arg The full available amount can be bought with the borrowed difference */
    auto trade = tm->buy(dt, stock, close, expect, 0.0, 0.0, close, PART_SIGNAL);
    CHECK_EQ(trade.business, BUSINESS_BUY);

    /** @arg After the buy the occupation is the rate times the position nominal */
    FundsRecord funds = tm->getFunds(dt, KQuery::DAY);
    CHECK_EQ(funds.net_assets(), doctest::Approx(4000.0));
    CHECK_EQ(tm->getOccupiedMargin(dt), doctest::Approx(0.6 * close * expect * stock.unit()));
    CHECK_EQ(tm->getAvailableMargin(dt),
             doctest::Approx(funds.net_assets() - 0.6 * close * expect * stock.unit()));
}

/** @par Test points */
TEST_CASE("test_TradeManager_short_without_borrow") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    auto tm = crtTM(dt, 4000.0, TC_Zero());
    tm->margin(make_shared<MG_TestNoBorrow>());

    /** @arg The short does not create a borrow record and does not inflate the assets */
    double lots = std::floor(4000.0 / (0.1 * close * stock.unit()));
    auto trade = tm->sellShort(dt, stock, close, lots, 0.0, 0.0, close, PART_SIGNAL);
    CHECK_EQ(trade.business, BUSINESS_SELL_SHORT);
    CHECK_UNARY(tm->getBorrowStockList().empty());
    FundsRecord funds = tm->getFunds(dt, KQuery::DAY);
    CHECK_EQ(funds.borrow_asset, 0.0);
    CHECK_EQ(funds.net_assets(), doctest::Approx(4000.0));

    /** @arg The sale proceeds raise the cash but the occupation view caps the next short */
    CHECK_UNARY(funds.cash > 4000.0);
    CHECK_EQ(tm->getOccupiedMargin(dt), doctest::Approx(0.1 * close * lots * stock.unit()));
    CHECK_UNARY(tm->getAvailableMargin(dt) < 0.1 * close * stock.unit());
    auto again = tm->sellShort(dt, stock, close, 1.0, 0.0, 0.0, close, PART_SIGNAL);
    CHECK_EQ(again.business, BUSINESS_INVALID);

    /** @arg Covering the short releases the margin */
    auto cover = tm->buyShort(dt, stock, close, 1.0, 0.0, 0.0, close, PART_SIGNAL);
    CHECK_EQ(cover.business, BUSINESS_BUY_SHORT);
    CHECK_UNARY(tm->getBorrowStockList().empty());
}

/** @par Test points */
TEST_CASE("test_TradeManager_short_still_borrows_stocks") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    /** @arg With the default model a security that can be lent out still needs the borrow */
    auto tm = crtTM(dt, 40000.0, TC_Zero());
    auto rejected = tm->sellShort(dt, stock, close, 10.0, 0.0, 0.0, close, PART_SIGNAL);
    CHECK_EQ(rejected.business, BUSINESS_INVALID);

    /** @arg After borrowing explicitly the short goes through and the asset is recorded */
    REQUIRE_UNARY(tm->borrowStock(dt, stock, close, 10.0));
    auto trade = tm->sellShort(dt, stock, close, 10.0, 0.0, 0.0, close, PART_SIGNAL);
    CHECK_EQ(trade.business, BUSINESS_SELL_SHORT);
    CHECK_UNARY(!tm->getBorrowStockList().empty());
    CHECK_UNARY(tm->getFunds(dt, KQuery::DAY).borrow_asset > 0.0);
}

/** @par Test points */
TEST_CASE("test_TradeManager_mixed_account_occupation") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    /** @arg A security matched by no key with default_rate 1.0 is a plain cash position */
    auto tm = crtTM(dt, 40000.0, TC_Zero());
    tm->margin(MG_ByType("12:0.10", 0.0, 1.0));
    auto trade = tm->buy(dt, stock, close, 10.0, 0.0, 0.0, close, PART_SIGNAL);
    CHECK_EQ(trade.business, BUSINESS_BUY);
    CHECK_EQ(tm->getOccupiedMargin(dt), 0.0);
    CHECK_EQ(tm->getAvailableMargin(dt),
             doctest::Approx(tm->getFunds(dt, KQuery::DAY).net_assets()));
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_compatibility") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);

    /** @arg The legacy ratio view is kept for every model */
    auto tm = crtTM(dt, 10000.0, TC_Zero());
    CHECK_EQ(tm->getMarginRate(dt, stock), 0.6);
    tm->margin(MG_None());
    CHECK_EQ(tm->getMarginRate(dt, stock), 1.0);
    tm->margin(MG_ByType("12:0.20", 0.0, 0.7));
    CHECK_EQ(tm->getMarginRate(dt, stock), 0.7);

    /** @arg Assigning null falls back to the plain cash model */
    tm->margin(MarginPtr());
    CHECK_EQ(tm->margin()->name(), "MG_None");
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_serialize") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());
    Datetime dt(200001040000);

#if HKU_SUPPORT_SERIALIZATION
    auto tm = crtTM(dt, 10000.0, TC_Zero(), "SYS", MG_ByType("12:0.20", 0.0, 0.5));

    /** @arg The margin model survives an archive round trip */
    std::string ofname("test_margin_serialize.xml");
    {
        // The archive must die before the stream it writes to
        std::ofstream ofs(ofname.c_str());
        REQUIRE_UNARY(ofs.is_open());
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(tm);
    }

    TradeManagerPtr new_tm;
    {
        std::ifstream ifs(ofname.c_str());
        REQUIRE_UNARY(ifs.is_open());
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(new_tm);
    }
    std::remove(ofname.c_str());

    CHECK_EQ(new_tm->margin()->name(), "MG_ByType");
    CHECK_EQ(new_tm->getMarginRate(dt, stock), 0.5);
    CHECK_EQ(new_tm->margin()->getParam<string>("rates"), "12:0.20");
#endif
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_serialize_old_archive") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());
    Datetime dt(200001040000);

#if HKU_SUPPORT_SERIALIZATION
    /** @arg An archive written before the margin model existed loads and keeps the default model */
    auto tm = crtTM(dt, 10000.0, TC_Zero(), "SYS", MG_ByType("12:0.20", 0.0, 0.5));

    std::string v1;
    {
        // The archive must die before the stream it writes to
        std::ostringstream oss;
        {
            boost::archive::xml_oarchive oa(oss);
            oa << BOOST_SERIALIZATION_NVP(tm);
        }
        v1 = oss.str();
    }

    // Downgrade by hand: drop the margin node and report the version of the account base class
    // that did not carry it yet
    const std::string base_tag("<TradeManagerBase ");
    size_t base_pos = v1.find(base_tag);
    REQUIRE_NE(base_pos, std::string::npos);
    size_t ver_pos = v1.find("version=\"1\"", base_pos);
    REQUIRE_NE(ver_pos, std::string::npos);
    std::string v0(v1);
    v0.replace(ver_pos, 11, "version=\"0\"");

    const std::string open_tag("<m_margin ");
    const std::string close_tag("</m_margin>");
    size_t begin = v0.find(open_tag);
    REQUIRE_NE(begin, std::string::npos);
    size_t end = v0.find(close_tag, begin);
    REQUIRE_NE(end, std::string::npos);
    v0.erase(begin, end + close_tag.size() - begin);
    REQUIRE_EQ(v0.find("m_margin"), std::string::npos);

    TradeManagerPtr old_tm;
    {
        std::istringstream iss(v0);
        boost::archive::xml_iarchive ia(iss);
        ia >> BOOST_SERIALIZATION_NVP(old_tm);
    }

    /** @arg The account falls back to the legacy financing model of its constructor */
    REQUIRE_UNARY(old_tm->margin() != nullptr);
    CHECK_EQ(old_tm->margin()->name(), "MG_Fixed");
    CHECK_EQ(old_tm->getMarginRate(dt, stock), 0.6);

    // An archive written before the switch existed carries no margin_accounting parameter either:
    // renaming the key in place keeps the archive layout and drops the entry from the account
    std::string v0b(v0);
    const std::string item_key("<name>margin_accounting</name>");
    size_t key_pos = v0b.find(item_key);
    REQUIRE_NE(key_pos, std::string::npos);
    v0b.replace(key_pos, item_key.size(), "<name>margin_accountinX</name>");
    REQUIRE_EQ(v0b.find("margin_accounting"), std::string::npos);

    TradeManagerPtr legacy_tm;
    {
        std::istringstream iss(v0b);
        boost::archive::xml_iarchive ia(iss);
        ia >> BOOST_SERIALIZATION_NVP(legacy_tm);
    }

    /** @arg A legacy archive without the switch keeps the cash accounting instead of throwing */
    CHECK_UNARY(!legacy_tm->getParam<bool>("margin_accounting"));
    CHECK_UNARY(!legacy_tm->isMarginAccounted(stock));
    REQUIRE_EQ(legacy_tm->buy(dt, stock, 10.0, 100.0, 0.0, 0.0, 10.0, PART_SIGNAL).business,
               BUSINESS_BUY);
    CHECK_EQ(legacy_tm->getPosition(dt, stock).margin, 0.0);

    /** @arg The rest of the archive is untouched */
    CHECK_EQ(old_tm->name(), "SYS");
    CHECK_EQ(old_tm->initCash(), 10000.0);
#endif
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_clone_keeps_rates") {
    StockManager& sm = StockManager::instance();
    Stock index = sm["sh000001"];
    REQUIRE_UNARY(!index.isNull());
    Datetime dt(200001040000);

    /** @arg The clone of a keyed model answers from its own table, not from the fallback ratio */
    auto mg = MG_ByType("2:0.20", 0.0, 0.9);
    auto cloned = mg->clone();
    REQUIRE_UNARY(cloned != nullptr);
    CHECK_EQ(cloned->name(), "MG_ByType");
    CHECK_EQ(cloned->getMarginRate(dt, index, MarginSide::LONG), 0.20);

    /** @arg An account clone carries the very same model */
    auto tm = crtTM(dt, 10000.0, TC_Zero(), "SYS", mg);
    auto tm_cloned = tm->clone();
    REQUIRE_UNARY(tm_cloned->margin() != nullptr);
    CHECK_EQ(tm_cloned->margin()->name(), "MG_ByType");
    CHECK_EQ(tm_cloned->getMarginRate(dt, index), 0.20);
}

/** @par Test points */
TEST_CASE("test_TradeManager_short_quota_rejects_whole_order") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    /** @arg With the securities lending off, the margin quota is the only cap and an order one
     *     unit over it is rejected whole instead of being scaled down to the quota */
    auto tm = crtTM(dt, 4000.0, TC_Zero());
    CHECK_EQ(tm->getParam<bool>("support_borrow_stock"), false);

    double atom = stock.minTradeNumber();
    price_t per_atom = tm->getOpenMargin(dt, stock, close, atom, MarginSide::SHORT);
    REQUIRE_UNARY(per_atom > 0.0);
    double quota =
      tm->getMaxOpenNumber(dt, stock, close, tm->getAvailableMargin(dt), MarginSide::SHORT);
    REQUIRE_UNARY(quota >= atom);

    // Borrow far more than the quota can pay for, so the lending pool never binds
    REQUIRE_UNARY(tm->borrowStock(dt, stock, close, 100000.0));

    auto trade = tm->sellShort(dt, stock, close, quota + atom, 0.0, 0.0, close, PART_SIGNAL);
    CHECK_EQ(trade.business, BUSINESS_INVALID);
    CHECK_EQ(trade.number, 0.0);
    CHECK_UNARY(tm->getShortPositionList().empty());

    /** @arg The quota itself still goes through in full */
    auto ok = tm->sellShort(dt, stock, close, quota, 0.0, 0.0, close, PART_SIGNAL);
    CHECK_EQ(ok.business, BUSINESS_SELL_SHORT);
    CHECK_EQ(ok.number, quota);
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_has_python_part") {
    Datetime dt(200001040000);
    auto tm = crtTM(dt, 10000.0, TC_Zero());

    /** @arg The built-in models keep the account pickable for a parallel run */
    CHECK_UNARY(!tm->hasPythonPart());

    /** @arg A model defined in Python is reported upward, so the system never loses the GIL */
    tm->margin(make_shared<MG_TestPython>());
    CHECK_UNARY(tm->hasPythonPart());

    /** @arg A clone carries the flag too */
    CHECK_UNARY(tm->clone()->hasPythonPart());

    /** @arg Swapping back to a built-in model clears it */
    tm->margin(MG_None());
    CHECK_UNARY(!tm->hasPythonPart());
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_zero_price_placeholder") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);

    /** @arg A financing account accepts the realPrice=0 market-order placeholder like the legacy
     *     behaviour: the quota cannot be measured without a price, so the gate is skipped and the
     *     plain cash/borrow logic decides */
    auto tm = crtTM(dt, 10000.0, TC_Zero());
    tm->setParam<bool>("support_borrow_cash", true);
    auto buy = tm->buy(dt, stock, 0.0, 100.0);
    CHECK_EQ(buy.business, BUSINESS_BUY);
    CHECK_EQ(buy.number, 100.0);
    CHECK_EQ(buy.realPrice, 0.0);
    CHECK_EQ(buy.cash, 10000.0);

    /** @arg A short opening with the placeholder price is not rejected by the quota either */
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);
    auto tm2 = crtTM(dt, 4000.0, TC_Zero());
    // The index type asks for a borrow record first when the lending is not automatic
    REQUIRE_UNARY(tm2->borrowStock(dt, stock, close, 100000.0));
    auto short_open = tm2->sellShort(dt, stock, 0.0, 100.0);
    CHECK_EQ(short_open.business, BUSINESS_SELL_SHORT);
    CHECK_EQ(short_open.number, 100.0);
    CHECK_EQ(tm2->getShortPositionList().size(), 1);
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_gate_tolerance") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh600000"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);

    /** @arg A quota exactly equal to one lot's margin is not lost to floating-point rounding
     *     (66.6 / (0.6 * 1.11 * 100) rounds just below 1 in double precision) */
    auto tm = crtTM(dt, 66.60, TC_Zero());
    tm->setParam<bool>("support_borrow_cash", true);
    auto one_lot = tm->buy(dt, stock, 1.11, 100);
    CHECK_EQ(one_lot.business, BUSINESS_BUY);
    CHECK_EQ(one_lot.number, 100.0);

    /** @arg An odd-lot order whose own margin still fits is admitted, although the floored
     *     whole-unit quota would have rejected it */
    auto tm2 = crtTM(dt, 10000.0, TC_Zero());
    tm2->setParam<bool>("support_borrow_cash", true);
    auto odd = tm2->buy(dt, stock, 10.0, 1650);
    CHECK_EQ(odd.business, BUSINESS_BUY);
    CHECK_EQ(odd.number, 1650.0);

    /** @arg An order whose own margin exceeds the available one is still rejected (a fresh
     *     account keeps the available amount equal to the cash, independent of the market price) */
    auto tm3 = crtTM(dt, 10000.0, TC_Zero());
    tm3->setParam<bool>("support_borrow_cash", true);
    auto over = tm3->buy(dt, stock, 10.0, 1700);
    CHECK_EQ(over.business, BUSINESS_INVALID);
    CHECK_EQ(over.number, 0.0);

    /** @arg A past date is rejected instead of mixing a replayed past value with today's
     *     occupied margin */
    CHECK_THROWS(tm->getAvailableMargin(Datetime(199001010000)));
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_long") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    auto tm = crtMarginAccountTM(dt, 100000.0);
    double number = 100.0;
    price_t margin = 0.1 * close * number * stock.unit();

    /** @arg The opening locks the margin only, the notional is just accumulated for reporting */
    auto trade = tm->buy(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL);
    REQUIRE_EQ(trade.business, BUSINESS_BUY);
    CHECK_EQ(trade.cash, doctest::Approx(100000.0 - margin));
    CHECK_EQ(tm->currentCash(), doctest::Approx(100000.0 - margin));

    PositionRecord position = tm->getPosition(dt, stock);
    CHECK_EQ(position.margin, doctest::Approx(margin));
    CHECK_EQ(position.buyMoney, doctest::Approx(close * number * stock.unit()));

    /** @arg The equity is unchanged right after the opening: the locked margin is still ours */
    FundsRecord funds = tm->getFunds(dt, KQuery::DAY);
    CHECK_EQ(funds.occupied_margin, doctest::Approx(margin));
    CHECK_EQ(funds.market_value, doctest::Approx(margin));
    CHECK_EQ(funds.net_assets(), doctest::Approx(100000.0));

    /** @arg The cash books are far more restrictive than the margin quota */
    double quota =
      tm->getMaxOpenNumber(dt, stock, close, tm->getAvailableMargin(dt), MarginSide::LONG);
    CHECK_UNARY(quota > number);

    /** @arg Closing at the same price returns the whole margin and leaves the cash intact */
    auto sell = tm->sell(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL);
    REQUIRE_EQ(sell.business, BUSINESS_SELL);
    CHECK_EQ(tm->currentCash(), doctest::Approx(100000.0));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).occupied_margin, 0.0);
    CHECK_UNARY(!tm->have(stock));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(100000.0));
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_short") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    auto tm = crtMarginAccountTM(dt, 100000.0);
    double number = 100.0;
    price_t margin = 0.1 * close * number * stock.unit();

    /** @arg A short opening locks the margin instead of crediting the sale proceeds */
    auto open = tm->sellShort(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL);
    REQUIRE_EQ(open.business, BUSINESS_SELL_SHORT);
    CHECK_EQ(tm->currentCash(), doctest::Approx(100000.0 - margin));
    CHECK_EQ(tm->getShortPosition(stock).margin, doctest::Approx(margin));

    FundsRecord funds = tm->getFunds(dt, KQuery::DAY);
    CHECK_EQ(funds.market_value, doctest::Approx(margin));
    CHECK_EQ(funds.short_market_value, 0.0);
    CHECK_EQ(funds.occupied_margin, doctest::Approx(margin));
    CHECK_EQ(funds.net_assets(), doctest::Approx(100000.0));

    /** @arg Covering below the opening price returns the margin plus the realized profit */
    price_t cover_price = close * 0.9;
    auto cover = tm->buyShort(dt, stock, cover_price, number, 0.0, 0.0, cover_price, PART_SIGNAL);
    REQUIRE_EQ(cover.business, BUSINESS_BUY_SHORT);
    CHECK_EQ(tm->currentCash(),
             doctest::Approx(100000.0 + (close - cover_price) * number * stock.unit()));
    CHECK_UNARY(!tm->haveShort(stock));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).occupied_margin, 0.0);
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_short_partial_close") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    Datetime dt2(200001050000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    price_t close2 = stock.getMarketValue(dt2, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);
    REQUIRE_UNARY(close2 > 0.0);

    auto tm = crtMarginAccountTM(dt, 100000.0);
    double number = 400.0;
    price_t margin = 0.1 * close * number * stock.unit();
    REQUIRE_EQ(tm->sellShort(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_SELL_SHORT);

    /** @arg A partial cover releases the margin of the covered part and realizes its profit */
    auto cover = tm->buyShort(dt2, stock, close2, 200.0, 0.0, 0.0, close2, PART_SIGNAL);
    REQUIRE_EQ(cover.business, BUSINESS_BUY_SHORT);
    double realized = (close - close2) * 200.0 * stock.unit();
    CHECK_EQ(tm->currentCash(), doctest::Approx(100000.0 - margin + margin * 0.5 + realized));

    /** @arg The locked margin follows the remaining short quantity */
    PositionRecord position = tm->getShortPosition(stock);
    CHECK_EQ(position.number, 200.0);
    CHECK_EQ(position.margin, doctest::Approx(margin * 0.5));

    /** @arg The occupation is half of the opening one and the equity keeps the released part */
    FundsRecord funds = tm->getFunds(dt2, KQuery::DAY);
    CHECK_EQ(funds.occupied_margin, doctest::Approx(margin * 0.5));
    CHECK_EQ(funds.net_assets(), doctest::Approx(100000.0 + realized * 2.0));
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_short_funding") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    // A cash-basis long leaves little free cash while the net assets stay large, so the quota gate
    // admits the short and only the funding rule can stop it
    const double long_number = 700.0;
    const double short_number = 200.0;
    price_t need = 0.1 * close * short_number * stock.unit();

    /** @arg Without a financing facility the margin and the fee must come from the cash on hand */
    auto no_borrow = crtTM(dt, 1000000.0, TC_Zero(), "SYS", make_shared<MG_TestNoBorrow>());
    REQUIRE_EQ(no_borrow->buy(dt, stock, close, long_number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    no_borrow->setParam<bool>("margin_accounting", true);
    price_t cash_before = no_borrow->currentCash();
    REQUIRE_UNARY(cash_before < need);
    CHECK_EQ(
      no_borrow->sellShort(dt, stock, close, short_number, 0.0, 0.0, close, PART_SIGNAL).business,
      BUSINESS_INVALID);
    CHECK_EQ(no_borrow->currentCash(), doctest::Approx(cash_before));
    CHECK_EQ(no_borrow->getFunds(dt, KQuery::DAY).borrow_cash, 0.0);

    /** @arg A financing account borrows the shortfall instead of overdrawing the cash */
    auto fin = crtTM(dt, 1000000.0, TC_Zero(), "SYS", make_shared<MG_TestNoBorrow>());
    fin->setParam<bool>("support_borrow_cash", true);
    REQUIRE_EQ(fin->buy(dt, stock, close, long_number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    fin->setParam<bool>("margin_accounting", true);

    auto open = fin->sellShort(dt, stock, close, short_number, 0.0, 0.0, close, PART_SIGNAL);
    REQUIRE_EQ(open.business, BUSINESS_SELL_SHORT);
    CHECK_EQ(fin->currentCash(), doctest::Approx(0.0));
    FundsRecord funds = fin->getFunds(dt, KQuery::DAY);
    CHECK_EQ(funds.borrow_cash, doctest::Approx(need - cash_before));
    CHECK_EQ(funds.net_assets(), doctest::Approx(1000000.0));
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_overdraft_close") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    auto tm = crtMarginAccountTM(dt, 100000.0);
    double number = 100.0;
    REQUIRE_EQ(tm->sellShort(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_SELL_SHORT);

    /** @arg A cover beyond the released margin overdraws the cash instead of being rejected */
    price_t cover_price = close * 2.0;
    auto cover = tm->buyShort(dt, stock, cover_price, number, 0.0, 0.0, cover_price, PART_SIGNAL);
    REQUIRE_EQ(cover.business, BUSINESS_BUY_SHORT);
    price_t realized = (close - cover_price) * number * stock.unit();
    CHECK_UNARY(tm->currentCash() < 0.0);
    CHECK_EQ(tm->currentCash(), doctest::Approx(100000.0 + realized));
    CHECK_UNARY(!tm->haveShort(stock));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(100000.0 + realized));
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_placeholder_close") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    auto tm = crtMarginAccountTM(dt, 100000.0);
    double number = 100.0;
    price_t margin = 0.1 * close * number * stock.unit();
    REQUIRE_EQ(tm->buy(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);

    /** @arg A placeholder close is rejected: the locked margin can not be released blind */
    auto placeholder = tm->sell(dt, stock, 0.0, 50.0, 0.0, 0.0, 0.0, PART_SIGNAL);
    CHECK_EQ(placeholder.business, BUSINESS_INVALID);
    CHECK_EQ(tm->currentCash(), doctest::Approx(100000.0 - margin));
    PositionRecord position = tm->getPosition(dt, stock);
    CHECK_EQ(position.number, number);
    CHECK_EQ(position.margin, doctest::Approx(margin));

    /** @arg The priced close still releases the whole margin and restores the cash */
    REQUIRE_EQ(tm->sell(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_SELL);
    CHECK_EQ(tm->currentCash(), doctest::Approx(100000.0));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).occupied_margin, 0.0);

    /** @arg The short side rejects the placeholder cover the same way */
    auto short_tm = crtMarginAccountTM(dt, 100000.0);
    REQUIRE_EQ(short_tm->sellShort(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_SELL_SHORT);
    CHECK_EQ(short_tm->buyShort(dt, stock, 0.0, number, 0.0, 0.0, 0.0, PART_SIGNAL).business,
             BUSINESS_INVALID);
    CHECK_EQ(short_tm->getShortPosition(stock).number, number);
    CHECK_EQ(short_tm->getShortPosition(stock).margin, doctest::Approx(margin));
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_funds_list") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    Datetime dt2(200001050000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    price_t close2 = stock.getMarketValue(dt2, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);
    REQUIRE_UNARY(close2 > 0.0);

    auto tm = crtMarginAccountTM(dt, 1000000.0);
    REQUIRE_EQ(tm->buy(dt, stock, close, 400.0, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    REQUIRE_EQ(tm->sellShort(dt, stock, close, 200.0, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_SELL_SHORT);
    FundsRecord live1 = tm->getFunds(dt, KQuery::DAY);
    REQUIRE_EQ(tm->sell(dt2, stock, close2, 200.0, 0.0, 0.0, close2, PART_SIGNAL).business,
               BUSINESS_SELL);
    FundsRecord live2 = tm->getFunds(dt2, KQuery::DAY);

    /** @arg The fast replay list matches the per date getFunds of the live books */
    DatetimeList dates;
    dates.push_back(dt);
    dates.push_back(dt2);
    FundsList list = tm->getFundsList(dates, KQuery::DAY);
    REQUIRE_EQ(list.size(), 2);
    CHECK_EQ(list[0].cash, doctest::Approx(live1.cash));
    CHECK_EQ(list[0].market_value, doctest::Approx(live1.market_value));
    CHECK_EQ(list[0].short_market_value, doctest::Approx(live1.short_market_value));
    CHECK_EQ(list[0].occupied_margin, doctest::Approx(live1.occupied_margin));
    CHECK_EQ(list[0].net_assets(), doctest::Approx(live1.net_assets()));

    CHECK_EQ(list[1].market_value, doctest::Approx(live2.market_value));
    CHECK_EQ(list[1].occupied_margin, doctest::Approx(live2.occupied_margin));
    CHECK_EQ(list[1].net_assets(), doctest::Approx(live2.net_assets()));
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_partial_close") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    Datetime dt2(200001050000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    price_t close2 = stock.getMarketValue(dt2, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);
    REQUIRE_UNARY(close2 > 0.0);

    auto tm = crtMarginAccountTM(dt, 1000000.0);
    double number = 2000.0;
    price_t margin = 0.1 * close * number * stock.unit();
    REQUIRE_EQ(tm->buy(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);

    /** @arg A partial close releases the margin and realizes the profit of the sold part */
    auto sell = tm->sell(dt2, stock, close2, 1000.0, 0.0, 0.0, close2, PART_SIGNAL);
    REQUIRE_EQ(sell.business, BUSINESS_SELL);
    double realized = (close2 - close) * 1000.0 * stock.unit();
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 - margin + margin * 0.5 + realized));

    PositionRecord position = tm->getPosition(dt2, stock);
    CHECK_EQ(position.number, 1000.0);
    CHECK_EQ(position.margin, doctest::Approx(margin * 0.5));

    /** @arg The equity grows by the profit realized on the sold part plus the unrealized one */
    FundsRecord funds = tm->getFunds(dt2, KQuery::DAY);
    CHECK_EQ(funds.occupied_margin, doctest::Approx(margin * 0.5));
    CHECK_EQ(funds.net_assets(), doctest::Approx(1000000.0 + realized * 2.0));
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_gate") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    double atom = stock.minTradeNumber();

    /** @arg With the accounting off the cash still caps the opening at the full notional */
    auto cash_tm = crtTM(dt, 400000.0, TC_Zero(), "SYS", make_shared<MG_TestNoBorrow>());
    double cash_max = std::floor(400000.0 / (close * atom * stock.unit())) * atom;
    REQUIRE_UNARY(cash_max >= atom);
    CHECK_EQ(cash_tm->buy(dt, stock, close, cash_max, 0.0, 0.0, close, PART_SIGNAL).business,
             BUSINESS_BUY);
    CHECK_EQ(cash_tm->getPosition(dt, stock).margin, 0.0);
    CHECK_EQ(cash_tm->buy(dt, stock, close, atom, 0.0, 0.0, close, PART_SIGNAL).business,
             BUSINESS_INVALID);

    /** @arg With the accounting on the whole quota goes through and the surplus is rejected */
    auto tm = crtMarginAccountTM(dt, 400000.0);
    double quota =
      tm->getMaxOpenNumber(dt, stock, close, tm->getAvailableMargin(dt), MarginSide::LONG);
    REQUIRE_UNARY(quota >= atom);
    REQUIRE_EQ(tm->buy(dt, stock, close, quota, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    auto over = tm->buy(dt, stock, close, atom, 0.0, 0.0, close, PART_SIGNAL);
    CHECK_EQ(over.business, BUSINESS_INVALID);
    CHECK_EQ(over.number, 0.0);
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_not_eligible") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    /** @arg A share that needs a borrow without automatic financing keeps the cash accounting
     *     even with the switch on */
    auto tm = crtTM(dt, 1000000.0, TC_Zero());
    REQUIRE_EQ(tm->margin()->name(), "MG_Fixed");
    tm->setParam<bool>("margin_accounting", true);
    CHECK_UNARY(!tm->isMarginAccounted(stock));

    double number = 100.0;
    REQUIRE_EQ(tm->buy(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    CHECK_EQ(tm->getPosition(dt, stock).margin, 0.0);
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 - close * number * stock.unit()));

    /** @arg Clearing the switch keeps the same books for the same security */
    auto off_tm = crtTM(dt, 1000000.0, TC_Zero(), "SYS", make_shared<MG_TestNoBorrow>());
    CHECK_UNARY(!off_tm->isMarginAccounted(stock));
    REQUIRE_EQ(off_tm->buy(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    CHECK_EQ(off_tm->getPosition(dt, stock).margin, 0.0);
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_replay") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    Datetime dt2(200001050000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    price_t close2 = stock.getMarketValue(dt2, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);
    REQUIRE_UNARY(close2 > 0.0);

    auto tm = crtMarginAccountTM(dt, 1000000.0);
    REQUIRE_EQ(tm->buy(dt, stock, close, 200.0, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    FundsRecord live = tm->getFunds(dt, KQuery::DAY);
    REQUIRE_EQ(tm->sell(dt2, stock, close2, 100.0, 0.0, 0.0, close2, PART_SIGNAL).business,
               BUSINESS_SELL);

    /** @arg The replayed books of the past match the ones observed at that moment */
    FundsRecord replay = tm->getFunds(dt, KQuery::DAY);
    CHECK_EQ(replay.cash, doctest::Approx(live.cash));
    CHECK_EQ(replay.market_value, doctest::Approx(live.market_value));
    CHECK_EQ(replay.occupied_margin, doctest::Approx(live.occupied_margin));
    CHECK_EQ(replay.net_assets(), doctest::Approx(live.net_assets()));
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_accounting_serialize") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

#if HKU_SUPPORT_SERIALIZATION
    // A built-in model keeps the archive self-contained (the test models are not exported)
    auto tm = crtTM(dt, 100000.0, TC_Zero(), "SYS", MG_Fixed(0.1));
    tm->setParam<bool>("support_borrow_cash", true);
    tm->setParam<bool>("margin_accounting", true);
    double number = 100.0;
    price_t margin = 0.1 * close * number * stock.unit();
    REQUIRE_EQ(tm->buy(dt, stock, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);

    std::string ofname("test_margin_accounting_serialize.xml");
    {
        std::ofstream ofs(ofname.c_str());
        REQUIRE_UNARY(ofs.is_open());
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(tm);
    }

    TradeManagerPtr new_tm;
    {
        std::ifstream ifs(ofname.c_str());
        REQUIRE_UNARY(ifs.is_open());
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(new_tm);
    }
    std::remove(ofname.c_str());

    /** @arg The locked margin and the switch survive the round trip */
    CHECK_EQ(new_tm->getParam<bool>("margin_accounting"), true);
    REQUIRE_EQ(new_tm->getPositionList().size(), 1);
    CHECK_EQ(new_tm->getPositionList()[0].margin, doctest::Approx(margin));
    CHECK_EQ(new_tm->getFunds(dt, KQuery::DAY).occupied_margin, doctest::Approx(margin));

    /** @arg An archive written before the field existed loads as the plain cash accounting */
    FundsRecord funds(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    funds.occupied_margin = 8.0;
    std::string v1;
    {
        std::ostringstream oss;
        {
            boost::archive::xml_oarchive oa(oss);
            oa << boost::serialization::make_nvp("FundsRecord", funds);
        }
        v1 = oss.str();
    }

    const std::string tag("<FundsRecord ");
    size_t tag_pos = v1.find(tag);
    REQUIRE_NE(tag_pos, std::string::npos);
    size_t ver_pos = v1.find("version=\"1\"", tag_pos);
    REQUIRE_NE(ver_pos, std::string::npos);
    std::string v0(v1);
    v0.replace(ver_pos, 11, "version=\"0\"");

    const std::string open_tag("<occupied_margin>");
    const std::string close_tag("</occupied_margin>");
    size_t begin = v0.find(open_tag);
    REQUIRE_NE(begin, std::string::npos);
    size_t end = v0.find(close_tag, begin);
    REQUIRE_NE(end, std::string::npos);
    v0.erase(begin, end + close_tag.size() - begin);
    REQUIRE_EQ(v0.find("occupied_margin"), std::string::npos);

    FundsRecord old_funds;
    {
        std::istringstream iss(v0);
        boost::archive::xml_iarchive ia(iss);
        ia >> boost::serialization::make_nvp("FundsRecord", old_funds);
    }
    CHECK_EQ(old_funds.occupied_margin, 0.0);
    CHECK_EQ(old_funds.cash, 1.0);
    CHECK_EQ(old_funds.borrow_asset, 7.0);
#endif
}

/** @par Test points */
TEST_CASE("test_FundsRecord_occupied_margin") {
    /** @arg Defaults to 0 and is summed by the record arithmetic */
    FundsRecord a;
    FundsRecord b;
    b.occupied_margin = 120.0;
    CHECK_EQ(a.occupied_margin, 0.0);
    CHECK_EQ((a + b).occupied_margin, 120.0);
    a += b;
    CHECK_EQ(a.occupied_margin, 120.0);

    /** @arg Takes part in the comparison and in the string form */
    FundsRecord c;
    c.occupied_margin = 1.0;
    CHECK_UNARY(!(a == c));
    std::ostringstream oss;
    oss << a;
    CHECK_UNARY(oss.str().find("120") != std::string::npos);
}

/** @par Test points */
TEST_CASE("test_PositionRecord_margin") {
    PositionRecord record;
    CHECK_EQ(record.margin, 0.0);

    /** @arg The field follows the copy, the move and the comparison */
    record.margin = 5.0;
    record.number = 100.0;
    record.totalNumber = 100.0;
    record.buyMoney = 1000.0;

    PositionRecord copied(record);
    CHECK_EQ(copied.margin, 5.0);

    PositionRecord moved(std::move(copied));
    CHECK_EQ(moved.margin, 5.0);

    PositionRecord other(record);
    other.margin = 6.0;
    CHECK_UNARY(!(moved == other));

    /** @arg The string form reports it */
    CHECK_UNARY(record.str().find("5.00") != std::string::npos);
}

/** @par Test points: the futures path books on the money basis, live matching the replay */
TEST_CASE("test_TradeManager_margin_accounting_futures_unit") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());
    // The expectations carry rb.unit() throughout, so the case stays exact whatever multiplier the
    // fixture data defines; it locks that the live and the replay share one money-basis formula
    double unit = rb.unit();

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    double number = 10.0;
    price_t nominal = roundEx(close * number * unit, 7);
    price_t margin = roundEx(0.1 * nominal, 7);

    auto tm = crtMarginAccountTM(dt, 1000000.0);
    REQUIRE_EQ(tm->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business, BUSINESS_BUY);

    /** @arg The opening locks only the margin, the notional stays out of the books */
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 - margin));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(1000000.0));

    /** @arg The replay of the opening books to the same date agrees with the live equity */
    CHECK_EQ(tm->getFunds(Datetime(200903272359), KQuery::DAY).net_assets(),
             doctest::Approx(tm->getFunds(dt, KQuery::DAY).net_assets()));

    /** @arg Closing one point higher realizes the money-basis profit */
    price_t p2 = close + 10000;
    price_t realized = roundEx(10000.0 * number * unit, 7);
    REQUIRE_EQ(tm->sell(dt, rb, p2, number, 0.0, 0.0, p2, PART_SIGNAL).business, BUSINESS_SELL);
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 + realized));

    /** @arg A short open and cover keeps the equity to the money-basis P&L as well */
    auto tm2 = crtMarginAccountTM(dt, 1000000.0);
    REQUIRE_EQ(tm2->sellShort(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_SELL_SHORT);
    CHECK_EQ(tm2->currentCash(), doctest::Approx(1000000.0 - margin));
    CHECK_EQ(tm2->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(1000000.0));
    REQUIRE_EQ(tm2->buyShort(dt, rb, p2, number, 0.0, 0.0, p2, PART_SIGNAL).business,
               BUSINESS_BUY_SHORT);
    // Covering higher is a loss on the short side
    CHECK_EQ(tm2->currentCash(), doctest::Approx(1000000.0 - realized));
}

/** @par Test points: every money flow of a margin-accounted futures trade scales with the
 *  contract multiplier; the expected amounts are absolute on purpose */
TEST_CASE("test_TradeManager_margin_accounting_futures_multiplier") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());
    // The type-level unit of the fixture is the neutral 1, the case supplies the multiplier itself
    REQUIRE_EQ(rb.unit(), 1.0);

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    static const double MULTIPLIER = 10.0;  // rebar: 10 tons per lot
    ScopedMultiplier scoped(rb, MULTIPLIER);
    REQUIRE_EQ(rb.unit(), MULTIPLIER);

    double number = 10.0;
    price_t nominal = roundEx(close * number * MULTIPLIER, 2);
    price_t margin = roundEx(0.1 * nominal, 2);
    REQUIRE_UNARY(margin > 0.0);

    // The quantity grid below assumes the lot size divides the tested numbers and the cap stays
    // above the largest of them, so the fixture cannot silently change those expectations
    REQUIRE_UNARY(rb.minTradeNumber() > 0.0);
    REQUIRE_EQ(std::floor(number / rb.minTradeNumber()) * rb.minTradeNumber(), number);
    REQUIRE_UNARY(rb.maxTradeNumber() >= 10.0 * number);

    auto tm = crtMarginAccountTM(dt, 1000000.0);

    /** @arg The lock is one tenth of the multiplied notional */
    CHECK_EQ(tm->getOpenMarginToLock(dt, rb, close, number, MarginSide::LONG),
             doctest::Approx(margin));

    /** @arg The openable quantity is grid-measured on the margin of one lot, not of one share */
    CHECK_EQ(tm->getMaxOpenNumber(dt, rb, close, margin, MarginSide::LONG),
             doctest::Approx(number));
    CHECK_EQ(tm->getMaxOpenNumber(dt, rb, close, 10.0 * margin, MarginSide::LONG),
             doctest::Approx(10.0 * number));

    REQUIRE_EQ(tm->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business, BUSINESS_BUY);

    /** @arg The opening pays the margin of the multiplied notional and books it as the lock */
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 - margin));
    CHECK_EQ(tm->getPosition(dt, rb).margin, doctest::Approx(margin));
    CHECK_EQ(tm->getPosition(dt, rb).openBasis, doctest::Approx(nominal));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).occupied_margin, doctest::Approx(margin));
    CHECK_EQ(tm->getOccupiedMargin(dt), doctest::Approx(margin));
    CHECK_EQ(tm->getAvailableMargin(dt), doctest::Approx(1000000.0 - margin));

    /** @arg At the entry price the equity is still the whole capital, the lock being part of it */
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(1000000.0));

    /** @arg Closing half pays the half margin back plus the half notional difference */
    price_t up = close + 10.0;
    double half = number / 2.0;
    price_t released = roundEx(margin * 0.5, 2);
    price_t realized = roundEx((up - close) * half * MULTIPLIER, 2);
    REQUIRE_EQ(tm->sell(dt, rb, up, half, 0.0, 0.0, up, PART_SIGNAL).business, BUSINESS_SELL);
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 - margin + released + realized));
    CHECK_EQ(tm->getPosition(dt, rb).margin, doctest::Approx(margin - released));

    /** @arg Backfilling the same records rebuilds the identical lock, cash and equity */
    auto copy = crtMarginAccountTM(dt, 1000000.0);
    for (const auto& tr : tm->getTradeList()) {
        if (tr.business == BUSINESS_INIT || tr.business == BUSINESS_BORROW_CASH) {
            continue;
        }
        REQUIRE_UNARY(copy->addTradeRecord(tr));
    }
    CHECK_EQ(copy->currentCash(), doctest::Approx(tm->currentCash()));
    CHECK_EQ(copy->getPosition(dt, rb).margin, doctest::Approx(margin - released));
    CHECK_EQ(copy->getPosition(dt, rb).openBasis, doctest::Approx(nominal / 2.0));

    // The rest is closed and re-opened on the next trading day: the day-1 query of the batch view
    // then takes the trade-list replay branch (a same-day query falls to the live branch) with a
    // still-open holding, so the replay accumulator runs on the multiplied contract too
    KQuery next_days = KQueryByDate(dt, Datetime(200904100000), KQuery::DAY);
    KData next_bars = rb.getKData(next_days);
    REQUIRE_UNARY(next_bars.size() >= 2);
    Datetime dt2 = next_bars[1].datetime;
    REQUIRE_UNARY(dt2 > Datetime(dt.year(), dt.month(), dt.day(), 23, 59));
    price_t close2 = next_bars[1].closePrice;
    REQUIRE_UNARY(close2 > 0.0);
    price_t realized2 = roundEx((close2 - close) * half * MULTIPLIER, 2);

    /** @arg The full close returns the rest, so nothing of the lock stays behind */
    REQUIRE_EQ(tm->sell(dt2, rb, close2, half, 0.0, 0.0, close2, PART_SIGNAL).business,
               BUSINESS_SELL);
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 + realized + realized2));
    CHECK_EQ(tm->getFunds(dt2, KQuery::DAY).occupied_margin, doctest::Approx(0.0));

    price_t nominal2 = roundEx(close2 * number * MULTIPLIER, 2);
    price_t margin2 = roundEx(0.1 * nominal2, 2);
    REQUIRE_EQ(tm->buy(dt2, rb, close2, number, 0.0, 0.0, close2, PART_SIGNAL).business,
               BUSINESS_BUY);

    /** @arg The batch view replays the first day onto its half-closed holding */
    FundsRecordList replay = tm->getFundsList({dt, dt2});
    REQUIRE_EQ(replay.size(), 2u);
    CHECK_EQ(replay[0].occupied_margin, doctest::Approx(margin - released));
    CHECK_EQ(replay[0].net_assets(), doctest::Approx(1000000.0 + realized));

    /** @arg The latest date reports the live books, the new lock with the entry-price equity */
    CHECK_EQ(replay[1].occupied_margin, doctest::Approx(margin2));
    CHECK_EQ(replay[1].net_assets(), doctest::Approx(1000000.0 + realized + realized2));
}

/** @par Test points: the short side of a multiplied futures contract keeps the same money basis */
TEST_CASE("test_TradeManager_margin_accounting_futures_multiplier_short") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());
    REQUIRE_EQ(rb.unit(), 1.0);

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    static const double MULTIPLIER = 10.0;
    ScopedMultiplier scoped(rb, MULTIPLIER);
    REQUIRE_EQ(rb.unit(), MULTIPLIER);

    double number = 10.0;
    price_t nominal = roundEx(close * number * MULTIPLIER, 2);
    price_t margin = roundEx(0.1 * nominal, 2);
    REQUIRE_UNARY(margin > 0.0);

    auto tm = crtMarginAccountTM(dt, 1000000.0);
    REQUIRE_EQ(tm->sellShort(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_SELL_SHORT);

    /** @arg The short opening locks the multiplied margin and keeps the sale proceeds out */
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 - margin));
    CHECK_EQ(tm->getShortPosition(rb).margin, doctest::Approx(margin));
    CHECK_EQ(tm->getShortPosition(rb).openBasis, doctest::Approx(nominal));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).occupied_margin, doctest::Approx(margin));
    CHECK_EQ(tm->getOccupiedMargin(dt), doctest::Approx(margin));

    /** @arg At the entry price the short leaves the equity at the whole capital */
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(1000000.0));

    /** @arg Covering higher realizes the multiplied loss against the released margin */
    price_t up = close + 10.0;
    price_t loss = roundEx((up - close) * number * MULTIPLIER, 2);
    REQUIRE_EQ(tm->buyShort(dt, rb, up, number, 0.0, 0.0, up, PART_SIGNAL).business,
               BUSINESS_BUY_SHORT);
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 - loss));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).occupied_margin, doctest::Approx(0.0));

    /** @arg A partial cover releases the margin and keeps the rest of the lock */
    auto tm2 = crtMarginAccountTM(dt, 1000000.0);
    double half = number / 2.0;
    REQUIRE_EQ(tm2->sellShort(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_SELL_SHORT);
    price_t released = roundEx(margin * 0.5, 2);
    REQUIRE_EQ(tm2->buyShort(dt, rb, close, half, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY_SHORT);
    CHECK_EQ(tm2->currentCash(), doctest::Approx(1000000.0 - margin + released));
    CHECK_EQ(tm2->getShortPosition(rb).margin, doctest::Approx(margin - released));
}

/** @par Test points: the margin accounting warns when cash lots join a margin holding */
TEST_CASE("test_TradeManager_margin_accounting_mix_warn") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t close = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);

    auto tm = crtMarginAccountTM(dt, 1000000.0);
    REQUIRE_EQ(tm->buy(dt, stock, close, 100.0, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    CHECK_UNARY(tm->getPosition(dt, stock).margin > 0.0);

    /** @arg Switching the accounting off mid-flight: the cash lot joins without a new margin */
    tm->setParam<bool>("margin_accounting", false);
    REQUIRE_EQ(tm->buy(dt, stock, close, 100.0, 0.0, 0.0, close, PART_SIGNAL).business,
               BUSINESS_BUY);
    PositionRecord position = tm->getPosition(dt, stock);
    // The margin stays locked while the basis now averages both accountings (registered ①)
    CHECK_UNARY(position.margin > 0.0);
    CHECK_EQ(position.number, doctest::Approx(200.0));
}

/** @par Test points: a rejected short leaves no borrow debt or fee residue behind */
TEST_CASE("test_TradeManager_margin_accounting_reject_no_residue") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    double atom = stock.minTradeNumber();

    // A cash-basis full buy drains the cash while the net assets stay at the capital; the short
    // opening then must finance itself, and because the model lends securities out (A-share
    // semantics) without any borrowable pool the order is rejected before any debt is created
    price_t p = 10.0;
    double number = 100.0 * atom;
    price_t init_cash = p * number * stock.unit();
    auto tm = crtTM(dt, init_cash, TC_Zero(), "SYS", make_shared<MG_TestSecuritiesLending>());
    REQUIRE_EQ(tm->buy(dt, stock, p, number, 0.0, 0.0, p, PART_SIGNAL).business, BUSINESS_BUY);
    CHECK_EQ(tm->currentCash(), doctest::Approx(0.0));

    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<bool>("support_borrow_cash", true);
    // support_borrow_stock keeps its default false

    /** @arg The short is rejected because no securities were borrowed */
    auto trade = tm->sellShort(dt, stock, p, 10.0 * atom, 0.0, 0.0, p, PART_SIGNAL);
    CHECK_EQ(trade.business, BUSINESS_INVALID);

    /** @arg The rejection leaves no borrow debt, no fee and an unchanged cash balance */
    CHECK_EQ(tm->currentCash(), doctest::Approx(0.0));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).borrow_cash, doctest::Approx(0.0));
    size_t borrow_records = 0;
    for (const auto& tr : tm->getTradeList()) {
        if (tr.business == BUSINESS_BORROW_CASH) {
            borrow_records++;
        }
    }
    CHECK_EQ(borrow_records, 0u);
}

/** @par Test points: a partial close followed by a new opening must not dilute the cost pool */
TEST_CASE("test_TradeManager_margin_accounting_dilution") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt1(200001040000);
    Datetime dt2(200001050000);
    Datetime dt3(200001060000);
    Datetime dt4(200001070000);
    double atom = stock.minTradeNumber();

    auto tm = crtMarginAccountTM(dt1, 1000000.0);

    // buy 2*atom @10 -> sell atom @12 -> buy atom @11 -> sell 2*atom @12
    REQUIRE_EQ(tm->buy(dt1, stock, 10.0, 2.0 * atom, 0.0, 0.0, 10.0, PART_SIGNAL).business,
               BUSINESS_BUY);
    REQUIRE_EQ(tm->sell(dt2, stock, 12.0, atom, 0.0, 0.0, 12.0, PART_SIGNAL).business,
               BUSINESS_SELL);

    /** @arg After the partial close the remaining cost basis is only the unsold half */
    CHECK_EQ(tm->getPosition(dt2, stock).openBasis, doctest::Approx(10.0 * atom));
    FundsRecord live2 = tm->getFunds(dt2, KQuery::DAY);

    REQUIRE_EQ(tm->buy(dt3, stock, 11.0, atom, 0.0, 0.0, 11.0, PART_SIGNAL).business, BUSINESS_BUY);
    REQUIRE_EQ(tm->sell(dt4, stock, 12.0, 2.0 * atom, 0.0, 0.0, 12.0, PART_SIGNAL).business,
               BUSINESS_SELL);

    /** @arg The line closes flat and the cash grows by the true profit (36 - 31 = 5 per lot) */
    CHECK_EQ(tm->currentCash(), doctest::Approx(1000000.0 + 5.0 * atom));
    CHECK_EQ(tm->getFunds(dt4, KQuery::DAY).cash, doctest::Approx(1000000.0 + 5.0 * atom));

    /** @arg The fast replay of the partial date agrees with the live equity it showed then */
    FundsRecordList replay = tm->getFundsList({dt2});
    REQUIRE_EQ(replay.size(), 1u);
    CHECK_EQ(replay[0].net_assets(), doctest::Approx(live2.net_assets()));
    CHECK_EQ(replay[0].market_value, doctest::Approx(live2.market_value));
    CHECK_EQ(replay[0].occupied_margin, doctest::Approx(live2.occupied_margin));
}

/** @par Test points: a sub-unit margin requirement keeps the holding on the margin books */
TEST_CASE("test_TradeManager_margin_accounting_tiny_margin_lock") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    Datetime dt2(200001050000);
    price_t p = 10.0;
    double number = stock.minTradeNumber();

    // A rate of 1e-9 leaves a requirement of 1e-6: rounding it to the cent would drop the lock to
    // zero and demote the holding to the cash books, charging nothing while valuing the full
    // nominal amount
    auto tm = crtTM(dt, 100000.0, TC_Zero(), "SYS", MG_Fixed(1e-9));
    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<bool>("support_borrow_cash", true);
    REQUIRE_UNARY(tm->isMarginAccounted(stock));
    REQUIRE_EQ(tm->buy(dt, stock, p, number, 0.0, 0.0, p, PART_SIGNAL).business, BUSINESS_BUY);

    /** @arg The lock keeps one monetary unit instead of collapsing to zero */
    CHECK_EQ(tm->getPosition(dt, stock).margin, doctest::Approx(0.01));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).occupied_margin, doctest::Approx(0.01));

    /** @arg The valuation follows the margin books: the lock plus the unrealized profit only */
    price_t mv = stock.getMarketValue(dt, KQuery::DAY);
    price_t equity = 0.01 + (mv - p) * number * stock.unit();
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).market_value, doctest::Approx(equity));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(100000.0 - 0.01 + equity));

    // A second opening makes the earlier date replayable through the trade records
    REQUIRE_EQ(tm->buy(dt2, stock, p, number, 0.0, 0.0, p, PART_SIGNAL).business, BUSINESS_BUY);

    /** @arg The replay keeps the same floor, so the historical books agree with the live ones */
    FundsRecord replay = tm->getFunds(Datetime(200001042359), KQuery::DAY);
    CHECK_EQ(replay.occupied_margin, doctest::Approx(0.01));
    FundsRecordList fast = tm->getFundsList({Datetime(200001040000)});
    REQUIRE_EQ(fast.size(), 1u);
    CHECK_EQ(fast[0].occupied_margin, doctest::Approx(0.01));
}

/** @par Test points: a reimported margin opening reproduces the financing of the live books */
TEST_CASE("test_TradeManager_margin_accounting_reimport_financing") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!stock.isNull());
    REQUIRE_UNARY(!rb.isNull());

    Datetime dt(200903270000);
    price_t close = rb.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(close > 0.0);
    double unit = rb.unit();

    double number = 10.0 * rb.minTradeNumber();
    price_t margin = roundEx(0.5 * close * number * unit, 2);
    REQUIRE_UNARY(margin > 0.0);

    // The model covers the futures only, so the index holding stays on the cash books and can tie
    // up the cash a margin would need
    price_t idx_price = stock.getMarketValue(dt, KQuery::DAY);
    REQUIRE_UNARY(idx_price > 0.0);
    double idx_atom = stock.minTradeNumber();
    price_t atom_value = roundEx(idx_price * idx_atom * stock.unit(), 2);
    REQUIRE_UNARY(atom_value > 0.0);
    double idx_number = std::ceil(margin / atom_value) * idx_atom;
    price_t idx_notional = roundEx(idx_number * idx_price * stock.unit(), 2);
    REQUIRE_UNARY(idx_notional >= margin);

    // The capital pays the tied-up cash plus half a margin: the equity still covers a margin, the
    // cash on hand does not, so the futures opening has to finance the rest
    price_t init_cash = roundEx(idx_notional + 0.5 * margin, 2);

    auto tm = crtTM(dt, init_cash, TC_Zero(), "SYS", MG_ByType("12:0.5", 0.0, 1.0));
    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<bool>("support_borrow_cash", true);
    REQUIRE_UNARY(tm->isMarginAccounted(rb));
    REQUIRE_UNARY(!tm->isMarginAccounted(stock));

    REQUIRE_EQ(tm->buy(dt, stock, idx_price, idx_number, 0.0, 0.0, idx_price, PART_SIGNAL).business,
               BUSINESS_BUY);
    REQUIRE_UNARY(tm->currentCash() < margin);
    REQUIRE_UNARY(tm->getFunds(dt, KQuery::DAY).net_assets() >= margin);

    REQUIRE_EQ(tm->buy(dt, rb, close, number, 0.0, 0.0, close, PART_SIGNAL).business, BUSINESS_BUY);

    /** @arg The live books financed the shortfall instead of rejecting the opening */
    price_t live_cash = tm->currentCash();
    price_t live_debt = tm->getFunds(dt, KQuery::DAY).borrow_cash;
    price_t live_margin = tm->getPosition(dt, rb).margin;
    REQUIRE_UNARY(live_debt > 0.0);
    CHECK_EQ(live_margin, doctest::Approx(margin));

    // The exported records carry the trades the strategy placed, not the debt entries that funded
    // them, so the add path has to rebuild the financing itself; before this it rejected every
    // financed opening on import
    auto copy = crtTM(dt, init_cash, TC_Zero(), "COPY", MG_ByType("12:0.5", 0.0, 1.0));
    copy->setParam<bool>("margin_accounting", true);
    copy->setParam<bool>("support_borrow_cash", true);
    for (const auto& tr : tm->getTradeList()) {
        if (tr.business == BUSINESS_INIT || tr.business == BUSINESS_BORROW_CASH) {
            continue;
        }
        REQUIRE_UNARY(copy->addTradeRecord(tr));
    }

    /** @arg The rebuilt books agree on the cash, the debt, the lock and the equity */
    CHECK_EQ(copy->currentCash(), doctest::Approx(live_cash));
    CHECK_EQ(copy->getFunds(dt, KQuery::DAY).borrow_cash, doctest::Approx(live_debt));
    CHECK_EQ(copy->getPosition(dt, rb).margin, doctest::Approx(live_margin));
    CHECK_EQ(copy->getFunds(dt, KQuery::DAY).net_assets(),
             doctest::Approx(tm->getFunds(dt, KQuery::DAY).net_assets()));
}

/** @par Test points: a model reporting no requirement leaves the opening on the cash books */
TEST_CASE("test_TradeManager_margin_accounting_zero_requirement") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t p = 10.0;
    double number = 100.0 * stock.minTradeNumber();
    price_t notional = roundEx(p * number * stock.unit(), 2);

    auto tm = crtTM(dt, 100000.0, TC_Zero(), "SYS", make_shared<MG_TestZeroRequirement>());
    tm->setParam<bool>("margin_accounting", true);
    tm->setParam<bool>("support_borrow_cash", true);
    REQUIRE_UNARY(tm->isMarginAccounted(stock));

    /** @arg The requirement is zero, so the lock must not be floored into a phantom unit */
    CHECK_EQ(tm->getOpenMarginToLock(dt, stock, p, number, MarginSide::LONG), doctest::Approx(0.0));

    /** @arg The opening falls back to the cash books instead of becoming a free holding */
    REQUIRE_EQ(tm->buy(dt, stock, p, number, 0.0, 0.0, p, PART_SIGNAL).business, BUSINESS_BUY);
    CHECK_EQ(tm->currentCash(), doctest::Approx(100000.0 - notional));
    CHECK_EQ(tm->getPosition(dt, stock).margin, doctest::Approx(0.0));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).occupied_margin, doctest::Approx(0.0));

    /** @arg The equity still only counts what the cash paid plus the unrealized profit */
    price_t mv = stock.getMarketValue(dt, KQuery::DAY);
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(),
             doctest::Approx(100000.0 + (mv - p) * number * stock.unit()));
}

/** @par Test points: a margin holding refuses the deposits and withdrawals that detach the equity
 */
TEST_CASE("test_TradeManager_margin_accounting_deposit_withdraw") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    Datetime dt(200001040000);
    price_t p = 10.0;
    double atom = stock.minTradeNumber();

    auto tm = crtMarginAccountTM(dt, 1000000.0);
    REQUIRE_EQ(tm->buy(dt, stock, p, 10.0 * atom, 0.0, 0.0, p, PART_SIGNAL).business, BUSINESS_BUY);
    price_t cash = tm->currentCash();
    PositionRecord pos = tm->getPosition(dt, stock);
    REQUIRE_UNARY(pos.margin > 0.0);

    /** @arg A deposit is refused and leaves the quantity, the basis and the lock untouched */
    CHECK_UNARY(!tm->checkinStock(dt, stock, p, atom));
    PositionRecord after = tm->getPosition(dt, stock);
    CHECK_EQ(after.number, doctest::Approx(pos.number));
    CHECK_EQ(after.openBasis, doctest::Approx(pos.openBasis));
    CHECK_EQ(after.margin, doctest::Approx(pos.margin));
    CHECK_EQ(tm->currentCash(), doctest::Approx(cash));

    /** @arg A withdrawal is refused as well, so the lock can't drop along with the lots */
    CHECK_UNARY(!tm->checkoutStock(dt, stock, p, atom));
    CHECK_EQ(tm->getPosition(dt, stock).number, doctest::Approx(pos.number));
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).occupied_margin, doctest::Approx(pos.margin));

    /** @arg The backfill refuses the same records instead of drifting from the live books */
    CHECK_UNARY(!tm->addTradeRecord(TradeRecord(stock, dt, BUSINESS_CHECKIN_STOCK, p, p, 0.0, atom,
                                                CostRecord(), 0.0, cash, PART_INVALID)));
    CHECK_UNARY(!tm->addTradeRecord(TradeRecord(stock, dt, BUSINESS_CHECKOUT_STOCK, p, p, 0.0, atom,
                                                CostRecord(), 0.0, cash, PART_INVALID)));
    CHECK_EQ(tm->getPosition(dt, stock).number, doctest::Approx(pos.number));
    CHECK_EQ(tm->getPosition(dt, stock).margin, doctest::Approx(pos.margin));
}

/** @} */
