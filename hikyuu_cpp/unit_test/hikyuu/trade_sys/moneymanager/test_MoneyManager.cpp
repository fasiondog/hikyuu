/*
 * test_MoneyManager.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/data_driver/DataDriverFactory.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_sys/moneymanager/MoneyManagerBase.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCapital.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCountTps.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>

using namespace hku;

class MoneyManagerTest : public MoneyManagerBase {
public:
    MoneyManagerTest() : MoneyManagerBase("MoneyManagerTest") {
        m_x = 0;
    }
    virtual ~MoneyManagerTest() {}

    TradeManagerPtr getTM() const {
        return m_tm;
    }
    const KQuery &getQuery() const {
        return m_query;
    }
    int getX() const {
        return m_x;
    }
    void setX(int x) {
        m_x = x;
    }

    virtual double _getBuyNumber(const Datetime &datetime, const Stock &stock, price_t price,
                                 price_t risk, SystemPart from) {
        return 0;
    }

    virtual void _reset() {
        m_x = 0;
    }

    virtual MoneyManagerPtr _clone() {
        MoneyManagerTest *p = new MoneyManagerTest;
        p->m_x = m_x;
        return MoneyManagerPtr(p);
    }

private:
    int m_x;
};

class MultSellMoneyManagerTest : public MoneyManagerBase {
public:
    MultSellMoneyManagerTest(bool support_mult) : MoneyManagerBase("MultSellMoneyManagerTest") {
        m_support_mult_buy_sell = support_mult;
    }
    virtual ~MultSellMoneyManagerTest() {}

    virtual double _getBuyNumber(const Datetime &datetime, const Stock &stock, price_t price,
                                 price_t risk, SystemPart from) {
        return 0;
    }

    // Return a fixed value to distinguish subclass sizing from the base guard
    virtual double _getSellNumber(const Datetime &datetime, const Stock &stock, price_t price,
                                  price_t risk, SystemPart from) {
        return 100.0;
    }

    // Return a fixed value distinguishable from the base default (0) so that the input guards of
    // getSellShortNumber can be pinned
    virtual double _getSellShortNumber(const Datetime &datetime, const Stock &stock, price_t price,
                                       price_t risk, SystemPart from) {
        return 200.0;
    }

    // Same as above: distinguishable from the base default (MAX_DOUBLE) so that the NaN risk
    // guard of getBuyShortNumber is a strong pin
    virtual double _getBuyShortNumber(const Datetime &datetime, const Stock &stock, price_t price,
                                      price_t risk, SystemPart from) {
        return 300.0;
    }

    virtual MoneyManagerPtr _clone() {
        return MoneyManagerPtr(new MultSellMoneyManagerTest(m_support_mult_buy_sell));
    }
};

class ThrowCloneMoneyManagerTest : public MoneyManagerBase {
public:
    ThrowCloneMoneyManagerTest() : MoneyManagerBase("ThrowCloneMoneyManagerTest") {}
    virtual ~ThrowCloneMoneyManagerTest() {}

    virtual double _getBuyNumber(const Datetime &datetime, const Stock &stock, price_t price,
                                 price_t risk, SystemPart from) {
        return 0;
    }

    virtual MoneyManagerPtr _clone() {
        throw std::runtime_error("test clone failure");
    }
};

class SelfCloneMoneyManagerTest : public MoneyManagerBase,
                                  public std::enable_shared_from_this<SelfCloneMoneyManagerTest> {
public:
    SelfCloneMoneyManagerTest() : MoneyManagerBase("SelfCloneMoneyManagerTest") {}
    virtual ~SelfCloneMoneyManagerTest() {}

    virtual double _getBuyNumber(const Datetime &datetime, const Stock &stock, price_t price,
                                 price_t risk, SystemPart from) {
        return 0;
    }

    virtual MoneyManagerPtr _clone() {
        return shared_from_this();
    }
};

class FixedBuyNumberMMTest : public MoneyManagerBase {
public:
    FixedBuyNumberMMTest() : MoneyManagerBase("FixedBuyNumberMMTest"), m_number(100.0) {}
    virtual ~FixedBuyNumberMMTest() {}

    virtual double _getBuyNumber(const Datetime &datetime, const Stock &stock, price_t price,
                                 price_t risk, SystemPart from) {
        return m_number;
    }

    virtual MoneyManagerPtr _clone() {
        return MoneyManagerPtr(new FixedBuyNumberMMTest);
    }

private:
    double m_number;
};

class ConfigurableBuyNumberMMTest : public MoneyManagerBase {
public:
    ConfigurableBuyNumberMMTest(double number)
    : MoneyManagerBase("ConfigurableBuyNumberMMTest"), m_number(number) {}
    virtual ~ConfigurableBuyNumberMMTest() {}

    virtual double _getBuyNumber(const Datetime &datetime, const Stock &stock, price_t price,
                                 price_t risk, SystemPart from) {
        return m_number;
    }

    virtual MoneyManagerPtr _clone() {
        return MoneyManagerPtr(new ConfigurableBuyNumberMMTest(m_number));
    }

private:
    double m_number;
};

/**
 * @defgroup test_MoneyManager test_MoneyManager
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MoneyManager") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    TradeManagerPtr tm = crtTM();

    /** @arg The basic operation */
    MoneyManagerPtr p(new MoneyManagerTest);
    MoneyManagerTest *p_src = (MoneyManagerTest *)p.get();
    CHECK_EQ(p->name(), "MoneyManagerTest");
    CHECK_EQ(p_src->getTM(), TradeManagerPtr());
    p->setTM(tm);
    CHECK_EQ(p_src->getTM(), tm);
    CHECK_EQ(p->getBuyNumber(Datetime(200001010000), stock, 10.0, 10.0, PART_SIGNAL), 0);
    CHECK_UNARY(
      (p->getSellNumber(Datetime(200001010000), stock, 10.0, 10.0, PART_SIGNAL) == MAX_DOUBLE));
    CHECK_EQ(p_src->getX(), 0);
    p_src->setX(10);
    CHECK_EQ(p_src->getX(), 10);
    p->reset();
    CHECK_EQ(p_src->getX(), 0);

    /** @arg The clone operation */
    p_src->setX(10);
    MoneyManagerPtr p_clone = p->clone();
    CHECK_NE(p, p_clone);
    p_src = (MoneyManagerTest *)p_clone.get();
    CHECK_EQ(p->name(), "MoneyManagerTest");
    // CHECK_EQ(p_src->getTM() == tm);
    CHECK_EQ(p_src->getX(), 10);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_getSellNumber_risk_guard") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm["sh000001"];

    /** @arg MM which does not support multi-trading: risk <= 0 liquidates the whole position */
    MoneyManagerPtr p(new MultSellMoneyManagerTest(false));
    p->setTM(crtTM());
    CHECK_UNARY(
      (p->getSellNumber(Datetime(200001010000), stock, 10.0, 0.0, PART_SIGNAL) == MAX_DOUBLE));
    CHECK_UNARY(
      (p->getSellNumber(Datetime(200001010000), stock, 10.0, -1.0, PART_SIGNAL) == MAX_DOUBLE));

    /** @arg A NaN risk is treated as breached (the guard uses !(risk > 0.0) for NaN-safety) */
    double nan = std::numeric_limits<double>::quiet_NaN();
    CHECK_UNARY(
      (p->getSellNumber(Datetime(200001010000), stock, 10.0, nan, PART_SIGNAL) == MAX_DOUBLE));

    /** @arg The sibling entries share the same NaN-safe guard: getSellShortNumber rejects to 0
     *  and getBuyShortNumber liquidates the whole short position (MAX_DOUBLE) */
    CHECK_EQ(p->getSellShortNumber(Datetime(200001010000), stock, 10.0, nan, PART_SIGNAL), 0.0);
    CHECK_UNARY(
      (p->getBuyShortNumber(Datetime(200001010000), stock, 10.0, nan, PART_SIGNAL) == MAX_DOUBLE));

    /** @arg MM which does not support multi-trading: risk > 0 is sized by the subclass */
    CHECK_EQ(p->getSellNumber(Datetime(200001010000), stock, 10.0, 10.0, PART_SIGNAL), 100.0);

    /** @arg MM which supports multi-trading: risk <= 0 is still delegated to the subclass */
    MoneyManagerPtr p2(new MultSellMoneyManagerTest(true));
    p2->setTM(crtTM());
    CHECK_EQ(p2->getSellNumber(Datetime(200001010000), stock, 10.0, 0.0, PART_SIGNAL), 100.0);
    CHECK_EQ(p2->getSellNumber(Datetime(200001010000), stock, 10.0, -1.0, PART_SIGNAL), 100.0);
    CHECK_EQ(p2->getSellNumber(Datetime(200001010000), stock, 10.0, 10.0, PART_SIGNAL), 100.0);

    /** @arg The capability flag is rebuilt by the subclass constructor after cloning */
    MoneyManagerPtr p3 = p2->clone();
    CHECK_UNARY(
      (p3->getSellNumber(Datetime(200001010000), stock, 10.0, 0.0, PART_SIGNAL) == 100.0));
}

