/*
 * _StockTypeInfo.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <hikyuu/serialization/StockTypeInfo_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_StockTypeInfo(py::module& m) {
    py::class_<StockTypeInfo>(m, "StockTypeInfo", "股票类型详情记录")
      .def(py::init<>())
      .def(py::init<uint32_t, const string&, price_t, price_t, int, double, double>())

      .def("__str__", &StockTypeInfo::toString)
      .def("__repr__", &StockTypeInfo::toString)

      .def_property_readonly("type", &StockTypeInfo::type, "证券类型")

      .def_property_readonly(
        "description", py::overload_cast<>(&StockTypeInfo::description, py::const_), "描述信息")

      .def_property_readonly("tick", &StockTypeInfo::tick, "最小跳动量")
      .def_property_readonly("tick_value", &StockTypeInfo::tickValue, "每一个tick价格")
      .def_property_readonly("unit", &StockTypeInfo::unit,
                             "每最小变动量价格，即单位价格 = tick_value/tick")
      .def_property_readonly("precision", &StockTypeInfo::precision, "价格精度")
      .def_property_readonly("min_trade_num", &StockTypeInfo::minTradeNumber, "每笔最小交易量")
      .def_property_readonly("max_trade_num", &StockTypeInfo::maxTradeNumber, "每笔最大交易量")

        DEF_PICKLE(StockTypeInfo);
}
