/*
 * _TradeCose.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/TradeCostBase.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyTradeCostBase : public TradeCostBase {
    PY_CLONE(PyTradeCostBase, TradeCostBase)

public:
    PyTradeCostBase() : TradeCostBase("PyTradeCostBase") {
        m_is_python_object = true;
    }

    PyTradeCostBase(const string& name) : TradeCostBase(name) {
        m_is_python_object = true;
    }

    CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                          double num) const override {
        PYBIND11_OVERLOAD_PURE(CostRecord, TradeCostBase, getBuyCost, datetime, stock, price, num);
    }

    CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                           double num) const override {
        PYBIND11_OVERLOAD_PURE(CostRecord, TradeCostBase, getSellCost, datetime, stock, price, num);
    }

    CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) const override {
        PYBIND11_OVERLOAD(CostRecord, TradeCostBase, getBorrowCashCost, datetime, cash);
    }

    CostRecord getReturnCashCost(const Datetime& borrow_datetime, const Datetime& return_datetime,
                                 price_t cash) const override {
        PYBIND11_OVERLOAD(CostRecord, TradeCostBase, getReturnCashCost, borrow_datetime,
                          return_datetime, cash);
    }

    CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override {
        PYBIND11_OVERLOAD(CostRecord, TradeCostBase, getBorrowStockCost, datetime, stock, price,
                          num);
    }

    CostRecord getReturnStockCost(const Datetime& borrow_datetime, const Datetime& return_datetime,
                                  const Stock& stock, price_t price, double num) const override {
        PYBIND11_OVERLOAD(CostRecord, TradeCostBase, getReturnStockCost, borrow_datetime,
                          return_datetime, stock, price, num);
    }
};

void export_TradeCost(py::module& m) {
    py::class_<TradeCostBase, TradeCostPtr, PyTradeCostBase>(m, "TradeCostBase",
                                                             R"(The trade cost algorithm base class

    The custom trade cost algorithm interfaces:

    :py:meth:`TradeCostBase.getBuyCost` - [Required] Get the buy cost
    :py:meth:`TradeCostBase.getSellCost` - [Required] Get the sell cost
    :py:meth:`TradeCostBase._clone` - [Required] The subclass clone interface)")

      .def(py::init<const string&>())

      .def("__str__", to_py_str<TradeCostBase>)
      .def("__repr__", to_py_str<TradeCostBase>)

      .def_property_readonly("name", &TradeCostBase::name, py::return_value_policy::copy,
                             "The cost algorithm name")

      .def("get_param", &TradeCostBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (TradeCostBase::*)(const std::string&, const boost::any&)>(
             &TradeCostBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("clone", &TradeCostBase::clone, "The clone operation")

      .def("get_buy_cost", &TradeCostBase::getBuyCost, py::arg("date"), py::arg("stock"),
           py::arg("price"), py::arg("num"),
           R"(get_buy_cost(self, date, stock, price, num)
    
        [Overload interface] Get the buy cost
        
        :param Datetime date: the buy moment
        :param Stock stock: the buy object
        :param float price: the buy price
        :param int num: the buy quantity
        :return: the trade cost record
        :rtype: CostRecord)")

      .def("get_sell_cost", &TradeCostBase::getSellCost, py::arg("date"), py::arg("stock"),
           py::arg("price"), py::arg("num"),
           R"(get_sell_cost(self, date, stock, price, num)
    
        [Overload interface] Get the sell cost
        
        :param Datetime date: the sell moment
        :param Stock stock: the sell object
        :param float price: the sell price
        :param int num: the sell quantity
        :return: the trade cost record
        :rtype: CostRecord)")

      //.def("getBorrowCashCost", &TradeCostBase::getBorrowCashCost,
      //     &TradeCostWrap::default_getBorrowCashCost)

      //.def("getReturnCashCost", &TradeCostBase::getReturnCashCost,
      //     &TradeCostWrap::default_getReturnCashCost)

      //.def("getBorrowStockCost", &TradeCostBase::getBorrowStockCost,
      //&TradeCostWrap::default_getBorrowStockCost) .def("getReturnStockCost",
      //&TradeCostBase::getReturnStockCost, &TradeCostWrap::default_getReturnStockCost)

      DEF_PICKLE(TradeCostPtr);
}