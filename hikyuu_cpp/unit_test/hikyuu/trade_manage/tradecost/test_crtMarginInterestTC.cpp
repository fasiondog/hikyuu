/*
 * test_crtMarginInterestTC.cpp
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/StockTypeInfo.h>
#include <hikyuu/config.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_Fixed.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_FixedA.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_MarginInterest.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_Zero.h>
#include <hikyuu/trade_manage/tradecost/imp/MarginInterestTradeCost.h>

#if HKU_SUPPORT_SERIALIZATION
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_crtMarginInterestTC test_crtMarginInterestTC
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

namespace {

/** A cost model answering a distinguishable record on every fee hook, and a rate of its own that
 *  the wrapper must not let through */
class InnerStubCost final : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    explicit InnerStubCost(bool python = false) : TradeCostBase("InnerStubCost") {
        m_is_python_object = python;
    }

    CostRecord getBuyCost(const Datetime&, const Stock&, price_t, double) const override {
        return fixedRecord(1.0);
    }

    CostRecord getSellCost(const Datetime&, const Stock&, price_t, double) const override {
        return fixedRecord(2.0);
    }

    CostRecord getBorrowCashCost(const Datetime&, price_t) const override {
        return fixedRecord(3.0);
    }

    CostRecord getReturnCashCost(const Datetime&, const Datetime&, price_t) const override {
        return fixedRecord(4.0);
    }

    CostRecord getBorrowStockCost(const Datetime&, const Stock&, price_t, double) const override {
        return fixedRecord(5.0);
    }

    CostRecord getReturnStockCost(const Datetime&, const Datetime&, const Stock&, price_t,
                                  double) const override {
        return fixedRecord(6.0);
    }

    double getBorrowCashRate(const Datetime&) const override {
        return 0.5;
    }

    double getBorrowStockRate(const Datetime&, const Stock&) const override {
        return 0.5;
    }

    TradeCostPtr _clone() override {
        return make_shared<InnerStubCost>();
    }

private:
    static CostRecord fixedRecord(price_t value) {
        CostRecord record;
        record.commission = value;
        record.total = value;
        return record;
    }
};

/* The fixture stocks carry only the market and the type, which is all the tables match on */
Stock mkStock(const string& market, uint32_t type) {
    return Stock(market, "000001", "fixture", type, true, Datetime(199001010000), Null<Datetime>(),
                 0.01, 0.01, 2, 100, 1000000);
}

}  // namespace

/** @par Test points: every trading fee is answered by the wrapped model, while the rates are not
 *  taken from it */
TEST_CASE("test_TC_MarginInterest_delegate") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    const Datetime dt(201001010000);

    auto inner = make_shared<InnerStubCost>();
    TradeCostPtr cost = TC_MarginInterest(inner);

    /** @arg The name and the wrapped model itself stay reachable */
    CHECK_EQ(cost->name(), "TC_MarginInterest");
    CHECK_EQ(std::static_pointer_cast<MarginInterestTradeCost>(cost)->getInnerCost()->name(),
             "InnerStubCost");

    /** @arg All six fee hooks are delegated as they are */
    CHECK_EQ(cost->getBuyCost(dt, sh, 10.0, 1000), inner->getBuyCost(dt, sh, 10.0, 1000));
    CHECK_EQ(cost->getSellCost(dt, sh, 10.0, 1000), inner->getSellCost(dt, sh, 10.0, 1000));
    CHECK_EQ(cost->getBorrowCashCost(dt, 10000.0), inner->getBorrowCashCost(dt, 10000.0));
    CHECK_EQ(cost->getReturnCashCost(dt, dt, 10000.0), inner->getReturnCashCost(dt, dt, 10000.0));
    CHECK_EQ(cost->getBorrowStockCost(dt, sh, 10.0, 1000),
             inner->getBorrowStockCost(dt, sh, 10.0, 1000));
    CHECK_EQ(cost->getReturnStockCost(dt, dt, sh, 10.0, 1000),
             inner->getReturnStockCost(dt, dt, sh, 10.0, 1000));
    CHECK_EQ(cost->getBuyCost(dt, sh, 10.0, 1000).total, doctest::Approx(1.0));

    /** @arg The rates belong to the wrapper, so the wrapped model cannot bill interest behind its
     *  back while nothing has been configured */
    CHECK_EQ(cost->getBorrowCashRate(dt), 0.0);
    CHECK_EQ(cost->getBorrowStockRate(dt, sh), 0.0);

    /** @arg Wrapping nothing keeps the zero cost accounting */
    TradeCostPtr bare = TC_MarginInterest();
    CHECK_EQ(bare->getBuyCost(dt, sh, 10.0, 1000).total, 0.0);
    CHECK_EQ(bare->getSellCost(dt, sh, 10.0, 1000).total, 0.0);
    CHECK_EQ(std::static_pointer_cast<MarginInterestTradeCost>(bare)->getInnerCost()->name(),
             "TC_Zero");
}