/** @par Test points */
TEST_CASE("test_MoneyManager_clone_fail_fast") {
    /** @arg The subclass _clone throws: the base clone throws instead of the self-ptr fallback
     * (ISS-028) */
    MoneyManagerPtr p(new ThrowCloneMoneyManagerTest);
    CHECK_THROWS_AS(p->clone(), std::exception);

    /** @arg The subclass _clone returns self: the base clone throws instead of the self-ptr
     * fallback (ISS-028) */
    MoneyManagerPtr p2(new SelfCloneMoneyManagerTest);
    CHECK_THROWS_AS(p2->clone(), std::exception);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_getBuyNumber_unit_cash_estimate") {
    // tickValue / tick = 2.0 makes stock.unit() != 1, the TM cost func is TC_Zero (cost.total == 0)
    Stock unit2_stock("TEST", "MMUNIT", "MM Unit Test", 1, true, Datetime(199001010000),
                      Datetime(209901010000), 1.0, 2.0, 2, 1, 100000);
    Parameter driver_param;
    driver_param.set<string>("type", "DoNothing");
    unit2_stock.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    REQUIRE_UNARY(!unit2_stock.isNull());
    CHECK_EQ(unit2_stock.unit(), 2.0);

    StockManager &sm = StockManager::instance();
    Stock unit1_stock = sm["sh000001"];
    REQUIRE_UNARY(!unit1_stock.isNull());

    /** @arg unit == 1: the estimate keeps the original behavior (cash 1000 exactly affords
     * 100 shares at price 10) */
    MoneyManagerPtr mm1(new FixedBuyNumberMMTest);
    mm1->setTM(crtTM(Datetime(200001010000), 1000.0));
    CHECK_EQ(mm1->getBuyNumber(Datetime(200001010000), unit1_stock, 10.0, 1.0, PART_SIGNAL), 100.0);

    /** @arg unit == 2: the estimate multiplies by stock.unit(), reducing the number to the
     * affordable 50 (regression for the missing unit, ISS-092) */
    MoneyManagerPtr mm2(new FixedBuyNumberMMTest);
    mm2->setTM(crtTM(Datetime(200001010000), 1000.0));
    CHECK_EQ(mm2->getBuyNumber(Datetime(200001010000), unit2_stock, 10.0, 1.0, PART_SIGNAL), 50.0);

    /** @arg unit == 2: even one minimum trade quantity is unaffordable, returns 0 */
    MoneyManagerPtr mm3(new FixedBuyNumberMMTest);
    mm3->setTM(crtTM(Datetime(200001010000), 15.0));
    CHECK_EQ(mm3->getBuyNumber(Datetime(200001010000), unit2_stock, 10.0, 1.0, PART_SIGNAL), 0.0);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_getBuyNumber_cash_shortage_binary_search") {
    // crtTM defaults to TC_Zero (cost.total == 0), so need_cash == n * price * unit;
    // sh000001 has minTradeNumber == 1, so the affordable lot grid here is per-share
    StockManager &sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg cash affords only part of the requested size: reduced to the exact largest
     * affordable quantity (5500 == 55000 / 10.0) */
    MoneyManagerPtr mm1(new ConfigurableBuyNumberMMTest(10000.0));
    mm1->setTM(crtTM(Datetime(200001010000), 55000.0));
    CHECK_EQ(mm1->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 5500.0);

    /** @arg cash exactly affords the reduction target: that quantity is kept
     * (inclusive boundary) */
    MoneyManagerPtr mm2(new ConfigurableBuyNumberMMTest(10000.0));
    mm2->setTM(crtTM(Datetime(200001010000), 50000.0));
    CHECK_EQ(mm2->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 5000.0);

    /** @arg cash exactly affords the full requested size: returned unchanged, the search
     * is skipped */
    MoneyManagerPtr mm3(new ConfigurableBuyNumberMMTest(10000.0));
    mm3->setTM(crtTM(Datetime(200001010000), 100000.0));
    CHECK_EQ(mm3->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 10000.0);

    /** @arg even one share is unaffordable: the search floors the quantity at min_trade
     * and returns 0 (9 < 10, one share at price 10) */
    MoneyManagerPtr mm4(new ConfigurableBuyNumberMMTest(10000.0));
    mm4->setTM(crtTM(Datetime(200001010000), 9.0));
    CHECK_EQ(mm4->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 0.0);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_getBuyNumber_auto_checkin_no_side_effect") {
    // The auto_checkin top-up is done by System at the actual trade: the query stays side-effect
    // free and the number is not constrained by the current cash
    StockManager &sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());

    /** @arg auto_checkin=true: the query neither deposits nor reduces the number (cash 55000
     * affords only 5500 shares) */
    MoneyManagerPtr mm(new ConfigurableBuyNumberMMTest(10000.0));
    TradeManagerPtr tm = crtTM(Datetime(200001010000), 55000.0);
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", true);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 10000.0);
    CHECK_EQ(tm->currentCash(), 55000.0);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 10000.0);
    CHECK_EQ(tm->currentCash(), 55000.0);
    for (const auto &tr : tm->getTradeList()) {
        CHECK_NE(tr.business, BUSINESS_CHECKIN);
    }
}

