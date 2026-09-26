/*
 * test_MoneyManager.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
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

/** @} */
