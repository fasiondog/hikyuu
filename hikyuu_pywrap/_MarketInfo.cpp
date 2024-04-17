/*
 * _MarketInfo.cpp
 *
 *  Created on: 2012-9-27
 *      Author: fasiondog
 */

#include <hikyuu/serialization/MarketInfo_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_MarketInfo(py::module& m) {
    py::class_<MarketInfo>(m, "MarketInfo", "市场信息记录")
      .def(py::init<>())
      .def(py::init<const string&, const string&, const string&, const string&, const Datetime&,
                    TimeDelta, TimeDelta, TimeDelta, TimeDelta>())

      .def("__str__", &MarketInfo::toString)
      .def("__repr__", &MarketInfo::toString)

      .def_property_readonly("market", py::overload_cast<>(&MarketInfo::market, py::const_),
                             py::return_value_policy::copy, "市场标识（如：沪市“SH”, 深市“SZ”）")
      .def_property_readonly("name", py::overload_cast<>(&MarketInfo::name, py::const_),
                             py::return_value_policy::copy, "市场全称")
      .def_property_readonly("description",
                             py::overload_cast<>(&MarketInfo::description, py::const_),
                             py::return_value_policy::copy, "描述说明")
      .def_property_readonly("code", py::overload_cast<>(&MarketInfo::code, py::const_),
                             py::return_value_policy::copy,
                             "该市场对应的主要指数代码，用于获取交易日历")

      .def_property_readonly("last_datetime", &MarketInfo::lastDate, "该市场K线数据最后交易日期")
      .def_property_readonly("open_time1", &MarketInfo::openTime1, "开市时间1")
      .def_property_readonly("close_time1", &MarketInfo::closeTime1, "闭市时间1")
      .def_property_readonly("open_time2", &MarketInfo::openTime2, "开市时间2")
      .def_property_readonly("close_time2", &MarketInfo::closeTime2, "闭市时间2")

        DEF_PICKLE(MarketInfo);
}