/** @par Test points */
TEST_CASE("test_MoneyManager_getBuyNumber_risk_guard") {
    // tick == tickValue keeps stock.unit() at 1; minTradeNumber == 100 matches the fixed size
    Stock stock("TEST", "MMRISK", "MM Risk Guard", 1, true, Datetime(199001010000),
                Datetime(209901010000), 1.0, 1.0, 2, 100, 100000);
    Parameter driver_param;
    driver_param.set<string>("type", "DoNothing");
    stock.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    REQUIRE_UNARY(!stock.isNull());

    constexpr double nan{std::numeric_limits<double>::quiet_NaN()};
    constexpr double inf{std::numeric_limits<double>::infinity()};

    // A non-zero result can only come from the subclass, so 0.0 proves the guard rejected it
    MoneyManagerPtr mm(new ConfigurableBuyNumberMMTest(100.0));
    mm->setTM(crtTM(Datetime(200001010000), 100000.0));
    Datetime dt(200001010000);

    /** @arg risk > 0: the size comes from the subclass */
    CHECK_EQ(mm->getBuyNumber(dt, stock, 10.0, 1.0, PART_SIGNAL), 100.0);

    /** @arg risk == 0 and risk < 0: rejected before the subclass */
    CHECK_EQ(mm->getBuyNumber(dt, stock, 10.0, 0.0, PART_SIGNAL), 0.0);
    CHECK_EQ(mm->getBuyNumber(dt, stock, 10.0, -1.0, PART_SIGNAL), 0.0);

    /** @arg A NaN risk fails the "risk <= 0.0" test and is rejected by !(risk > 0.0) */
    CHECK_EQ(mm->getBuyNumber(dt, stock, 10.0, nan, PART_SIGNAL), 0.0);

    /** @arg An infinite risk is a positive risk: not rejected by the guard */
    CHECK_EQ(mm->getBuyNumber(dt, stock, 10.0, inf, PART_SIGNAL), 100.0);

    /** @arg A NaN price is rejected by the price guard */
    CHECK_EQ(mm->getBuyNumber(dt, stock, nan, 1.0, PART_SIGNAL), 0.0);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_getBuyNumber_nonfinite_number") {
    Stock stock("TEST", "MMNUMBER", "MM Number Guard", 1, true, Datetime(199001010000),
                Datetime(209901010000), 1.0, 1.0, 2, 1, 1000000);
    Parameter driver_param;
    driver_param.set<string>("type", "DoNothing");
    stock.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    REQUIRE_UNARY(!stock.isNull());

    // minTradeNumber == 1 makes the lot quotient equal to the raw number, so the int64_t cast
    // boundary of 9.0e18 is exercised exactly
    constexpr double nan{std::numeric_limits<double>::quiet_NaN()};
    constexpr double inf{std::numeric_limits<double>::infinity()};
    constexpr double cast_limit{9.0e18};

    auto buyNumber = [&](double number) {
        MoneyManagerPtr mm(new ConfigurableBuyNumberMMTest(number));
        mm->setTM(crtTM(Datetime(200001010000), 100000.0));
        return mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL);
    };

    /** @arg A NaN size is rejected before the int64_t cast (undefined behaviour before) */
    CHECK_EQ(buyNumber(nan), 0.0);

    /** @arg A +/-inf size is rejected */
    CHECK_EQ(buyNumber(inf), 0.0);
    CHECK_EQ(buyNumber(-inf), 0.0);

    /** @arg A finite size out of the int64_t range is rejected */
    CHECK_EQ(buyNumber(1.0e30), 0.0);

    /** @arg The boundary itself is accepted: the size falls through to the maxTradeNumber clamp */
    CHECK_EQ(buyNumber(cast_limit), stock.maxTradeNumber());

    /** @arg One representable double above the boundary is rejected */
    CHECK_EQ(buyNumber(std::nextafter(cast_limit, inf)), 0.0);

    /** @arg Zero and a negative finite size stay below minTradeNumber and are ignored */
    CHECK_EQ(buyNumber(0.0), 0.0);
    CHECK_EQ(buyNumber(-100.0), 0.0);

    /** @arg A dirty minTradeNumber == 0 makes the quotient +inf, rejected before the cast */
    Stock zero_stock("TEST", "MMZERO", "MM Zero Min Trade", 1, true, Datetime(199001010000),
                     Datetime(209901010000), 1.0, 1.0, 2, 0, 1000000);
    zero_stock.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    REQUIRE_UNARY(!zero_stock.isNull());
    MoneyManagerPtr mm(new ConfigurableBuyNumberMMTest(100.0));
    mm->setTM(crtTM(Datetime(200001010000), 100000.0));
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), zero_stock, 10.0, 1.0, PART_SIGNAL), 0.0);

    /** @arg A negative minTradeNumber makes the quotient negative, rejected before the cast */
    Stock neg_min_stock("TEST", "MMNEGMIN", "MM Negative Min Trade", 1, true,
                        Datetime(199001010000), Datetime(209901010000), 1.0, 1.0, 2, 1, 1000000);
    neg_min_stock.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    neg_min_stock.minTradeNumber(-100.0);
    MoneyManagerPtr mm2(new ConfigurableBuyNumberMMTest(100.0));
    mm2->setTM(crtTM(Datetime(200001010000), 100000.0));
    CHECK_EQ(mm2->getBuyNumber(Datetime(200001010000), neg_min_stock, 10.0, 1.0, PART_SIGNAL), 0.0);

    /** @arg A negative maxTradeNumber is rejected in the clamp instead of casting a negative
     * out-of-range quotient */
    Stock neg_max_stock("TEST", "MMNEGMAX", "MM Negative Max Trade", 1, true,
                        Datetime(199001010000), Datetime(209901010000), 1.0, 1.0, 2, 1, 1000000);
    neg_max_stock.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    neg_max_stock.maxTradeNumber(-100.0);
    MoneyManagerPtr mm3(new ConfigurableBuyNumberMMTest(100.0));
    mm3->setTM(crtTM(Datetime(200001010000), 100000.0));
    CHECK_EQ(mm3->getBuyNumber(Datetime(200001010000), neg_max_stock, 10.0, 1.0, PART_SIGNAL), 0.0);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_getBuyNumber_max_trade_clamp") {
    // minTradeNumber == 100 with a maxTradeNumber of 550 (not a whole number of lots)
    Stock stock("TEST", "MMODDMAX", "MM Odd Max Trade", 1, true, Datetime(199001010000),
                Datetime(209901010000), 1.0, 1.0, 2, 100, 550);
    Parameter driver_param;
    driver_param.set<string>("type", "DoNothing");
    stock.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    REQUIRE_UNARY(!stock.isNull());

    /** @arg The clamp returns stock.maxTradeNumber directly (no lot-floor adjustment) */
    MoneyManagerPtr mm(new ConfigurableBuyNumberMMTest(1000.0));
    mm->setTM(crtTM(Datetime(200001010000), 1000000.0));
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 550.0);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_sell_entries_input_guard") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    REQUIRE_UNARY(!stock.isNull());
    constexpr double nan{std::numeric_limits<double>::quiet_NaN()};
    Datetime dt(200001010000);

    /** @arg getSellNumber rejects a null stock and a non-positive price like getBuyNumber does */
    MoneyManagerPtr mm(new MultSellMoneyManagerTest(false));
    mm->setTM(crtTM());
    CHECK_EQ(mm->getSellNumber(dt, Stock(), 10.0, 10.0, PART_SIGNAL), 0.0);
    CHECK_EQ(mm->getSellNumber(dt, stock, nan, 10.0, PART_SIGNAL), 0.0);
    CHECK_EQ(mm->getSellNumber(dt, stock, 0.0, 10.0, PART_SIGNAL), 0.0);
    CHECK_EQ(mm->getSellNumber(dt, stock, 10.0, 10.0, PART_SIGNAL), 100.0);

    /** @arg getSellShortNumber applies the same guards before reaching the subclass */
    CHECK_EQ(mm->getSellShortNumber(dt, Stock(), 10.0, 10.0, PART_SIGNAL), 0.0);
    CHECK_EQ(mm->getSellShortNumber(dt, stock, nan, 10.0, PART_SIGNAL), 0.0);
    CHECK_EQ(mm->getSellShortNumber(dt, stock, 10.0, 10.0, PART_SIGNAL), 200.0);

    /** @arg getBuyShortNumber applies the same guards before the force-cover branches */
    CHECK_EQ(mm->getBuyShortNumber(dt, Stock(), 10.0, 10.0, PART_SIGNAL), 0.0);
    CHECK_EQ(mm->getBuyShortNumber(dt, stock, nan, 10.0, PART_SIGNAL), 0.0);
    CHECK_EQ(mm->getBuyShortNumber(dt, stock, 10.0, 10.0, PART_SIGNAL), 300.0);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_clone_and_reset_state") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    auto tm = crtTM(Datetime(200001010000), 100000.0);

    /** @arg A clone carries over the runtime tranche counters from the source instance */
    MoneyManagerPtr mm(new MoneyManagerTest);
    mm->setTM(tm);
    auto tr = tm->buy(Datetime(200001010000), stock, 10.0, 100, 0, 0, 10.0, PART_SIGNAL);
    REQUIRE_UNARY(!tr.isNull());
    mm->buyNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 1);

    MoneyManagerPtr cloned = mm->clone();
    CHECK_EQ(cloned->currentBuyCount(stock), 1);
    CHECK_EQ(mm->currentBuyCount(stock), 1);

    /** @arg reset leaves a valid day query instead of a null one, so the cash lookup of a
     * standalone MM keeps a consistent ktype */
    MoneyManagerPtr p(new MoneyManagerTest);
    MoneyManagerTest *raw = (MoneyManagerTest *)p.get();
    p->setTM(tm);
    p->setQuery(KQuery(-10, Null<int64_t>(), KQuery::MIN));
    CHECK_EQ(raw->getQuery().kType(), KQuery::MIN);
    p->reset();
    CHECK_EQ(raw->getQuery().kType(), KQuery::DAY);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_short_side_defaults") {
    // The built-in MMs can cover a short; MM_FixedCount (n), MM_FixedRisk (risk), the net-assets
    // based ones and MM_Nothing (net assets) also open one, while MM_FixedCountTps falls back to
    // the base default (no short entry); the matrix is documented in the money_manager docs
    StockManager &sm = StockManager::instance();
    Stock stock = sm["sh600000"];

    /** @arg MM_FixedCount: sized by the fixed n; cover all */
    auto fcm = MM_FixedCount(300);
    fcm->setTM(crtTM());
    CHECK_EQ(fcm->getSellShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 300.0);
    CHECK_UNARY((fcm->getBuyShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL) ==
                 MAX_DOUBLE));

    /** @arg MM_Nothing: sized by the net assets (net assets 10000 / (10 * 1) = 1000); cover all */
    auto mm = MM_Nothing();
    mm->setTM(crtTM(Datetime(200001010000), 10000.0, TC_Zero()));
    CHECK_EQ(mm->getSellShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 1000.0);
    CHECK_UNARY(
      (mm->getBuyShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL) == MAX_DOUBLE));

    /** @arg MM_FixedCountTps has no override: the base defaults, no short entry, cover all */
    auto mm2 = MM_FixedCountTps({100.0}, {100.0});
    mm2->setTM(crtTM(Datetime(200001010000), 10000.0, TC_Zero()));
    CHECK_EQ(mm2->getSellShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 0.0);
    CHECK_UNARY(
      (mm2->getBuyShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL) == MAX_DOUBLE));
}

