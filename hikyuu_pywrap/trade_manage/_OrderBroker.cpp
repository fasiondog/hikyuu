/*
 * _OrderBroker.cpp
 *
 *  Created on: 2017年6月28日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/OrderBrokerBase.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class OrderBrokerWrap : public OrderBrokerBase, public wrapper<OrderBrokerBase> {
public:
    OrderBrokerWrap() : OrderBrokerBase() {}
    OrderBrokerWrap(const string& name) : OrderBrokerBase(name) {}

    Datetime _buy(Datetime datetime, const string& market, const string& code, price_t price,
                  double num) {
        return this->get_override("_buy")(datetime, market, code, price, num);
    }

    Datetime _sell(Datetime datetime, const string& market, const string& code, price_t price,
                   double num) {
        return this->get_override("_sell")(datetime, market, code, price, num);
    }
};

string (OrderBrokerBase::*ob_get_name)() const = &OrderBrokerBase::name;
void (OrderBrokerBase::*ob_set_name)(const string&) = &OrderBrokerBase::name;

void export_OrderBroker() {
    class_<OrderBrokerWrap, boost::noncopyable>(
      "OrderBrokerBase",
      R"(订单代理包装基类，用户可以参考自定义自己的订单代理，加入额外的处理
      
    :param bool real: 下单前是否重新实时获取实时分笔数据
    :param float slip: 如果当前的卖一价格和指示买入的价格绝对差值不超过slip则下单，否则忽略; 对卖出操作无效，立即以当前价卖出)")

      .def(init<>())
      .def(init<const string&>())

      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property("name", ob_get_name, ob_set_name, "代理名称")

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

      .def("_buy", pure_virtual(&OrderBrokerBase::_buy),
           R"(_buy(self, datetime, market, code, price, num)

    【子类接口】执行买入操作

    :param Datetime datetime: 策略指示时间
    :param str market: 市场标识
    :param str code: 证券代码
    :param float price: 买入价格
    :param float num: 买入数量
    :return: 操作执行的时刻。实盘时，应返回委托单时间或服务器交易时间。
    :rtype: Datetime)")

      .def("_sell", pure_virtual(&OrderBrokerBase::_sell),
           R"(_sell(self, datetime, market, code, price, num)

    【子类接口】执行卖出操作

    :param Datetime datetime: 策略指示时间
    :param str market: 市场标识
    :param str code: 证券代码
    :param float price: 卖出价格
    :param float num: 卖出数量
    :return: 操作执行的时刻。实盘时，应返回委托单时间或服务器交易时间。
    "rtype: Datetime)");

    register_ptr_to_python<OrderBrokerPtr>();
}
