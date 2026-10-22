/*
 * test_System_hasPythonPart.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-22
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/indicator/Indicator.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_manage/TradeManager.h>
#include <hikyuu/trade_sys/condition/crt/CN_Bool.h>
#include <hikyuu/trade_sys/condition/crt/CN_Logic.h>
#include <hikyuu/trade_sys/environment/crt/EV_Bool.h>
#include <hikyuu/trade_sys/multifactor/ScoresFilterBase.h>
#include <hikyuu/trade_sys/selector/SelectorBase.h>
#include <hikyuu/trade_sys/signal/crt/SG_AllwaysBuy.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_Indicator.h>
#include <hikyuu/trade_sys/system/System.h>
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>

using namespace hku;

/**
 * @defgroup test_system_has_python_part test_system_has_python_part
 * @ingroup test_hikyuu_system_suite
 * @{
 */

namespace {

class PyMarkedImp : public IndicatorImp {
public:
    PyMarkedImp() : IndicatorImp("PyMarkedImp") {
        m_is_python_object = true;
    }
    virtual IndicatorImpPtr _clone() override {
        return std::make_shared<PyMarkedImp>();
    }
};

class PyMarkedSignal : public SignalBase {
public:
    PyMarkedSignal() {
        m_is_python_object = true;
    }
    virtual SignalPtr _clone() override {
        return std::make_shared<PyMarkedSignal>();
    }
    virtual void _calculate(const KData&) override {}
};

class PyMarkedSelector : public SelectorBase {
public:
    PyMarkedSelector() {
        m_is_python_object = true;
    }
    virtual SelectorPtr _clone() override {
        return std::make_shared<PyMarkedSelector>();
    }
    virtual void _calculate() override {}
    virtual SystemWeightList _getSelected(Datetime date) override {
        return SystemWeightList();
    }
};

class PlainSelector : public SelectorBase {
public:
    virtual SelectorPtr _clone() override {
        return std::make_shared<PlainSelector>();
    }
    virtual void _calculate() override {}
    virtual SystemWeightList _getSelected(Datetime date) override {
        return SystemWeightList();
    }
    void addProtoSystem(const SystemPtr& sys) {
        m_pro_sys_list.push_back(sys);
    }
};

class PyMarkedFilter : public ScoresFilterBase {
public:
    PyMarkedFilter() : ScoresFilterBase("PyMarkedFilter") {
        m_is_python_object = true;
    }
    virtual ScoreRecordList _filter(const ScoreRecordList& scores, const Datetime& date,
                                    const KQuery& query) override {
        return scores;
    }
    virtual ScoresFilterPtr _clone() override {
        return std::make_shared<PyMarkedFilter>();
    }
};

class PyMarkedCost : public TradeCostBase {
public:
    PyMarkedCost() : TradeCostBase("PyMarkedCost") {
        m_is_python_object = true;
    }
    virtual CostRecord getBuyCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord();
    }
    virtual CostRecord getSellCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord();
    }
    virtual TradeCostPtr _clone() override {
        return std::make_shared<PyMarkedCost>();
    }
};

}  // namespace

/** @par Test points */
TEST_CASE("test_trade_sys_part_hasPythonPart") {
    Indicator py_ind(std::make_shared<PyMarkedImp>());
    Indicator cpp_ind = CLOSE();

    /** @arg a condition holding a python indicator reports it */
    CHECK_UNARY(CN_Bool(py_ind)->hasPythonPart());
    CHECK_UNARY(!CN_Bool(cpp_ind)->hasPythonPart());

    /** @arg an environment holding a python indicator reports it */
    CHECK_UNARY(EV_Bool(py_ind)->hasPythonPart());
    CHECK_UNARY(!EV_Bool(cpp_ind)->hasPythonPart());

    /** @arg a stoploss holding a python indicator reports it */
    CHECK_UNARY(ST_Indicator(py_ind)->hasPythonPart());
    CHECK_UNARY(!ST_Indicator(cpp_ind)->hasPythonPart());
}

/** @par Test points */
TEST_CASE("test_System_hasPythonPart_parts") {
    /** @arg a system without any python part */
    System sys;
    sys.setSG(SG_AllwaysBuy());
    CHECK_UNARY(!sys.hasPythonPart());

    /** @arg a python signal makes the system report it */
    System sys_sg;
    sys_sg.setSG(std::make_shared<PyMarkedSignal>());
    CHECK_UNARY(sys_sg.hasPythonPart());

    /** @arg a c++ condition holding a python indicator makes the system report it */
    System sys_cn;
    sys_cn.setCN(CN_Bool(Indicator(std::make_shared<PyMarkedImp>())));
    CHECK_UNARY(sys_cn.hasPythonPart());

    /** @arg the aggregate form reports a python selector */
    MultiSystem ms;
    CHECK_UNARY(!ms.hasPythonPart());
    ms.setSE(std::make_shared<PyMarkedSelector>());
    CHECK_UNARY(ms.hasPythonPart());
}

/** @par Test points */
TEST_CASE("test_System_hasPythonPart_nested") {
    Indicator py_ind(std::make_shared<PyMarkedImp>());
    Indicator cpp_ind = CLOSE();

    /** @arg a logical condition combination reports the python operand */
    CHECK_UNARY((CN_Bool(cpp_ind) & CN_Bool(py_ind))->hasPythonPart());
    CHECK_UNARY(!(CN_Bool(cpp_ind) & CN_Bool(cpp_ind))->hasPythonPart());

    /** @arg a selector reports a python scores filter and a python prototype system */
    auto only_filter = std::make_shared<PyMarkedFilter>();
    CHECK_UNARY(only_filter->hasPythonPart());
    PlainSelector by_filter;
    by_filter.setScoresFilter(only_filter);
    CHECK_UNARY(by_filter.getScoresFilter() == only_filter);
    CHECK_UNARY(by_filter.hasPythonPart());

    auto proto = std::make_shared<System>();
    proto->setSG(std::make_shared<PyMarkedSignal>());
    auto by_proto = std::make_shared<PlainSelector>();
    by_proto->addProtoSystem(proto);
    CHECK_UNARY(by_proto->hasPythonPart());

    MultiSystem ms;
    ms.setSE(by_proto);
    CHECK_UNARY(ms.hasPythonPart());

    /** @arg a trade manager reports a python cost function */
    TradeManager tm;
    CHECK_UNARY(!tm.hasPythonPart());
    tm.costFunc(std::make_shared<PyMarkedCost>());
    CHECK_UNARY(tm.hasPythonPart());
}

/** @} */
