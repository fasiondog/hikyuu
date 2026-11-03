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
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_manage/crt/MG_Fixed.h>
#include <hikyuu/trade_manage/crt/MG_None.h>
#include <hikyuu/trade_manage/crt/MG_ByType.h>

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

/** @} */
