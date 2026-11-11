/*
 * _TradeCose.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/tradecost/TradeCostBase.h>
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
        // OVERRIDE_NAME: the Python interface is the snake_case name documented in trade_cost.rst,
        // while the deprecated PYBIND11_OVERLOAD macros dispatch on the C++ name and ignore such
        // overrides
        PYBIND11_OVERRIDE_PURE_NAME(CostRecord, TradeCostBase, "get_buy_cost", getBuyCost, datetime,
                                    stock, price, num);
    }

    CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                           double num) const override {
        PYBIND11_OVERRIDE_PURE_NAME(CostRecord, TradeCostBase, "get_sell_cost", getSellCost,
                                    datetime, stock, price, num);
    }

    CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) const override {
        PYBIND11_OVERRIDE_NAME(CostRecord, TradeCostBase, "get_borrow_cash_cost", getBorrowCashCost,
                               datetime, cash);
    }

    CostRecord getReturnCashCost(const Datetime& borrow_datetime, const Datetime& return_datetime,
                                 price_t cash) const override {
        PYBIND11_OVERRIDE_NAME(CostRecord, TradeCostBase, "get_return_cash_cost", getReturnCashCost,
                               borrow_datetime, return_datetime, cash);
    }

    CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override {
        PYBIND11_OVERRIDE_NAME(CostRecord, TradeCostBase, "get_borrow_stock_cost",
                               getBorrowStockCost, datetime, stock, price, num);
    }

    CostRecord getReturnStockCost(const Datetime& borrow_datetime, const Datetime& return_datetime,
                                  const Stock& stock, price_t price, double num) const override {
        PYBIND11_OVERRIDE_NAME(CostRecord, TradeCostBase, "get_return_stock_cost",
                               getReturnStockCost, borrow_datetime, return_datetime, stock, price,
                               num);
    }

    double getBorrowCashRate(const Datetime& datetime) const override {
        PYBIND11_OVERRIDE_NAME(double, TradeCostBase, "get_borrow_cash_rate", getBorrowCashRate,
                               datetime);
    }

    double getBorrowStockRate(const Datetime& datetime, const Stock& stock) const override {
        PYBIND11_OVERRIDE_NAME(double, TradeCostBase, "get_borrow_stock_rate", getBorrowStockRate,
                               datetime, stock);
    }
};

void export_TradeCost(py::module& m) {
    py::class_<TradeCostBase, TradeCostPtr, PyTradeCostBase>(m, "TradeCostBase",
                                                             R"(The trade cost algorithm base class

    The custom trade cost algorithm interfaces:

    :py:meth:`TradeCostBase.get_buy_cost` - [Required] Get the buy cost
    :py:meth:`TradeCostBase.get_sell_cost` - [Required] Get the sell cost
    :py:meth:`TradeCostBase.get_borrow_cash_cost` - [Optional] Get the cost of borrowing cash
    :py:meth:`TradeCostBase.get_return_cash_cost` - [Optional] Get the cost of returning borrowed cash
    :py:meth:`TradeCostBase.get_borrow_stock_cost` - [Optional] Get the cost of borrowing stock
    :py:meth:`TradeCostBase.get_return_stock_cost` - [Optional] Get the cost of returning borrowed stock
    :py:meth:`TradeCostBase.get_borrow_cash_rate` - [Optional] Get the interest rate of the borrowed cash
    :py:meth:`TradeCostBase.get_borrow_stock_rate` - [Optional] Get the interest rate of the borrowed stock
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

      .def("get_borrow_cash_cost", &TradeCostBase::getBorrowCashCost, py::arg("date"),
           py::arg("cash"),
           R"(get_borrow_cash_cost(self, date, cash)

        [Overload interface] Get the cost of borrowing cash (margin financing)

        :param Datetime date: the borrow moment
        :param float cash: the borrowed cash
        :return: the trade cost record
        :rtype: CostRecord)")

      .def("get_return_cash_cost", &TradeCostBase::getReturnCashCost, py::arg("borrow_date"),
           py::arg("return_date"), py::arg("cash"),
           R"(get_return_cash_cost(self, borrow_date, return_date, cash)

        [Overload interface] Get the cost of returning borrowed cash

        :param Datetime borrow_date: the borrow moment
        :param Datetime return_date: the return moment
        :param float cash: the returned cash
        :return: the trade cost record
        :rtype: CostRecord)")

      .def("get_borrow_stock_cost", &TradeCostBase::getBorrowStockCost, py::arg("date"),
           py::arg("stock"), py::arg("price"), py::arg("num"),
           R"(get_borrow_stock_cost(self, date, stock, price, num)

        [Overload interface] Get the cost of borrowing stock (short selling)

        :param Datetime date: the borrow moment
        :param Stock stock: the borrowed stock
        :param float price: the borrow price
        :param int num: the borrowed quantity
        :return: the trade cost record
        :rtype: CostRecord)")

      .def("get_return_stock_cost", &TradeCostBase::getReturnStockCost, py::arg("borrow_date"),
           py::arg("return_date"), py::arg("stock"), py::arg("price"), py::arg("num"),
           R"(get_return_stock_cost(self, borrow_date, return_date, stock, price, num)

        [Overload interface] Get the cost of returning borrowed stock

        :param Datetime borrow_date: the borrow moment
        :param Datetime return_date: the return moment
        :param Stock stock: the returned stock
        :param float price: the return price
        :param int num: the returned quantity
        :return: the trade cost record
        :rtype: CostRecord)")

      .def("get_borrow_cash_rate", &TradeCostBase::getBorrowCashRate, py::arg("date"),
           R"(get_borrow_cash_rate(self, date)

        [Overload interface] Get the annualized interest rate of the borrowed cash

        The rate is billed by the daily settlement of the account, prorated by its annual_days
        parameter. Returning 0.0 means that nothing is billed, which is the default.

        :param Datetime date: the settled moment
        :return: an annualized decimal rate, e.g. 0.06 for 6% per year
        :rtype: float)")

      .def("get_borrow_stock_rate", &TradeCostBase::getBorrowStockRate, py::arg("date"),
           py::arg("stock"),
           R"(get_borrow_stock_rate(self, date, stock)

        [Overload interface] Get the annualized interest rate of the borrowed stock

        The interest of a securities lending is billed in cash on the market value of the borrowed
        shares at the settlement price. Returning 0.0 means that nothing is billed.

        :param Datetime date: the settled moment
        :param Stock stock: the borrowed stock
        :return: an annualized decimal rate, e.g. 0.10 for 10% per year
        :rtype: float)")

        DEF_PICKLE(TradeCostPtr);
}
