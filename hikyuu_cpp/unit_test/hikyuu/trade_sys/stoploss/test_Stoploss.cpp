/*
 * test_Stoploss.cpp
 *
 *  Created on: 2013-3-13
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/build_in.h>
#include <hikyuu/trade_sys/stoploss/StoplossBase.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_Indicator.h>
#include <hikyuu/trade_sys/stoploss/imp/FixedPercentStoploss.h>
#include <hikyuu/trade_sys/stoploss/imp/IndicatorStoploss.h>

using namespace hku;

class StoplossTest : public StoplossBase {
public:
    StoplossTest() : StoplossBase("StoplossTest"), m_x(0) {}
    virtual ~StoplossTest() {}

    virtual price_t _getPrice(const Datetime &datetime, price_t price) {
        return m_x < 10 ? 0.0 : 1.0;
    }

    virtual void _reset() {
        m_x = 0;
    }

    virtual StoplossPtr _clone() {
        StoplossTest *p = new StoplossTest;
        p->m_x = m_x;
        return StoplossPtr(p);
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

class StoplossAccum : public StoplossBase {
public:
    StoplossAccum() : StoplossBase("StoplossAccum") {}
    virtual ~StoplossAccum() {}

    virtual price_t _getPrice(const Datetime &, price_t) override {
        return 0.0;
    }
    virtual void _reset() override {
        m_count = 0;
    }
    virtual void _calculate() override {
        m_count += static_cast<int>(m_kdata.size());
    }
    virtual StoplossPtr _clone() override {
        auto p = make_shared<StoplossAccum>();
        p->m_count = m_count;
        return p;
    }

    int count() const {
        return m_count;
    }

private:
    int m_count{0};
};

// A subclass that, like Condition/Environment subclasses, copies its cached calculation result in
// _clone(); the base clone() copies m_kdata, so the clone is immediately usable (EV/CN convention).
class StoplossCloneWithCache : public StoplossBase {
public:
    StoplossCloneWithCache() : StoplossBase("StoplossCloneWithCache"), m_calc(0) {
        setParam<int>("offset", 0);
    }
    virtual ~StoplossCloneWithCache() {}
    virtual price_t _getPrice(const Datetime &, price_t) override {
        return static_cast<price_t>(m_calc);
    }
    virtual void _reset() override {
        m_calc = -1;
    }
    virtual void _calculate() override {
        m_calc = static_cast<int>(m_kdata.size()) + getParam<int>("offset");
    }
    virtual StoplossPtr _clone() override {
        auto p = make_shared<StoplossCloneWithCache>();
        p->m_calc = m_calc;
        return p;
    }

private:
    int m_calc{0};
};

/**
 * @defgroup test_Stoploss test_Stoploss
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Stoploss") {
    /** @arg The basic operation */
    StoplossPtr p(new StoplossTest);
    CHECK_EQ(p->name(), "StoplossTest");
    CHECK_EQ(p->getPrice(Datetime(200101010000), 1.0), 0.0);
    StoplossTest *p_src = (StoplossTest *)p.get();
    CHECK_EQ(p_src->getX(), 0);

    p_src->setX(10);
    CHECK_EQ(p->getPrice(Datetime(200101010000), 1.0), 1.0);
    CHECK_EQ(p_src->getX(), 10);
    p->reset();
    CHECK_EQ(p_src->getX(), 0);

    /** @arg Test the clone operation */
    p_src->setX(10);
    StoplossPtr p_clone = p->clone();
    CHECK_EQ(p_clone->name(), "StoplossTest");
    p_src = (StoplossTest *)p_clone.get();
    CHECK_EQ(p_src->getX(), 10);
    CHECK_NE(p, p_clone);
}

/** @arg setTO clears the subclass cache (_reset) before _calculate, so switching KData does not
 * accumulate stale results */
TEST_CASE("test_Stoploss_setTO_reset_before_calculate") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    KData k2 = sm.getStock("sz000001").getKData(KQuery(0, 5));
    CHECK_UNARY(!k1.empty());
    CHECK_UNARY(!k2.empty());

    auto p = make_shared<StoplossAccum>();
    p->setTO(k1);
    CHECK_EQ(p->count(), static_cast<int>(k1.size()));

    p->setTO(k2);
    CHECK_EQ(p->count(), static_cast<int>(k2.size()));
}

