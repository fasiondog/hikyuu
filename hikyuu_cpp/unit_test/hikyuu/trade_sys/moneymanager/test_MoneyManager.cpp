/*
 * test_MoneyManager.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/data_driver/DataDriverFactory.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/moneymanager/MoneyManagerBase.h>

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

class SelfCloneMoneyManagerTest : public MoneyManagerBase {
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

/** @} */
