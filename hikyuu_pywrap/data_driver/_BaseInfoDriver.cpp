/*
 * _BaseInfoDriver.cpp
 *
 *  Created on: 2017-10-07
 *      Author: fasiondog
 */

#include "_BaseInfoDriver.h"

using namespace hku;
namespace py = pybind11;

static string BaseInfoDriver_to_str(const BaseInfoDriver& v) {
    std::stringstream out;
    out << v;
    return out.str();
}

void export_BaseInfoDriver(py::module& m) {
    py::class_<BaseInfoDriver, BaseInfoDriverPtr, PyBaseInfoDriver>(m, "BaseInfoDriver",
                                                                    R"(The basic information data driver base class

    The subclass interfaces:
        - _init(self) (Required)
        - getAllStockInfo(self) (Required)
        - getStockInfo(self, market, code) (Required)
        - getMarketInfo(self, market) (Required)
        - getAllMarketInfo(self) (Required)
        - getAllStockTypeInfo(self) (Required)
        - getStockTypeInfo(self, type) (Required)
        - getAllHolidays(self) (Required)
        - getAllZhBond10(self) (Required)
        - getStockWeightList(self, market, code, start, end)
        - getAllStockWeightList(self)
        - getHistoryFinance(self, market, code, start, end)
        - getHistoryFinanceField(self)
        - getFinanceInfo(self, market, code)
    )")
      .def(py::init<const string&>(), R"(Initialize

    :param str name: the driver name)")
      .def_property_readonly("name", &BaseInfoDriver::name, py::return_value_policy::copy,
                             "The driver name")
      .def("__str__", BaseInfoDriver_to_str)
      .def("__repr__", BaseInfoDriver_to_str)

      .def("get_param", &BaseInfoDriver::getParam<boost::any>, "Get the specified parameter")
      .def("set_param",
           static_cast<void (BaseInfoDriver::*)(const std::string&, const boost::any&)>(
             &BaseInfoDriver::setParam),
           "Set the specified parameter")
      .def("have_param", &BaseInfoDriver::haveParam, "Whether the specified parameter exists")

      .def("_init", &BaseInfoDriver::_init, "[Subclass interface (Required)] Initialize the driver")
      .def("getAllStockInfo", &BaseInfoDriver::getAllStockInfo,
           "[Subclass interface (Required)] Get the detailed information of all the stocks")
      .def("getStockInfo", &BaseInfoDriver::getStockInfo, py::arg("market"), py::arg("code"),
           R"([Subclass interface (Required)] Get the specified security information

    :param str market: the market abbreviation
    :param str code: the security code)")
      .def("getStockWeightList", &BaseInfoDriver::getStockWeightList, py::arg("market"),
           py::arg("code"), py::arg("start"), py::arg("end"),
           R"(Get the dividend list within the specified date range [start, end)

    :param str market: the market abbreviation
    :param str code: the security code
    :param Datetime start: the start date
    :param Datetime end: the end date)")
      .def("getAllStockWeightList", &BaseInfoDriver::getAllStockWeightList,
           "Get the dividend lists of all the stocks")
      .def("getHistoryFinance", &BaseInfoDriver::getHistoryFinance, py::arg("market"),
           py::arg("code"), py::arg("start"), py::arg("end"),
           R"(Get the historical finance information

    :param str market: the market abbreviation
    :param str code: the security code
    :param Datetime start: the start date of the finance report publishing
    :param Datetime end: the end date of the query)")
      .def("getHistoryFinanceField", &BaseInfoDriver::getHistoryFinanceField,
           "Get the indexes and the names of the historical finance information fields")
      .def("getFinanceInfo", &BaseInfoDriver::getFinanceInfo, py::arg("market"), py::arg("code"),
           R"(Get the current finance information

    :param str market: the market identifier
    :param str code: the security code)")
      .def("getMarketInfo", &BaseInfoDriver::getMarketInfo, py::arg("market"),
           R"([Subclass interface (Required)] Get the specified MarketInfo

    :param str market: the market abbreviation
    :return: if it is not found, return Null<MarketInfo>())")
      .def("getAllMarketInfo", &BaseInfoDriver::getAllMarketInfo,
           "[Subclass interface (Required)] Get all the market information")
      .def("getAllStockTypeInfo", &BaseInfoDriver::getAllStockTypeInfo,
           "[Subclass interface (Required)] Get all the security type information")
      .def("getStockTypeInfo", &BaseInfoDriver::getStockTypeInfo, py::arg("type"),
           R"([Subclass interface (Required)] Get the detailed information of the corresponding security type

    :param int type: the security type
    :return: the corresponding security type information; if it does not exist, return Null<StockTypeInfo>())")
      .def("getAllHolidays", &BaseInfoDriver::getAllHolidays,
           "[Subclass interface (Required)] Get all the holiday dates")
      .def("getAllZhBond10", &BaseInfoDriver::getAllZhBond10,
           "[Subclass interface (Required)] Get all the Chinese 10-year treasury bond information");
}