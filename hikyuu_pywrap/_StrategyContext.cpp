/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/StrategyContext.h>

using namespace boost::python;
using namespace hku;

Datetime (StrategyContext::*get_start_datetime)() const = &StrategyContext::startDatetime;
void (StrategyContext::*set_start_datetime)(const Datetime&) = &StrategyContext::startDatetime;

void (StrategyContext::*set_stock_list)(const vector<string>&) = &StrategyContext::setStockCodeList;

void setStockList(StrategyContext* self, object seq) {
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

void setKTypeList(StrategyContext* self, object seq) {
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

void export_StrategeContext() {
    class_<StrategyContext>("StrategyContext", "策略上下文", init<>())
      .add_property("start_datetime", get_start_datetime, set_start_datetime, "起始日期")
      .add_property("stock_list",
                    make_function(&StrategyContext::getStockCodeList,
                                  return_value_policy<copy_const_reference>()),
                    setStockList, "股票代码列表")
      .add_property(
        "ktype_list",
        make_function(&StrategyContext::getKTypeList, return_value_policy<copy_const_reference>()),
        setKTypeList, "需要的K线类型");
}
