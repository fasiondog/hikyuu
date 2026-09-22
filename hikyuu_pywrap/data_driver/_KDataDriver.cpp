/*
 * _KDataDriver.cpp
 *
 *  Created on: 2017-10-07
 *      Author: fasiondog
 */

#include "_KDataDriver.h"

using namespace hku;
namespace py = pybind11;

void export_KDataDriver(py::module& m) {
    py::class_<KDataDriver, KDataDriverPtr, PyKDataDriver>(m, "KDataDriver",
                                                           R"(The K-line data driver base class

  The subclass interfaces:
    - _init(self)
    - isIndexFirst(self) (Required)
    - canParallelLoad(self) (Required)
    - getCount(self, market, code, ktype)
    - _getIndexRangeByDate(self, market, code, query)
    - _getKRecordList(self, market, code, query)
    - _getTimeLineList(self, market, code, query)
    - _getTransList(self, market, code, query)
  )")
      .def(py::init<>())
      .def(py::init<const string&>())
      .def_property_readonly("name", &KDataDriver::name, py::return_value_policy::copy, "The driver name")

      .def("__str__", to_py_str<KDataDriver>)
      .def("__repr__", to_py_str<KDataDriver>)

      .def("get_param", &KDataDriver::getParam<boost::any>, "Get the value of the specified parameter")
      .def("set_param",
           static_cast<void (KDataDriver::*)(const std::string&, const boost::any&)>(
             &KDataDriver::setParam),
           "Set the parameter")
      .def("have_param", &KDataDriver::haveParam, "Whether the specified parameter exists")

      .def("clone", &KDataDriver::clone, "Clone the driver")

      .def("_init", &KDataDriver::_init, "[Subclass interface] Initialize the driver")
      .def("isIndexFirst", &KDataDriver::isIndexFirst,
           "[Subclass interface (Required)] Judge whether this engine is faster when querying by the position index way, or faster when querying by the date way")
      .def("canParallelLoad", &KDataDriver::canParallelLoad,
           "[Subclass interface (Required)] Whether the parallel data loading is supported")
      .def("getCount", &KDataDriver::getCount, py::arg("market"), py::arg("code"), py::arg("ktype"),
           R"(Get the amount of the K-line data of the specified type

    :param str market: the market abbreviation
    :param str code: the security code
    :param Query.KType ktype: the K-line type
    :rtype int)")
      .def(
        "_getIndexRangeByDate",
        [](KDataDriver& self, const string& market, const string& code, const KQuery& query) {
            size_t start = 0, end = 0;
            self.getIndexRangeByDate(market, code, query, start, end);
            return py::make_tuple(start, end);
        },
        py::arg("market"), py::arg("code"), py::arg("query"),
        R"([Subclass interface] Get the K-line record indexes corresponding to the specified date range

    :param str market: the market abbreviation
    :param str code: the security code
    :param KQuery query: the query condition
    :return: the (start, end) corresponding K-line record positions)")
      .def("_getKRecordList", &KDataDriver::getKRecordList, py::arg("market"), py::arg("code"),
           py::arg("query"), "[Subclass interface] Get the K-line data")
      .def("_getTimeLineList", &KDataDriver::getTimeLineList, py::arg("market"), py::arg("code"),
           py::arg("query"), "[Subclass interface] Get the time-line data")
      .def("_getTransList", &KDataDriver::getTransList, py::arg("market"), py::arg("code"),
           py::arg("query"), "[Subclass interface] Get the historical tick data")
      .def("isColumnFirst", &KDataDriver::isColumnFirst, "Whether it is column-first (the column database stores the K-line data)");
}