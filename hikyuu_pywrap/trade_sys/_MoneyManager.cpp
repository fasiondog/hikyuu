/*
 * _MoneyManager.cpp
 *
 *  Created on: 2013-3-13
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/moneymanager/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyMoneyManagerBase : public MoneyManagerBase {
    PY_CLONE(PyMoneyManagerBase, MoneyManagerBase)

public:
    PyMoneyManagerBase() : MoneyManagerBase() {
        m_is_python_object = true;
    }

    PyMoneyManagerBase(const string& name) : MoneyManagerBase(name) {
        m_is_python_object = true;
    }

    PyMoneyManagerBase(const MoneyManagerBase& base) : MoneyManagerBase(base) {
        m_is_python_object = true;
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, MoneyManagerBase, _reset, );
    }

    void _buyNotify(const TradeRecord& tr) override {
        PYBIND11_OVERLOAD_NAME(void, MoneyManagerBase, "_buy_notify", _buyNotify, tr);
    }

    void _sellNotify(const TradeRecord& tr) override {
        PYBIND11_OVERLOAD_NAME(void, MoneyManagerBase, "_sell_notify", _sellNotify, tr);
    }

    double _getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price, price_t risk,
                         SystemPart from) override {
        PYBIND11_OVERLOAD_PURE_NAME(double, MoneyManagerBase, "_get_buy_num", _getBuyNumber,
                                    datetime, stock, price, risk, from);
    }

    double _getSellNumber(const Datetime& datetime, const Stock& stock, price_t price, price_t risk,
                          SystemPart from) override {
        PYBIND11_OVERLOAD_NAME(double, MoneyManagerBase, "_get_sell_num", _getSellNumber, datetime,
                               stock, price, risk, from);
    }

    double _getSellShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                               price_t risk, SystemPart from) override {
        PYBIND11_OVERLOAD_NAME(double, MoneyManagerBase, "_get_sell_short_num", _getSellShortNumber,
                               datetime, stock, price, risk, from);
    }

    double _getBuyShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                              price_t risk, SystemPart from) override {
        PYBIND11_OVERLOAD_NAME(double, MoneyManagerBase, "_get_buy_short_num", _getBuyShortNumber,
                               datetime, stock, price, risk, from);
    }
};

void export_MoneyManager(py::module& m) {
    py::class_<MoneyManagerBase, MMPtr, PyMoneyManagerBase>(m, "MoneyManagerBase",
                                                            py::dynamic_attr(),
                                                            R"(The money manager strategy base class

Common parameters:

    - auto-checkin=False (bool): when the account cash is insufficient to buy the quantity indicated by the money manager strategy, automatically deposit (checkin) enough cash into the account.
    - max-stock=20000 (int): the maximum number of the kinds of the held securities (i.e. how many stocks are held, not the position size of each stock)
    - disable_ev_force_clean_position=False (bool): disable forcibly clearing the positions when the market environment becomes invalid
    - disable_cn_force_clean_position=False (bool): disable forcibly clearing the positions when the system valid condition becomes invalid

The custom money manager strategy interfaces:

    - _buyNotify : [Optional] Receive the actual buy notification, reserved for the multiple position increase/decrease processing
    - _sellNotify : [Optional] Receive the actual sell notification, reserved for the multiple position increase/decrease processing
    - _getBuyNumber : [Required] Get the quantity that can be bought of the specified trading object
    - _getSellNumber : [Optional] Get the quantity that can be sold of the specified trading object; if it is not overloaded, default to selling all the held quantity
    - _reset : [Optional] Reset the private attributes
    - _clone : [Required] The clone interface)")
      .def(py::init<>())
      .def(py::init<const MoneyManagerBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor
        
    :param str name: the name)")

      .def("__str__", to_py_str<MoneyManagerBase>)
      .def("__repr__", to_py_str<MoneyManagerBase>)

      .def_property("name", py::overload_cast<>(&MoneyManagerBase::name, py::const_),
                    py::overload_cast<const string&>(&MoneyManagerBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("tm", &MoneyManagerBase::getTM, &MoneyManagerBase::setTM,
                    "Set or get the trade manager object")
      .def_property("query", &MoneyManagerBase::getQuery, &MoneyManagerBase::setQuery,
                    py::return_value_policy::copy, "Set or get the query condition")

      .def("current_buy_count", &MoneyManagerBase::currentBuyCount, "The current consecutive buy count")
      .def("current_sell_count", &MoneyManagerBase::currentSellCount, "The current consecutive sell count")

      .def("get_param", &MoneyManagerBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (MoneyManagerBase::*)(const std::string&, const boost::any&)>(
             &MoneyManagerBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &MoneyManagerBase::haveParam, "Whether the specified parameter exists")

      .def("reset", &MoneyManagerBase::reset, "The reset operation")
      .def("clone", &MoneyManagerBase::clone, "The clone operation")

      .def("_buy_notify", &MoneyManagerBase::_buyNotify,
           R"(_buy_notify(self, trade_record)

    [Overload interface] When the trading system performs the actual buy operation, notify the trade changes; it only needs to be overloaded when there are multiple position increases/decreases

    :param TradeRecord trade_record: the actual buy trade record when the actual buying occurs)")

      .def("_sell_notify", &MoneyManagerBase::_sellNotify,
           R"(_sell_notify(self, trade_record)

    [Overload interface] When the trading system performs the actual sell operation, notify the actual trade changes; it only needs to be overloaded when there are multiple position increases/decreases

    :param TradeRecord trade_record: the actual sell trade record when the actual selling occurs)")

      .def("get_buy_num", &MoneyManagerBase::getBuyNumber,
           R"(get_buy_num(self, datetime, stock, price, risk, part_from)

    Get the quantity that can be bought of the specified trading object

    :param Datetime datetime: the trading time
    :param Stock stock: the trading object
    :param float price: the trading price
    :param float risk: the risk taken in the trade; if it is 0, it means a total loss, i.e. the market value falls to 0 yuan
    :param System.Part part_from: the source system part
    :return: the quantity that can be bought
    :rtype: float)")

      .def("get_sell_num", &MoneyManagerBase::getSellNumber,
           R"(get_sell_num(self, datetime, stock, price, risk, part_from)

    Get the quantity that can be sold of the specified trading object
    
    :param Datetime datetime: the trading time
    :param Stock stock: the trading object
    :param float price: the trading price
    :param float risk: the new risk taken in the trade; if it is 0, it means a total loss, i.e. the market value falls to 0 yuan
    :param System.Part part_from: the source system part
    :return: the quantity that can be sold
    :rtype: float)")

      .def("_get_buy_num", &MoneyManagerBase::_getBuyNumber,
           R"(_get_buy_num(self, datetime, stock, price, risk, part_from)

    [Overload interface] Get the quantity that can be bought of the specified trading object

    :param Datetime datetime: the trading time
    :param Stock stock: the trading object
    :param float price: the trading price
    :param float risk: the risk taken in the trade; if it is 0, it means a total loss, i.e. the market value falls to 0 yuan
    :param System.Part part_from: the source system part
    :return: the quantity that can be bought
    :rtype: float)")

      .def("_get_sell_num", &MoneyManagerBase::_getSellNumber,
           R"(_get_sell_num(self, datetime, stock, price, risk, part_from)

    [Overload interface] Get the quantity that can be sold of the specified trading object. If it is not overloaded, default to selling all the held quantity.

    :param Datetime datetime: the trading time
    :param Stock stock: the trading object
    :param float price: the trading price
    :param float risk: the new risk taken in the trade; if it is 0, it means a total loss, i.e. the market value falls to 0 yuan
    :param System.Part part_from: the source system part
    :return: the quantity that can be sold
    :rtype: float)")

      .def("get_sell_short_num", &MoneyManagerBase::getSellShortNumber)
      .def("get_buy_short_num", &MoneyManagerBase::getBuyShortNumber)
      .def("_get_sell_short_num", &MoneyManagerBase::_getSellShortNumber)
      .def("_get_buy_short_num", &MoneyManagerBase::_getBuyShortNumber)

      .def("_reset", &MoneyManagerBase::_reset, R"([Overload interface] The subclass reset interface, resetting the internal private variables)")

        DEF_PICKLE(MMPtr);

    m.def("MM_Nothing", MM_Nothing, R"(MM_Nothing()

    A special money manager strategy, equivalent to no money management; buy as much as the money available.)");

    m.def("MM_FixedRisk", MM_FixedRisk, py::arg("risk") = 1000.00,
          R"(MM_FixedRisk([risk = 1000.00])

    The fixed risk money manager strategy limits each trade to a pre-determined or fixed capital risk, e.g. a fixed risk of 1000 yuan per trade. The formula: the trading quantity = the fixed risk / the trading risk.

    :param float risk: the fixed risk
    :return: the money manager strategy instance)");

    m.def("MM_FixedCapital", MM_FixedCapital, py::arg("capital") = 10000.00,
          R"(MM_FixedCapital([capital = 10000.0])

    The fixed capital money manager strategy. The buy quantity = the current cash / capital

    :param float capital: the fixed capital unit
    :return: the money manager strategy instance)");

    m.def("MM_FixedCapitalFunds", MM_FixedCapitalFunds, py::arg("capital") = 10000.00,
          R"(MM_FixedCapitalFunds([capital = 10000.0]) 

    The fixed total capital money manager strategy. The buy quantity = the current total assets / capital
  
    :param float capital: the fixed capital unit
    :return: the money manager strategy instance)");

    m.def("MM_FixedCount", MM_FixedCount, py::arg("n") = 100, R"(MM_FixedCount([n = 100])

    The fixed trading quantity money manager strategy. Buy a fixed quantity each time.
    
    :param float n: the quantity to buy each time (it should be an integer multiple of the minimum trading quantity of the trading object; the program does not check this here)
    :return: the money manager strategy instance)");

    m.def("MM_FixedPercent", MM_FixedPercent, py::arg("p") = 0.03, R"(MM_FixedPercent([p = 0.03])

    The fixed percentage risk model. The formula: P (the position size) = the account balance * the percentage / R (the trading risk per share). [BOOK3]_, [BOOK4]_ .
    
    :param float p: the percentage
    :return: the money manager strategy instance)");

    m.def("MM_FixedUnits", MM_FixedUnits, py::arg("n") = 33, R"(MM_FixedUnits([n = 33])

    The fixed unit money manager strategy. The formula: the buy quantity = the current cash / n / the current risk

    :param int n: n capital units
    :return: the money manager strategy instance)");

    m.def("MM_WilliamsFixedRisk", MM_WilliamsFixedRisk, py::arg("p") = 0.1,
          py::arg("max_loss") = 1000.0,
          R"( MM_WilliamsFixedRisk([p=0.1, max_loss=1000.0])

    The Williams fixed risk money manager strategy. The buy quantity = (the account balance × the risk percentage p) ÷ the maximum loss (max_loss)

    :param float p: the risk percentage
    :param float max_loss: the maximum loss
    :return: the money manager strategy instance)");

    m.def("MM_FixedCountTps", MM_FixedCountTps, py::arg("buy_counts"), py::arg("sell_counts"),
          R"(MM_FixedCountTps([buy_counts, sell_counts])
          
    The consecutive buy/sell fixed quantity money manager strategy.
    
    :param list buy_counts: the buy quantity list
    :param list sell_counts: the sell quantity list
    :return: the money manager strategy instance)");

    // Note: the portfolio-level fund allocation (AF_*) has been moved out to _AllocateFunds.cpp
    // (AllocateFundsBase).
}