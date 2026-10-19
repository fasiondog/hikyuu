/*
 * test_LogicCondition.cpp
 *
 *  Created on: 2026-10-14
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/condition/ConditionBase.h>
#include <hikyuu/trade_sys/condition/crt/CN_Bool.h>
#include <hikyuu/trade_sys/condition/crt/CN_Logic.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

namespace {
// A condition that fills every bar with a constant value, used to inject non-finite operand values
class ConstCondition : public ConditionBase {
public:
    explicit ConstCondition(price_t v) : ConditionBase("TEST_CONST"), m_v(v) {}

    void _calculate() override {
        for (size_t i = 0; i < m_values.size(); ++i) {
            m_values[i] = m_v;
        }
    }

    ConditionPtr _clone() override {
        return std::make_shared<ConstCondition>(m_v);
    }

private:
    price_t m_v;
};
}  // namespace

/**
 * @defgroup test_LogicCondition test_LogicCondition
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test point: a null operand in CN_Div degrades to always-invalid instead of crashing */
TEST_CASE("test_LogicCondition_div_null_operand") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0, 10));
    if (kdata.empty()) {
        return;
    }

    ConditionPtr a = CN_Bool(CVAL(1.0));
    a->setTO(kdata);

    /** @arg dividing by a null condition yields no valid day (CN-004) */
    ConditionPtr d = a / ConditionPtr();
    d->setTO(kdata);
    CHECK_EQ(d->getDatetimeList().size(), 0);
    for (size_t i = 0; i < kdata.size(); ++i) {
        CHECK_UNARY_FALSE(d->isValid(kdata[i].datetime));
    }
}

/** @par Test point: non-finite values in combination conditions are filtered to Null */
TEST_CASE("test_LogicCondition_isfinite_filter") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0, 10));
    if (kdata.empty()) {
        return;
    }

    price_t nan_v = Null<price_t>();
    price_t inf_v = std::numeric_limits<double>::infinity();
    ConditionPtr nan_c = std::make_shared<ConstCondition>(nan_v);
    ConditionPtr inf_c = std::make_shared<ConstCondition>(inf_v);
    ConditionPtr c2 = std::make_shared<ConstCondition>(2.0);
    ConditionPtr c3 = std::make_shared<ConstCondition>(3.0);

    /** @arg Add with a NaN/inf operand yields Null at every bar (CN-005) */
    ConditionPtr add_nan = nan_c + c2;
    add_nan->setTO(kdata);
    CHECK_UNARY(std::isnan(add_nan->at(0)));
    ConditionPtr add_inf = inf_c + c3;
    add_inf->setTO(kdata);
    CHECK_UNARY(std::isnan(add_inf->at(0)));

    /** @arg Add of finite operands is unchanged */
    ConditionPtr add_ok = c2 + c3;
    add_ok->setTO(kdata);
    CHECK_EQ(add_ok->at(0), 5.0);

    /** @arg Sub / Multi with a NaN operand yield Null, finite operands unchanged */
    ConditionPtr sub_nan = nan_c - c2;
    sub_nan->setTO(kdata);
    CHECK_UNARY(std::isnan(sub_nan->at(0)));
    ConditionPtr sub_ok = c3 - c2;
    sub_ok->setTO(kdata);
    CHECK_EQ(sub_ok->at(0), 1.0);

    ConditionPtr mul_nan = nan_c * c2;
    mul_nan->setTO(kdata);
    CHECK_UNARY(std::isnan(mul_nan->at(0)));
    ConditionPtr mul_ok = c2 * c3;
    mul_ok->setTO(kdata);
    CHECK_EQ(mul_ok->at(0), 6.0);
}

/** @par Test points: per-operator single-side / empty semantics (lock the CN combination matrix) */
TEST_CASE("test_LogicCondition_single_side") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0, 10));
    if (kdata.empty()) {
        return;
    }
    size_t total = kdata.size();

    ConditionPtr null_c;
    ConditionPtr c2 = std::make_shared<ConstCondition>(2.0);
    ConditionPtr c3 = std::make_shared<ConstCondition>(3.0);
    ConditionPtr r;

    /** @arg Add: single side passes through; both null yields all invalid */
    r = c2 + null_c;
    r->setTO(kdata);
    CHECK_EQ(r->at(0), 2.0);
    CHECK_EQ(r->getDatetimeList().size(), total);

    r = null_c + c3;
    r->setTO(kdata);
    CHECK_EQ(r->at(0), 3.0);

    r = null_c + null_c;
    r->setTO(kdata);
    CHECK_EQ(r->at(0), 0.0);
    CHECK_EQ(r->getDatetimeList().size(), 0);

    /** @arg Sub: only cond2 negates (-d2); only cond1 passes d1 through */
    r = null_c - c3;
    r->setTO(kdata);
    CHECK_EQ(r->at(0), -3.0);
    CHECK_EQ(r->getDatetimeList().size(), 0);

    r = c2 - null_c;
    r->setTO(kdata);
    CHECK_EQ(r->at(0), 2.0);

    /** @arg Or: a single valid side is normalized to 1.0 */
    r = c2 | null_c;
    r->setTO(kdata);
    CHECK_EQ(r->at(0), 1.0);
    CHECK_EQ(r->getDatetimeList().size(), total);

    r = null_c | c3;
    r->setTO(kdata);
    CHECK_EQ(r->at(0), 1.0);

    /** @arg And needs both operands: a null side yields always invalid */
    r = c2 & null_c;
    r->setTO(kdata);
    CHECK_EQ(r->at(0), 0.0);
    CHECK_EQ(r->getDatetimeList().size(), 0);

    r = null_c & c3;
    r->setTO(kdata);
    CHECK_EQ(r->getDatetimeList().size(), 0);

    /** @arg Multi needs both operands: a null side yields always invalid */
    r = c2 * null_c;
    r->setTO(kdata);
    CHECK_EQ(r->at(0), 0.0);
    CHECK_EQ(r->getDatetimeList().size(), 0);

    /** @arg Div: a null right side yields NaN; a null left side yields all 0 (invalid) */
    r = c2 / null_c;
    r->setTO(kdata);
    CHECK_UNARY(std::isnan(r->at(0)));
    CHECK_EQ(r->getDatetimeList().size(), 0);

    r = null_c / c3;
    r->setTO(kdata);
    CHECK_EQ(r->at(0), 0.0);
    CHECK_EQ(r->getDatetimeList().size(), 0);
}

/** @} */
