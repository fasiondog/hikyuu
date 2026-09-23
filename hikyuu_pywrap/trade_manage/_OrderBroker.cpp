/*
 * _OrderBroker.cpp
 *
 *  Created on: 2017-06-28
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/OrderBrokerBase.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyOrderBrokerBase : public OrderBrokerBase {
public:
    using OrderBrokerBase::OrderBrokerBase;

    void _buy(Datetime datetime, const string& market, const string& code, price_t price,
              double num, price_t stoploss, price_t goalPrice, SystemPart from,
              const string& remark) override {
        PYBIND11_OVERLOAD_PURE(void, OrderBrokerBase, _buy, datetime, market, code, price, num,
                               stoploss, goalPrice, from, remark);
    }

    void _sell(Datetime datetime, const string& market, const string& code, price_t price,
               double num, price_t stoploss, price_t goalPrice, SystemPart from,
               const string& remark) override {
        PYBIND11_OVERLOAD_PURE(void, OrderBrokerBase, _sell, datetime, market, code, price, num,
                               stoploss, goalPrice, from, remark);
    }

    string _getAssetInfo() override {
        PYBIND11_OVERLOAD_NAME(string, OrderBrokerBase, "_get_asset_info", _getAssetInfo);
    }
};

void export_OrderBroker(py::module& m) {
    py::class_<BrokerPositionRecord>(m, "BrokerPositionRecord")
      .def(py::init<>())
      .def(py::init<const Stock&, price_t, price_t>())
      .def("__str__", &BrokerPositionRecord::str)
      .def("__repr__", &BrokerPositionRecord::str)
      .def_readwrite("stock", &BrokerPositionRecord::stock, "The position object")
      .def_readwrite("number", &BrokerPositionRecord::number, "The position quantity")
      .def_readwrite("money", &BrokerPositionRecord::money, "The total funds spent on buying");

    py::class_<OrderBrokerBase, OrderBrokerPtr, PyOrderBrokerBase>(
      m, "OrderBrokerBase",
      R"(The order broker wrapper base class; users can refer to it to customize their own order brokers, adding the extra processing
      
    :param bool real: whether to re-fetch the real-time tick data before placing the order
    :param float slip: if the absolute difference between the current ask price and the instructed buy price does not exceed slip, place the order, otherwise ignore; it is invalid for the sell operation, selling immediately at the current price)")

      .def(py::init<>())
      .def(py::init<const string&>(), R"(
    :param str name: the broker name)")

      .def("__str__", to_py_str<OrderBrokerBase>)
      .def("__repr__", to_py_str<OrderBrokerBase>)

      .def_property("name", py::overload_cast<>(&OrderBrokerBase::name, py::const_),
                    py::overload_cast<const string&>(&OrderBrokerBase::name),
                    py::return_value_policy::copy, "The name (readable and writable)")

      .def("buy", &OrderBrokerBase::buy, "For the details, see the subclass implementation interface: _buy")
      .def("sell", &OrderBrokerBase::sell, "For the details, see the subclass implementation interface: _sell")
      .def("get_asset_info", &OrderBrokerBase::getAssetInfo, "For the details, see the subclass implementation interface: _get_asset_info")

      .def(
        "_buy", &OrderBrokerBase::_buy,
        R"(_buy(self, datetime, market, code, price, num, stoploss, goal_price, part_from, remark)

    [Subclass interface] Execute the buy operation

    :param Datetime datetime: the strategy instruction time
    :param str market: the market identifier
    :param str code: the security code
    :param float price: the buy price
    :param float num: the buy quantity
    :param float stoploss: the planned stop-loss price
    :param float goal_price: the planned profit target price
    :param SystemPart part_from: the signal source,
    :param str remark: the order remark)")

      .def(
        "_sell", &OrderBrokerBase::_sell,
        R"(_sell(self, datetime, market, code, price, num, stoploss, goal_price, part_from, remark)

    [Subclass interface] Execute the sell operation

    :param Datetime datetime: the strategy instruction time
    :param str market: the market identifier
    :param str code: the security code
    :param float price: the sell price
    :param float num: the sell quantity
    :param float stoploss: the planned stop-loss price
    :param float goal_price: the planned profit target price
    :param SystemPart part_from: the signal source
    :param str remark: the order remark)")

      .def("_get_asset_info", &OrderBrokerBase::_getAssetInfo, R"(_get_asset_info(self)

    [Subclass interface] Get the current asset information; the subclass needs to return a json string conforming to the following specification:

    {
        "datetime": "2001-01-01 18:00:00.12345",
        "cash": 0.0,
        "positions": [
            {"market": "SZ", "code": "000001", "number": 100.0, "stoploss": 0.0, "goal_price": 0.0,
             "cost_price": 0.0},
            {"market": "SH", "code": "600001", "number": 100.0, "stoploss": 0.0, "goal_price": 0.0,
             "cost_price": 0.0},
         ]
    }    

    :return: return the current asset information as a string (in json format)
    :rtype: str)");
}
