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
    py::class_<StockTypeInfo>(m, "StockTypeInfo", "The stock type detail record")
      .def(py::init<>())
      .def(py::init<uint32_t, const string&, price_t, price_t, int, double, double>())

      .def("__str__", &StockTypeInfo::toString)
      .def("__repr__", &StockTypeInfo::toString)

      .def_property_readonly("type", &StockTypeInfo::type, "The security type")

      .def_property_readonly(
        "description", py::overload_cast<>(&StockTypeInfo::description, py::const_), "The description information")

      .def_property_readonly("tick", &StockTypeInfo::tick, "The minimum tick")
      .def_property_readonly("tick_value", &StockTypeInfo::tickValue, "The price of each tick")
      .def_property_readonly("unit", &StockTypeInfo::unit,
                             "The price of each minimum change, i.e. the unit price = tick_value/tick")
      .def_property_readonly("precision", &StockTypeInfo::precision, "The price precision")
      .def_property_readonly("min_trade_num", &StockTypeInfo::minTradeNumber, "The minimum trading quantity per order")
      .def_property_readonly("max_trade_num", &StockTypeInfo::maxTradeNumber, "The maximum trading quantity per order")

        DEF_PICKLE(StockTypeInfo);
}
