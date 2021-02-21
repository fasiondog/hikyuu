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
        this->get_override("init")();
    }

    void on_spot() override {
        this->get_override("on_spot")();
    }
};

const string& (StrategyBase::*strategy_get_name)() const = &StrategyBase::name;
void (StrategyBase::*strategy_set_name)(const string&) = &StrategyBase::name;

Datetime (StrategyBase::*get_start_datetime)() const = &StrategyBase::startDatetime;
void (StrategyBase::*set_start_datetime)(const Datetime&) = &StrategyBase::startDatetime;

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
      .add_property("start_datetime", get_start_datetime, set_start_datetime, "起始日期")
      .add_property(
        "stock_list",
        make_function(&StrategyBase::getStockCodeList, return_value_policy<copy_const_reference>()),
        setStockList, "股票代码列表")
      .add_property(
        "ktype_list",
        make_function(&StrategyBase::getKTypeList, return_value_policy<copy_const_reference>()),
        setKTypeList, "需要的K线类型")

      .def("run", &StrategyBase::run)
      .def("init", pure_virtual(&StrategyBase::init))
      .def("on_spot", pure_virtual(&StrategyBase::on_spot));
}