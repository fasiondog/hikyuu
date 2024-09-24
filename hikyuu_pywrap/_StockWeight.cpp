/*
 * StockWeight.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <hikyuu/serialization/StockWeight_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_StockWeight(py::module& m) {
    py::class_<StockWeight>(m, "StockWeight", "权息记录")
      .def(py::init<>())
      .def(py::init<const Datetime&>())
      .def(py::init<const Datetime&, price_t, price_t, price_t, price_t, price_t, price_t, price_t,
                    price_t>())

      .def("__str__", to_py_str<StockWeight>)
      .def("__repr__", to_py_str<StockWeight>)

      .def_property_readonly("datetime", &StockWeight::datetime, "权息日期")
      .def_property_readonly("count_as_gift", &StockWeight::countAsGift, "每10股送X股")
      .def_property_readonly("count_for_sell", &StockWeight::countForSell, "每10股配X股")
      .def_property_readonly("price_for_sell", &StockWeight::priceForSell, "配股价")
      .def_property_readonly("bonus", &StockWeight::bonus, "每10股红利")
      .def_property_readonly("increasement", &StockWeight::increasement, "每10股转增X股")
      .def_property_readonly("total_count", &StockWeight::totalCount, "总股本（万股）")
      .def_property_readonly("free_count", &StockWeight::freeCount, "流通股（万股）")
      .def_property_readonly("suogu", &StockWeight::suogu, "扩缩股比例")

        DEF_PICKLE(StockWeight);
}