/** @par Test points: both rates are scalars of the [0, 1) interval, 0.0 billing nothing */
TEST_CASE("test_TC_MarginInterest_cash_rate") {
    const Datetime dt(201001010000);

    /** @arg The default answers nothing, and a configured rate is answered as is */
    TradeCostPtr cost = TC_MarginInterest(TC_FixedA(), 0.06);
    CHECK_EQ(cost->getBorrowCashRate(dt), doctest::Approx(0.06));
    CHECK_EQ(TC_MarginInterest()->getBorrowCashRate(dt), 0.0);

    /** @arg Both ends of the valid interval: 0.0 is accepted, 1.0 is not */
    CHECK_NOTHROW(cost->setParam<double>("borrow_cash_rate", 0.0));
    CHECK_THROWS(cost->setParam<double>("borrow_cash_rate", 1.0));
    CHECK_THROWS(cost->setParam<double>("borrow_cash_rate", -0.01));
    CHECK_THROWS(cost->setParam<double>("borrow_cash_rate", std::nan("")));

    /** @arg The lending fallback rate is guarded by the same interval, and a security matched by no
     *  key takes it */
    CHECK_NOTHROW(cost->setParam<double>("default_borrow_stock_rate", 0.1));
    CHECK_EQ(cost->getBorrowStockRate(dt, mkStock("NASDAQ", STOCKTYPE_A)), doctest::Approx(0.1));
    CHECK_THROWS(cost->setParam<double>("default_borrow_stock_rate", 1.0));
    CHECK_THROWS(cost->setParam<double>("default_borrow_stock_rate", -0.1));
    CHECK_THROWS(cost->setParam<double>("default_borrow_stock_rate", std::nan("")));
}

/** @par Test points: the lending rates come from the type and market table, falling back to the
 *  configured default */
TEST_CASE("test_TC_MarginInterest_stock_rate_table") {
    const Datetime dt(201001010000);
    Stock fut = mkStock("SHF", 12);
    Stock a = mkStock("SH", STOCKTYPE_A);
    Stock us = mkStock("NASDAQ", STOCKTYPE_A);

    /** @arg A type key hits before a market key, and an unmatched security takes the default */
    auto cost = TC_MarginInterest(TC_Zero(), 0.0, "12:0.10, SH:0.05", 0.02);
    CHECK_EQ(cost->getBorrowStockRate(dt, fut), doctest::Approx(0.10));
    CHECK_EQ(cost->getBorrowStockRate(dt, a), doctest::Approx(0.05));
    CHECK_EQ(cost->getBorrowStockRate(dt, us), doctest::Approx(0.02));

    /** @arg Unlike the margin ratios, a rate of 0 is valid and bills nothing */
    auto zero = TC_MarginInterest(TC_Zero(), 0.0, "SHF:0");
    CHECK_EQ(zero->getBorrowStockRate(dt, fut), 0.0);

    /** @arg An out of range or malformed ratio is rejected at once */
    CHECK_THROWS(TC_MarginInterest(TC_Zero(), 0.0, "SHF:1.2"));
    CHECK_THROWS(TC_MarginInterest(TC_Zero(), 0.0, "SHF:abc"));
    CHECK_THROWS(TC_MarginInterest(TC_Zero(), 0.0, "SHF"));

    /** @arg A rejected expression is refused by the param check, which is the only place parsing
     *  happens, so the table stays as it was */
    CHECK_THROWS(cost->setParam<string>("borrow_stock_rates", "12:0.3, bad"));
    CHECK_EQ(cost->getBorrowStockRate(dt, fut), doctest::Approx(0.10));

    /** @arg Restoring the very same expression is a no-op that keeps the table */
    cost->setParam<string>("borrow_stock_rates", "12:0.10, SH:0.05");
    CHECK_EQ(cost->getBorrowStockRate(dt, fut), doctest::Approx(0.10));
}

