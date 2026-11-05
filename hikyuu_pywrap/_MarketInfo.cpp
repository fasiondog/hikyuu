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
    py::class_<MarketInfo>(m, "MarketInfo", "The market information record")
      .def(py::init<>())
      .def(py::init<const string&, const string&, const string&, const string&, const Datetime&,
                    TimeDelta, TimeDelta, TimeDelta, TimeDelta, int, int, int>(),
           py::arg("market"), py::arg("name"), py::arg("description"), py::arg("code"),
           py::arg("last_datetime"), py::arg("open_time1"), py::arg("close_time1"),
           py::arg("open_time2"), py::arg("close_time2"),
           py::arg("tz_offset_min") = 480, py::arg("dst_rule") = 0,
           py::arg("data_lag") = 0)

      .def("__str__", &MarketInfo::toString)
      .def("__repr__", &MarketInfo::toString)

      .def_property_readonly("market", py::overload_cast<>(&MarketInfo::market, py::const_),
                             py::return_value_policy::copy, "The market identifier (e.g.: the Shanghai market 'SH', the Shenzhen market 'SZ')")
      .def_property_readonly("name", py::overload_cast<>(&MarketInfo::name, py::const_),
                             py::return_value_policy::copy, "The full name of the market")
      .def_property_readonly("description",
                             py::overload_cast<>(&MarketInfo::description, py::const_),
                             py::return_value_policy::copy, "The description")
      .def_property_readonly("code", py::overload_cast<>(&MarketInfo::code, py::const_),
                             py::return_value_policy::copy,
                             "The main index code corresponding to this market, used to get the trading calendar")

      .def_property_readonly("last_datetime", &MarketInfo::lastDate, "The last trading date of the K-line data of this market")
      .def_property_readonly("open_time1", &MarketInfo::openTime1, "The open time 1")
      .def_property_readonly("close_time1", &MarketInfo::closeTime1, "The close time 1")
      .def_property_readonly("open_time2", &MarketInfo::openTime2, "The open time 2")
      .def_property_readonly("close_time2", &MarketInfo::closeTime2, "The close time 2")
      .def_property_readonly("tz_offset_min", &MarketInfo::tzOffsetMin,
                             "Standard UTC offset in minutes (e.g. 480 for UTC+8)")
      .def_property_readonly("dst_rule", &MarketInfo::dstRule,
                             "Daylight saving rule: 0=none, 1=US, 2=EU")
      .def_property_readonly("data_lag", &MarketInfo::dataLag,
                             "Visibility delay in trading days relative to the base market")

        DEF_PICKLE(MarketInfo);
}