/** @par Test points */
TEST_CASE("test_MoneyManager_getSellShortNumber_lot_align") {
    StockManager& sm = StockManager::instance();

    /** @arg sh600000 has minTradeNumber == 100: a fractional size is aligned down to the lot
     * size (cash 10000 / capital 7 = 1428.57 -> 1400) */
    Stock stock = sm["sh600000"];
    REQUIRE_UNARY(!stock.isNull());
    auto mm = MM_FixedCapital(7.0);
    mm->setTM(crtTM(Datetime(200001010000), 10000.0, TC_Zero()));
    CHECK_EQ(mm->getSellShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 1400.0);

    /** @arg the size is clamped to maxTradeNumber: a stock with maxTradeNumber == 150 */
    Stock small_stock("TEST", "MMSHORT", "MM Short Align", 1, true, Datetime(199001010000),
                      Datetime(209901010000), 1.0, 1.0, 2, 1, 150);
    Parameter driver_param;
    driver_param.set<string>("type", "DoNothing");
    small_stock.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    REQUIRE_UNARY(!small_stock.isNull());
    auto mm2 = MM_FixedCapital(1.0);
    mm2->setTM(crtTM(Datetime(200001010000), 10000.0, TC_Zero()));
    CHECK_EQ(mm2->getSellShortNumber(Datetime(200001010000), small_stock, 10.0, 1.0, PART_SIGNAL),
             150.0);

    /** @arg below the lot size: rejected */
    auto mm3 = MM_FixedCapital(10000.0);
    mm3->setTM(crtTM(Datetime(200001010000), 100.0, TC_Zero()));
    CHECK_EQ(mm3->getSellShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 0.0);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_max_stock") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    Stock held = sm["sh600000"];
    REQUIRE_UNARY(!stock.isNull());
    REQUIRE_UNARY(!held.isNull());

    auto tm = crtTM(Datetime(200001010000), 100000.0);
    REQUIRE_UNARY(
      tm->buy(Datetime(200001010000), held, 10.0, 100, 0.0, 0.0, 10.0, PART_SIGNAL).business ==
      BUSINESS_BUY);
    CHECK_EQ(tm->getStockNumber(), 1);

    MoneyManagerPtr mm(new FixedBuyNumberMMTest);
    mm->setTM(tm);

    /** @arg max_stock=0 is rejected by the parameter check */
    CHECK_THROWS_AS(mm->setParam<int>("max_stock", 0), std::exception);

    /** @arg the held kinds reach the limit: the new buy is rejected */
    mm->setParam<int>("max_stock", 1);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 0.0);

    /** @arg below the limit: allowed */
    mm->setParam<int>("max_stock", 2);
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 100.0);
}

