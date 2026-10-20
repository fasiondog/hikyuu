/*
 * test_ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/profitgoal/ProfitGoalBase.h>

using namespace hku;

class ProfitGoalTest : public ProfitGoalBase {
public:
    ProfitGoalTest() : ProfitGoalBase("ProfitGoalTest"), m_x(0) {}
    virtual ~ProfitGoalTest() {}

    virtual price_t _getGoal(const Datetime &datetime, price_t price) {
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

class ProfitGoalCalcTest : public ProfitGoalBase {
public:
    ProfitGoalCalcTest() : ProfitGoalBase("ProfitGoalCalcTest") {}
    virtual ~ProfitGoalCalcTest() {}

    virtual price_t _getGoal(const Datetime &, price_t) {
        return Null<price_t>();
    }

    virtual ProfitGoalPtr _clone() {
        return ProfitGoalPtr(new ProfitGoalCalcTest);
    }

    virtual void _calculate() {
        m_calc_count++;
    }

    int getCalcCount() const {
        return m_calc_count;
    }

private:
    int m_calc_count{0};
};

class ProfitGoalNormalizeTest : public ProfitGoalBase {
public:
    ProfitGoalNormalizeTest() : ProfitGoalBase("ProfitGoalNormalizeTest") {}
    virtual ~ProfitGoalNormalizeTest() {}

    void setGoal(price_t g) {
        m_goal = g;
    }
    void setShortGoal(price_t g) {
        m_short_goal = g;
    }

    virtual price_t _getGoal(const Datetime &, price_t) override {
        return m_goal;
    }
    virtual price_t _getShortGoal(const Datetime &, price_t) override {
        return m_short_goal;
    }
    virtual ProfitGoalPtr _clone() override {
        return ProfitGoalPtr(new ProfitGoalNormalizeTest);
    }

private:
    price_t m_goal{Null<price_t>()};
    price_t m_short_goal{Null<price_t>()};
};

/**
 * @defgroup test_ProfitGoal test_ProfitGoal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ProfitGoal") {
    /** @arg The basic operation */
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

    /** @arg Test the clone operation */
    p_src->setX(10);
    ProfitGoalPtr p_clone = p->clone();
    CHECK_EQ(p_clone->name(), "ProfitGoalTest");
    p_src = (ProfitGoalTest *)p_clone.get();
    CHECK_EQ(p_src->getX(), 10);
    CHECK_NE(p, p_clone);

    /** @arg The default short goal interface sets no target */
    CHECK_UNARY(std::isnan(p->getShortGoal(Datetime(200101010000), 1.0)));
}

/** @arg getGoal normalizes an invalid return to Null (no goal); the valid semantics are kept */
TEST_CASE("test_ProfitGoal_normalizeGoal") {
    auto assert_null_or_eq = [](price_t got, price_t want) {
        if (std::isnan(want)) {
            CHECK_UNARY(std::isnan(got));
        } else {
            CHECK_EQ(got, want);
        }
    };
    const price_t nan = Null<price_t>();
    const price_t inf = std::numeric_limits<double>::infinity();

    ProfitGoalPtr p(new ProfitGoalNormalizeTest);
    auto* raw = (ProfitGoalNormalizeTest*)p.get();
    Datetime dt(200101010000);

    /** @arg long: NaN / 0 / positive finite kept; negative / inf dropped to Null */
    raw->setGoal(nan);
    assert_null_or_eq(p->getGoal(dt, 1.0), nan);
    raw->setGoal(0.0);
    assert_null_or_eq(p->getGoal(dt, 1.0), 0.0);
    raw->setGoal(10.0);
    assert_null_or_eq(p->getGoal(dt, 1.0), 10.0);
    raw->setGoal(-1.0);
    assert_null_or_eq(p->getGoal(dt, 1.0), nan);
    raw->setGoal(inf);
    assert_null_or_eq(p->getGoal(dt, 1.0), nan);
    raw->setGoal(-inf);
    assert_null_or_eq(p->getGoal(dt, 1.0), nan);

    /** @arg short: same rules, and returning the current price is kept as a forced close */
    raw->setShortGoal(inf);
    assert_null_or_eq(p->getShortGoal(dt, 8.0), nan);
    raw->setShortGoal(-2.0);
    assert_null_or_eq(p->getShortGoal(dt, 8.0), nan);
    raw->setShortGoal(8.0);
    assert_null_or_eq(p->getShortGoal(dt, 8.0), 8.0);

    /** @arg the clamp is idempotent and reset does not change the behavior (still no goal) */
    raw->setGoal(-1.0);
    assert_null_or_eq(p->getGoal(dt, 1.0), nan);
    assert_null_or_eq(p->getGoal(dt, 1.0), nan);
    p->reset();
    raw->setGoal(-1.0);
    assert_null_or_eq(p->getGoal(dt, 1.0), nan);
}

/** @par Test points */
TEST_CASE("test_ProfitGoal_setTO") {
    StockManager &sm = StockManager::instance();
    Stock stk = sm.getStock("sh600000");
    KData k =
      stk.getKData(KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY));
    KData other =
      stk.getKData(KQueryByDate(Datetime(199911100000LL), Datetime(200001250000LL), KQuery::DAY));

    ProfitGoalPtr p(new ProfitGoalCalcTest);
    ProfitGoalCalcTest *p_src = (ProfitGoalCalcTest *)p.get();

    /** @arg setTO calls the subclass calculation once for the same trading object */
    p->setTO(k);
    CHECK_EQ(p_src->getCalcCount(), 1);
    p->setTO(k);
    CHECK_EQ(p_src->getCalcCount(), 1);

    /** @arg another trading object is calculated again */
    p->setTO(other);
    CHECK_EQ(p_src->getCalcCount(), 2);

    /** @arg a null trading object is not calculated, and reset releases the calculation state */
    p->setTO(Null<KData>());
    CHECK_EQ(p_src->getCalcCount(), 2);
    p->reset();
    CHECK_UNARY(p->getTO().empty());
    p->setTO(k);
    CHECK_EQ(p_src->getCalcCount(), 3);

    /** @arg the same trading object is not calculated again, even after a parameter change */
    p->setParam<double>("n", 2.0);
    p->setTO(k);
    CHECK_EQ(p_src->getCalcCount(), 3);

    /** @arg the clone is not calculated until setTO */
    ProfitGoalPtr p_clone = p->clone();
    ProfitGoalCalcTest *c_src = (ProfitGoalCalcTest *)p_clone.get();
    CHECK_EQ(c_src->getCalcCount(), 0);
    c_src->setTO(k);
    CHECK_EQ(c_src->getCalcCount(), 1);
}

/** @} */
