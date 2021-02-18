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

    void on_bar() override {
        this->get_override("on_bar")();
    }
};

const string& (StrategyBase::*strategy_get_name)() const = &StrategyBase::name;
void (StrategyBase::*strategy_set_name)(const string&) = &StrategyBase::name;

void export_Strategy() {
    class_<StrategyBaseWrap, boost::noncopyable>("StrategyBase", init<>())
      .add_property("name",
                    make_function(strategy_get_name, return_value_policy<copy_const_reference>()),
                    strategy_set_name)
      .def("start", &StrategyBase::start)
      .def("init", pure_virtual(&StrategyBase::init))
      .def("on_bar", pure_virtual(&StrategyBase::on_bar));
}