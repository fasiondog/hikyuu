/*
 * test_ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/profitgoal/ProfitGoalBase.h>

using namespace hku;

class ProfitGoalTest : public ProfitGoalBase {
public:
    ProfitGoalTest() : ProfitGoalBase("ProfitGoalTest"), m_x(0) {}
    virtual ~ProfitGoalTest() {}

    virtual price_t getGoal(const Datetime &datetime, price_t price) {
        return m_x < 10 ? 0.0 : 1.0;
    }

    virtual void _reset() {
        m_x = 0;
    }

    virtual ProfitGoalPtr _clone() {
        ProfitGoalTest *p = new ProfitGoalTest;
        p->m_x = m_x;
        return ProfitGoalPtr(p);
    }

    virtual void _calculate() {}

    int getX() const {
        return m_x;
    }
    void setX(int x) {
        m_x = x;
    }

private:
    int m_x;
};

/**
 * @defgroup test_ProfitGoal test_ProfitGoal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ProfitGoal") {
    /** @arg 基本操作 */
    ProfitGoalPtr p(new ProfitGoalTest);
    CHECK_EQ(p->name(), "ProfitGoalTest");
    CHECK_EQ(p->getGoal(Datetime(200101010000), 1.0), 0.0);
    ProfitGoalTest *p_src = (ProfitGoalTest *)p.get();
    CHECK_EQ(p_src->getX(), 0);

    p_src->setX(10);
    CHECK_EQ(p->getGoal(Datetime(200101010000), 1.0), 1.0);
    CHECK_EQ(p_src->getX(), 10);
    p->reset();
    CHECK_EQ(p_src->getX(), 0);

    /** @arg 测试克隆操作 */
    p_src->setX(10);
    ProfitGoalPtr p_clone = p->clone();
    CHECK_EQ(p_clone->name(), "ProfitGoalTest");
    p_src = (ProfitGoalTest *)p_clone.get();
    CHECK_EQ(p_src->getX(), 10);
    CHECK_NE(p, p_clone);
}

/** @} */
