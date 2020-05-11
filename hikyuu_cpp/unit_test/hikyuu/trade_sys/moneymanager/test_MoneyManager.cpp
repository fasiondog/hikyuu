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

/**
 * @defgroup test_MoneyManager test_MoneyManager
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MoneyManager") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm["sh000001"];
    TradeManagerPtr tm = crtTM();

    /** @arg 基本操作 */
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

    /** @arg 克隆操作 */
    p_src->setX(10);
    MoneyManagerPtr p_clone = p->clone();
    CHECK_NE(p, p_clone);
    p_src = (MoneyManagerTest *)p_clone.get();
    CHECK_EQ(p->name(), "MoneyManagerTest");
    // CHECK_EQ(p_src->getTM() == tm);
    CHECK_EQ(p_src->getX(), 10);
}

/** @} */
