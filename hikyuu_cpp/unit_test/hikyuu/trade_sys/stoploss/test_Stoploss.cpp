/*
 * test_Stoploss.cpp
 *
 *  Created on: 2013-3-13
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/stoploss/StoplossBase.h>

using namespace hku;

class StoplossTest : public StoplossBase {
public:
    StoplossTest() : StoplossBase("StoplossTest"), m_x(0) {}
    virtual ~StoplossTest() {}

    virtual price_t getPrice(const Datetime &datetime, price_t price) {
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

    virtual price_t getPrice(const Datetime &, price_t) override {
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

/** @} */
