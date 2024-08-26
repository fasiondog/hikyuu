/*
 * _OrderBroker.cpp
 *
 *  Created on: 2017年6月28日
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
              double num, price_t stoploss, price_t goalPrice, SystemPart from) override {
        PYBIND11_OVERLOAD_PURE(void, OrderBrokerBase, _buy, datetime, market, code, price, num,
                               stoploss, goalPrice, from);
    }

    void _sell(Datetime datetime, const string& market, const string& code, price_t price,
               double num, price_t stoploss, price_t goalPrice, SystemPart from) override {
        PYBIND11_OVERLOAD_PURE(void, OrderBrokerBase, _sell, datetime, market, code, price, num,
                               stoploss, goalPrice, from);
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
      .def_readwrite("stock", &BrokerPositionRecord::stock, "持仓对象")
      .def_readwrite("number", &BrokerPositionRecord::number, "持仓数量")
      .def_readwrite("money", &BrokerPositionRecord::money, "买入花费总资金");

    py::class_<OrderBrokerBase, OrderBrokerPtr, PyOrderBrokerBase>(
      m, "OrderBrokerBase",
      R"(订单代理包装基类，用户可以参考自定义自己的订单代理，加入额外的处理
      
    :param bool real: 下单前是否重新实时获取实时分笔数据
    :param float slip: 如果当前的卖一价格和指示买入的价格绝对差值不超过slip则下单，否则忽略; 对卖出操作无效，立即以当前价卖出)")

      .def(py::init<>())
      .def(py::init<const string&>(), R"(
    :param str name: 代理名称)")

      .def("__str__", to_py_str<OrderBrokerBase>)
      .def("__repr__", to_py_str<OrderBrokerBase>)

      .def_property("name", py::overload_cast<>(&OrderBrokerBase::name, py::const_),
                    py::overload_cast<const string&>(&OrderBrokerBase::name),
                    py::return_value_policy::copy, "名称（可读写）")

      .def("buy", &OrderBrokerBase::buy, "详情见子类实现接口: _buy")
      .def("sell", &OrderBrokerBase::sell, "详情见子类实现接口: _sell")
      .def("get_asset_info", &OrderBrokerBase::getAssetInfo, "详情见子类实现接口: _get_asset_info")

      .def("_buy", &OrderBrokerBase::_buy,
           R"(_buy(self, datetime, market, code, price, num, stoploss, goal_price, part_from)

    【子类接口】执行买入操作

    :param Datetime datetime: 策略指示时间
    :param str market: 市场标识
    :param str code: 证券代码
    :param float price: 买入价格
    :param float num: 买入数量
    :param float stoploss: 计划止损价
    :param float goal_price: 计划盈利目标价
    :param SystemPart part_from: 信号来源)")

      .def("_sell", &OrderBrokerBase::_sell,
           R"(_sell(self, datetime, market, code, price, num, stoploss, goal_price, part_from)

    【子类接口】执行卖出操作

    :param Datetime datetime: 策略指示时间
    :param str market: 市场标识
    :param str code: 证券代码
    :param float price: 卖出价格
    :param float num: 卖出数量
    :param float stoploss: 计划止损价
    :param float goal_price: 计划盈利目标价
    :param SystemPart part_from: 信号来源)")

      .def("_get_asset_info", &OrderBrokerBase::_getAssetInfo, R"(_get_asset_info(self)

    【子类接口】获取当前资产信息，子类需返回符合如下规范的 json 字符串:

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

    :return: 以字符串（json格式）方式返回当前资产信息
    :rtype: str)");
}