/** @arg clone copies m_kdata and the subclass cache, so the clone is directly usable; setTO always
 * recalculates (lazy recalculation), with the same or a different KData alike */
TEST_CASE("test_Stoploss_clone_preserves_cache") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    KData k2 = sm.getStock("sz000001").getKData(KQuery(0, 5));
    CHECK_UNARY(!k1.empty());
    CHECK_UNARY(!k2.empty());

    auto p = make_shared<StoplossCloneWithCache>();
    p->setTO(k1);
    CHECK_EQ(p->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k1.size()));

    StoplossPtr pc = p->clone();
    // The clone carries the cached result and is usable before setTO
    CHECK_EQ(pc->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k1.size()));

    // setTO with the same KData recalculates instead of short-circuiting
    pc->setTO(k1);
    CHECK_EQ(pc->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k1.size()));

    // A different KData triggers a fresh calculation
    pc->setTO(k2);
    CHECK_EQ(pc->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k2.size()));
}

/** @arg stop-loss is floored to the price tick and never rounds back up to the buy price (ST-001)
 */
TEST_CASE("test_Stoploss_fixedpercent_rounding") {
    auto st = ST_FixedPercent(0.002);
    // empty kdata -> precision 2; 2.50*(1-0.002)=2.495 -> floor -> 2.49 (roundEx would give 2.50)
    CHECK_EQ(st->getPrice(Datetime(200101010000), 2.50), 2.49);
    // 3.33*(1-0.001)=3.32667... -> floor -> 3.32
    CHECK_EQ(st->getPrice(Datetime(200101010000), 3.33), 3.32);

    auto st2 = ST_FixedPercent(0.03);
    CHECK_LT(st2->getPrice(Datetime(200101010000), 10.0), 10.0);
}

/** @arg short stop-loss is above the planned price and invalid input yields no stop-loss (ST-006)
 */
TEST_CASE("test_Stoploss_fixedpercent_short_and_validation") {
    auto st = ST_FixedPercent(0.03);
    // empty kdata -> precision 2; 10*(1+0.03)=10.3 -> roundUp -> 10.3 (above 10)
    CHECK_EQ(st->getShortPrice(Datetime(200101010000), 10.0), 10.3);
    CHECK_EQ(st->getPrice(Datetime(200101010000), 0.0), 0.0);
    CHECK_EQ(st->getShortPrice(Datetime(200101010000), 0.0), 0.0);
}

/** @arg non-finite indicator values are dropped, so getPrice returns 0 (no stop-loss) (ST-005) */
TEST_CASE("test_Stoploss_indicator_nan_filtered") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    CHECK_UNARY(!k1.empty());

    // CLOSE()/(CLOSE()-CLOSE()) divides by zero -> Inf/NaN at every bar
    Indicator nan_ind = CLOSE() / (CLOSE() - CLOSE());
    auto st = ST_Indicator(nan_ind);
    st->setTO(k1);
    for (size_t i = 0; i < k1.size(); ++i) {
        price_t v = st->getPrice(k1[i].datetime, 0.0);
        CHECK_UNARY(std::isfinite(v));
        CHECK_EQ(v, 0.0);
    }
}

/** @arg clone keeps its own result cache independent of the source (ST-007) */
TEST_CASE("test_Stoploss_indicator_clone_independent") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    KData k2 = sm.getStock("sz000001").getKData(KQuery(5, 15));
    CHECK_UNARY(!k1.empty());
    CHECK_UNARY(!k2.empty());

    auto st = ST_Indicator(CLOSE());
    st->setTO(k1);
    StoplossPtr st_clone = st->clone();
    st->setTO(k2);
    for (size_t i = 0; i < k1.size(); ++i) {
        CHECK_EQ(st_clone->getPrice(k1[i].datetime, 0.0), k1[i].closePrice);
    }
}

/** @arg paramChanged invalidates the cache, so setTO with the same KData recomputes (ST-009) */
TEST_CASE("test_Stoploss_paramChanged_invalidates_cache") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    CHECK_UNARY(!k1.empty());

    auto p = make_shared<StoplossCloneWithCache>();
    p->setTO(k1);
    CHECK_EQ(p->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k1.size()));

    p->setParam<int>("offset", 7);
    // Same KData, but the parameter changed -> cache must be invalidated and recomputed
    p->setTO(k1);
    CHECK_EQ(p->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k1.size() + 7));
}

/** @} */
