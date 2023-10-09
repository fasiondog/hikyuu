/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/strategy/StrategyBase.h>
#include "../_Parameter.h"

using namespace boost::python;
using namespace hku;

class StrategyBaseWrap : public StrategyBase, public wrapper<StrategyBase> {
public:
    StrategyBaseWrap() : StrategyBase() {}
    virtual ~StrategyBaseWrap() {}

    void init() override {
        if (override func = this->get_override("init")) {
            func();
        } else {
            this->StrategyBase::init();
        }
    }

    void default_init() {
        this->StrategyBase::init();
    }

    void onTick() override {
        if (override func = this->get_override("on_tick")) {
            func();
        } else {
            this->StrategyBase::onTick();
        }
    }

    void default_onTick() {
        this->StrategyBase::onTick();
    }

    void onBar(const KQuery::KType& ktype) override {
        if (override func = this->get_override("on_bar")) {
            func(ktype);
        } else {
            this->StrategyBase::onBar(ktype);
        }
    }

    void default_onBar(const KQuery::KType& ktype) {
        this->StrategyBase::onBar(ktype);
    }

    void onMarketOpen() override {
        if (override func = this->get_override("on_market_open")) {
            func();
        } else {
            this->StrategyBase::onMarketOpen();
        }
    }

    void default_onMarketOpen() {
        this->StrategyBase::onMarketOpen();
    }

    void onMarketClose() override {
        if (override func = this->get_override("on_market_close")) {
            func();
        } else {
            this->StrategyBase::onMarketClose();
        }
    }

    void default_onMarketClose() {
        this->StrategyBase::onMarketClose();
    }

    void onClock(TimeDelta delta) override {
        if (override func = this->get_override("on_clock")) {
            func(delta);
        } else {
            this->StrategyBase::onClock(delta);
        }
    }

    void default_onClock(TimeDelta delta) {
        this->StrategyBase::onClock(delta);
    }
};

const string& (StrategyBase::*strategy_get_name)() const = &StrategyBase::name;
void (StrategyBase::*strategy_set_name)(const string&) = &StrategyBase::name;

Datetime (StrategyBase::*get_strategy_start_datetime)() const = &StrategyBase::startDatetime;
void (StrategyBase::*set_strategy_start_datetime)(const Datetime&) = &StrategyBase::startDatetime;

void setStockList(StrategyBase* self, object seq) {
    vector<string> stk_list;
    size_t total = len(seq);
    for (size_t i = 0; i < total; i++) {
        extract<string> x(seq[i]);
        if (x.check()) {
            stk_list.push_back(x());
        }
    }
    self->setStockCodeList(std::move(stk_list));
}

void setKTypeList(StrategyBase* self, object seq) {
    vector<string> stk_list;
    size_t total = len(seq);
    for (size_t i = 0; i < total; i++) {
        extract<string> x(seq[i]);
        if (x.check()) {
            stk_list.push_back(x());
        }
    }
    self->setKTypeList(stk_list);
}

void export_Strategy() {
    class_<StrategyBaseWrap, boost::noncopyable>("StrategyBase", init<>())
      .add_property("name",
                    make_function(strategy_get_name, return_value_policy<copy_const_reference>()),
                    strategy_set_name)
      .add_property(
        "sm", make_function(&StrategyBase::getSM, return_value_policy<reference_existing_object>()),
        "获取 StockManager 实例")
      .add_property("tm", &StrategyBase::getTM, &StrategyBase::setTM, "账户管理")
      .add_property("start_datetime", get_strategy_start_datetime, set_strategy_start_datetime,
                    "起始日期")
      .add_property(
        "stock_list",
        make_function(&StrategyBase::getStockCodeList, return_value_policy<copy_const_reference>()),
        setStockList, "股票代码列表")
      .add_property(
        "ktype_list",
        make_function(&StrategyBase::getKTypeList, return_value_policy<copy_const_reference>()),
        setKTypeList, "需要的K线类型")

      .def("run", &StrategyBase::run)
      .def("init", &StrategyBase::init, &StrategyBaseWrap::default_init)
      .def("on_tick", &StrategyBase::onTick, &StrategyBaseWrap::default_onTick)
      .def("on_bar", &StrategyBase::onBar, &StrategyBaseWrap::default_onBar)
      .def("on_market_open", &StrategyBase::onMarketOpen, &StrategyBaseWrap::default_onMarketOpen)
      .def("on_market_close", &StrategyBase::onMarketClose,
           &StrategyBaseWrap::default_onMarketClose)
      .def("on_clock", &StrategyBase::onClock, &StrategyBaseWrap::default_onClock);
}