/** @par Test points: clone keeps both the parameters and the wrapped model */
TEST_CASE("test_TC_MarginInterest_clone") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    const Datetime dt(201001010000);

    auto inner = make_shared<InnerStubCost>();
    auto cost = TC_MarginInterest(inner, 0.06, "SH:0.1");
    auto copied = cost->clone();

    /** @arg The rates and the table survive the clone */
    CHECK_EQ(copied->name(), "TC_MarginInterest");
    CHECK_EQ(copied->getBorrowCashRate(dt), doctest::Approx(0.06));
    CHECK_EQ(copied->getBorrowStockRate(dt, sh), doctest::Approx(0.1));

    /** @arg The fees still come from a model of the same kind, not from a zero one */
    CHECK_EQ(copied->getBuyCost(dt, sh, 10.0, 1000).total, doctest::Approx(1.0));
}

/** @par Test points: the Python flag of the wrapped model is reported through the wrapper, so that
 *  a run never releases the GIL while a Python cost model is still being called */
TEST_CASE("test_TC_MarginInterest_python_part") {
    /** @arg The built-in models on both sides keep the account pickable for a parallel run */
    CHECK_UNARY(!TC_MarginInterest(TC_FixedA())->hasPythonPart());

    /** @arg A Python defined inner model is reported upward, and a clone carries the flag too */
    auto wrapped = TC_MarginInterest(make_shared<InnerStubCost>(true));
    CHECK_UNARY(wrapped->hasPythonPart());
    CHECK_UNARY(wrapped->clone()->hasPythonPart());

    /** @arg The flag belongs to the model, not to the wrapping: an inner built-in one stays off */
    CHECK_UNARY(!TC_MarginInterest(make_shared<InnerStubCost>())->hasPythonPart());
}

#if HKU_SUPPORT_SERIALIZATION
/** @par Test points: the serialization round trip keeps the wrapped model and answers the rates
 *  again even though the table itself is not stored */
TEST_CASE("test_TC_MarginInterest_export") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    const Datetime dt(201001010000);
    string filename(sm.tmpdir());
    filename += "/TC_MarginInterest.xml";

    TradeCostPtr cost = TC_MarginInterest(TC_Fixed(0.0003, 5.0), 0.06, "SH:0.1");
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(cost);
    }

    TradeCostPtr cost2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(cost2);
    }

    /** @arg The loaded model keeps the fees of the wrapped one, and loading re-runs the param
     *  check, which rebuilds the table that has not been stored */
    CHECK_EQ(cost2->name(), "TC_MarginInterest");
    CHECK_EQ(cost2->getBorrowCashRate(dt), doctest::Approx(0.06));
    CHECK_EQ(cost2->getBorrowStockRate(dt, sh), doctest::Approx(0.1));
    CHECK_EQ(cost2->getBuyCost(dt, sh, 10.0, 1000), cost->getBuyCost(dt, sh, 10.0, 1000));
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
