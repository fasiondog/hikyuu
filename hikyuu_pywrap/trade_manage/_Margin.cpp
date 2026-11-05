/*
 * _Margin.cpp
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/margin/MarginBase.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyMarginBase : public MarginBase {
    PY_CLONE(PyMarginBase, MarginBase)

public:
    PyMarginBase() : MarginBase("PyMarginBase") {
        m_is_python_object = true;
    }

    PyMarginBase(const string& name) : MarginBase(name) {
        m_is_python_object = true;
    }

    bool isMarginTrading(const Stock& stock) const override {
        PYBIND11_OVERRIDE_NAME(bool, MarginBase, "is_margin_trading", isMarginTrading, stock);
    }

    bool needBorrowAsset(const Stock& stock) const override {
        PYBIND11_OVERRIDE_NAME(bool, MarginBase, "need_borrow_asset", needBorrowAsset, stock);
    }

    price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price, double num,
                          MarginSide side) const override {
        PYBIND11_OVERRIDE_PURE_NAME(price_t, MarginBase, "get_open_margin", getOpenMargin, datetime,
                                    stock, price, num, side);
    }

    price_t getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                              double num, MarginSide side) const override {
        PYBIND11_OVERRIDE_NAME(price_t, MarginBase, "get_maintain_margin", getMaintainMargin,
                               datetime, stock, price, num, side);
    }

    double getMarginRate(const Datetime& datetime, const Stock& stock,
                         MarginSide side) const override {
        PYBIND11_OVERRIDE_NAME(double, MarginBase, "get_margin_rate", getMarginRate, datetime,
                               stock, side);
    }

    double getMaxOpenNumber(const Datetime& datetime, const Stock& stock, price_t price,
                            price_t available, MarginSide side) const override {
        PYBIND11_OVERRIDE_NAME(double, MarginBase, "get_max_open_number", getMaxOpenNumber,
                               datetime, stock, price, available, side);
    }
};

void export_Margin(py::module& m) {
    py::enum_<MarginSide>(m, "MarginSide", "The side of an opened position")
      .value("LONG", MarginSide::LONG, "Long side")
      .value("SHORT", MarginSide::SHORT, "Short side")
      .export_values();

    py::class_<MarginBase, MarginPtr, PyMarginBase>(m, "MarginBase",
                                                    R"(The margin model base class

    The custom margin model interfaces:

    :py:meth:`MarginBase.getOpenMargin` - [Required] Get the margin occupied by an opening
    :py:meth:`MarginBase.isMarginTrading` - [Optional] Whether the security uses the margin accounting
    :py:meth:`MarginBase.needBorrowAsset` - [Optional] Whether a short opening needs borrowing
    :py:meth:`MarginBase.getMaintainMargin` - [Optional] Get the maintain margin
    :py:meth:`MarginBase.getMarginRate` - [Optional] The ratio view of the margin
    :py:meth:`MarginBase.getMaxOpenNumber` - [Optional] The max openable quantity
    :py:meth:`MarginBase._clone` - [Required] The subclass clone interface)")

      .def(py::init<const string&>())

      .def("__str__", to_py_str<MarginBase>)
      .def("__repr__", to_py_str<MarginBase>)

      .def_property_readonly("name", &MarginBase::name, py::return_value_policy::copy,
                             "The margin model name")

      .def("get_param", &MarginBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (MarginBase::*)(const std::string&, const boost::any&)>(
             &MarginBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &MarginBase::haveParam, "Whether the specified parameter exists")

      .def("clone", &MarginBase::clone, "The clone operation")

      .def("is_margin_trading", &MarginBase::isMarginTrading, py::arg("stock"),
           R"(is_margin_trading(self, stock)

        [Overload interface] Whether the security is covered by the margin accounting

        :param Stock stock: the security to check
        :return: True if the position occupies margin
        :rtype: bool)")

      .def("need_borrow_asset", &MarginBase::needBorrowAsset, py::arg("stock"),
           R"(need_borrow_asset(self, stock)

        [Overload interface] Whether opening a short of the security needs borrowing the asset

        :param Stock stock: the security to check
        :return: True if the short opening borrows the security
        :rtype: bool)")

      .def("get_open_margin", &MarginBase::getOpenMargin, py::arg("date"), py::arg("stock"),
           py::arg("price"), py::arg("num"), py::arg("side") = MarginSide::LONG,
           R"(get_open_margin(self, date, stock, price, num, side=MarginSide.LONG)

        [Overload interface] Get the margin occupied by an opening

        :param Datetime date: the trade moment
        :param Stock stock: the security to open
        :param float price: the opening price
        :param float num: the opening quantity
        :param MarginSide side: the long or the short side
        :return: the occupied funds, 0 for invalid inputs
        :rtype: float)")

      .def("get_maintain_margin", &MarginBase::getMaintainMargin, py::arg("date"), py::arg("stock"),
           py::arg("price"), py::arg("num"), py::arg("side") = MarginSide::LONG,
           R"(get_maintain_margin(self, date, stock, price, num, side=MarginSide.LONG)

        [Overload interface] Get the maintain margin, i.e. the additional-margin threshold

        :param Datetime date: the trade moment
        :param Stock stock: the security to open
        :param float price: the opening price
        :param float num: the opening quantity
        :param MarginSide side: the long or the short side
        :return: the maintain margin
        :rtype: float)")

      .def("get_margin_rate", &MarginBase::getMarginRate, py::arg("date"), py::arg("stock"),
           py::arg("side") = MarginSide::LONG,
           R"(get_margin_rate(self, date, stock, side=MarginSide.LONG)

        [Overload interface] The ratio view of the margin, a value in (0, 1]

        :param Datetime date: the trade moment
        :param Stock stock: the security
        :param MarginSide side: the long or the short side
        :return: the margin ratio, 1.0 when there is no leverage
        :rtype: float)")

      .def("get_max_open_number", &MarginBase::getMaxOpenNumber, py::arg("date"), py::arg("stock"),
           py::arg("price"), py::arg("available"), py::arg("side") = MarginSide::LONG,
           R"(get_max_open_number(self, date, stock, price, available, side=MarginSide.LONG)

        [Overload interface] Get the maximum openable quantity under the available funds

        :param Datetime date: the trade moment
        :param Stock stock: the security to open
        :param float price: the opening price
        :param float available: the available funds
        :param MarginSide side: the long or the short side
        :return: the quantity aligned to the minimum trading unit, 0 when nothing can be opened
        :rtype: float)")

        DEF_PICKLE(MarginPtr);
}
