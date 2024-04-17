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

    Datetime _buy(Datetime datetime, const string& market, const string& code, price_t price,
                  double num) override {
        PYBIND11_OVERLOAD_PURE(Datetime, OrderBrokerBase, _buy, datetime, market, code, price, num);
    }

    Datetime _sell(Datetime datetime, const string& market, const string& code, price_t price,
                   double num) override {
        PYBIND11_OVERLOAD_PURE(Datetime, OrderBrokerBase, _sell, datetime, market, code, price,
                               num);
    }
};

void export_OrderBroker(py::module& m) {
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

      .def("buy", &OrderBrokerBase::buy, R"(buy(self, datetime, market, code, price, num)

    执行买入操作

    :param Datetime datetime: 策略指示时间
    :param str market: 市场标识
    :param str code: 证券代码
    :param float price: 买入价格
    :param float num: 买入数量
    :return: 操作执行的时刻。实盘时，应返回委托单时间或服务器交易时间。
    :rtype: Datetime)")

      .def("sell", &OrderBrokerBase::sell, R"(sell(self, datetime, market, code, price, num)

    执行卖出操作

    :param Datetime datetime: 策略指示时间
    :param str market: 市场标识
    :param str code: 证券代码
    :param float price: 卖出价格
    :param float num: 卖出数量
    :return: 操作执行的时刻。实盘时，应返回委托单时间或服务器交易时间。
    "rtype: Datetime)")

      .def("_buy", &OrderBrokerBase::_buy,
           R"(_buy(self, datetime, market, code, price, num)

    【子类接口】执行买入操作

    :param Datetime datetime: 策略指示时间
    :param str market: 市场标识
    :param str code: 证券代码
    :param float price: 买入价格
    :param float num: 买入数量
    :return: 操作执行的时刻。实盘时，应返回委托单时间或服务器交易时间。
    :rtype: Datetime)")

      .def("_sell", &OrderBrokerBase::_sell,
           R"(_sell(self, datetime, market, code, price, num)

    【子类接口】执行卖出操作

    :param Datetime datetime: 策略指示时间
    :param str market: 市场标识
    :param str code: 证券代码
    :param float price: 卖出价格
    :param float num: 卖出数量
    :return: 操作执行的时刻。实盘时，应返回委托单时间或服务器交易时间。
    "rtype: Datetime)");
}
