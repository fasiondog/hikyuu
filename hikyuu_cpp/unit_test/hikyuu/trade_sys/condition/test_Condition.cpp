/*
 * test_Condition.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/condition/ConditionBase.h>

using namespace hku;

class ConditionTest : public ConditionBase {
public:
    ConditionTest() : ConditionBase("TEST") {
        setParam<int>("n", 10);
        m_flag = false;
    }

    ~ConditionTest() {}

    KData getKData() const {
        return m_kdata;
    }

    virtual void _calculate() {
        _addValid(m_kdata[2].datetime, 2.0);
        _addValid(m_kdata[3].datetime, 1.5);
    }

    virtual void _reset() {
        if (m_flag) {
            m_flag = false;
        } else {
            m_flag = true;
        }
    }

    virtual ConditionPtr _clone() {
        ConditionTest *p = new ConditionTest;
        p->m_flag = m_flag;
        return ConditionPtr(p);
    }

private:
    bool m_flag;
};

/**
 * @defgroup test_Condition test_Condition
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_Condition") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    ConditionTest *p_src;

    /** @arg 基本操作 */
    ConditionPtr p(new ConditionTest);
    p_src = (ConditionTest *)p.get();
    CHECK_EQ(p->name(), "TEST");
    CHECK_EQ(p->getParam<int>("n"), 10);
    CHECK_EQ(p_src->getKData().getStock(), Null<Stock>());
    REQUIRE(p->size() == 0);
    p->setTO(kdata);
    CHECK_EQ(p->size(), kdata.size());
    CHECK_EQ(p_src->getKData().getStock(), stock);
    CHECK_UNARY(p->isValid(Datetime(19901221)));
    CHECK_UNARY(p->isValid(Datetime(19901224)));
    CHECK_UNARY_FALSE(p->isValid(Datetime(19901219)));

    auto ds = p->getDatetimeList();
    CHECK_EQ(ds.size(), 2);
    CHECK_EQ(ds[0], Datetime(19901221));
    CHECK_EQ(ds[1], Datetime(19901224));

    /** @arg 克隆操作 */
    p->setParam<int>("n", 20);
    ConditionPtr p_clone = p->clone();
    CHECK_NE(p, p_clone);
    p_src = (ConditionTest *)p_clone.get();
    CHECK_EQ(p_src->getKData().getStock(), stock);

    p->setParam<int>("n", 30);
    CHECK_EQ(p->getParam<int>("n"), 30);
    p->reset();
    CHECK_EQ(p->isValid(Datetime(200001010000)), false);

    CHECK_EQ(p_clone->name(), "TEST");
    CHECK_EQ(p_clone->getParam<int>("n"), 20);
    // CHECK_EQ(p_clone->isValid(Datetime(200001010000)) , true);
    p_clone->reset();
    CHECK_EQ(p_clone->isValid(Datetime(200001010000)), false);
}

/** @} */
