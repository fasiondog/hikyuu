/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#include <hikyuu/StrategyContext.h>
#include "pybind_utils.h"

namespace py = pybind11;
using namespace hku;

Datetime (StrategyContext::*get_start_datetime)() const = &StrategyContext::startDatetime;
void (StrategyContext::*set_start_datetime)(const Datetime&) = &StrategyContext::startDatetime;

void (StrategyContext::*set_stock_list)(const vector<string>&) = &StrategyContext::setStockCodeList;

void setStockList(StrategyContext* self, const py::sequence& seq) {
    vector<string> stk_list = python_list_to_vector<string>(seq);
    self->setStockCodeList(std::move(stk_list));
}

void setKTypeList(StrategyContext* self, const py::sequence& seq) {
    vector<string> stk_list = python_list_to_vector<string>(seq);
    self->setKTypeList(stk_list);
}

void export_StrategeContext(py::module& m) {
    py::class_<StrategyContext>(m, "StrategyContext", "策略上下文")
      .def(py::init<>())
      .def_property("start_datetime", get_start_datetime, set_start_datetime, "起始日期")
      .def_property("stock_list",
                    py::overload_cast<>(&StrategyContext::getStockCodeList, py::const_),
                    setStockList, "股票代码列表")
      .def_property("ktype_list",
                    py::overload_cast<>(&StrategyContext::getKTypeList, py::const_),
                    setKTypeList, "需要的K线类型");
}