/** @par Test points */
TEST_CASE("test_MoneyManager_ev_cn_force_clean_position") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    MoneyManagerPtr mm(new MultSellMoneyManagerTest(false));
    mm->setTM(crtTM(Datetime(200001010000), 100000.0));
    Datetime dt(200001010000);

    /** @arg PART_ENVIRONMENT force-liquidates unless disabled (the subclass sizes 100 / 300) */
    CHECK_UNARY((mm->getSellNumber(dt, stock, 10.0, 1.0, PART_ENVIRONMENT) == MAX_DOUBLE));
    CHECK_UNARY((mm->getBuyShortNumber(dt, stock, 10.0, 1.0, PART_ENVIRONMENT) == MAX_DOUBLE));

    /** @arg PART_CONDITION force-liquidates unless disabled */
    CHECK_UNARY((mm->getSellNumber(dt, stock, 10.0, 1.0, PART_CONDITION) == MAX_DOUBLE));
    CHECK_UNARY((mm->getBuyShortNumber(dt, stock, 10.0, 1.0, PART_CONDITION) == MAX_DOUBLE));

    /** @arg disabling the EV clean position delegates to the subclass */
    mm->setParam<bool>("disable_ev_force_clean_position", true);
    CHECK_EQ(mm->getSellNumber(dt, stock, 10.0, 1.0, PART_ENVIRONMENT), 100.0);
    CHECK_EQ(mm->getBuyShortNumber(dt, stock, 10.0, 1.0, PART_ENVIRONMENT), 300.0);

    /** @arg disabling the CN clean position delegates to the subclass */
    mm->setParam<bool>("disable_cn_force_clean_position", true);
    CHECK_EQ(mm->getSellNumber(dt, stock, 10.0, 1.0, PART_CONDITION), 100.0);
    CHECK_EQ(mm->getBuyShortNumber(dt, stock, 10.0, 1.0, PART_CONDITION), 300.0);

    /** @arg PART_SIGNAL is never affected by the flags */
    mm->setParam<bool>("disable_ev_force_clean_position", false);
    mm->setParam<bool>("disable_cn_force_clean_position", false);
    CHECK_EQ(mm->getSellNumber(dt, stock, 10.0, 1.0, PART_SIGNAL), 100.0);
    CHECK_EQ(mm->getBuyShortNumber(dt, stock, 10.0, 1.0, PART_SIGNAL), 300.0);
}

/** @} */
