/*
 * _Slippage.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/slippage/SlippageBase.h>
#include <hikyuu/trade_sys/slippage/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PySlippageBase : public SlippageBase {
    PY_CLONE(PySlippageBase, SlippageBase)

public:
    using SlippageBase::SlippageBase;

    void _calculate() override {
        PYBIND11_OVERLOAD_PURE(void, SlippageBase, _calculate, );
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, SlippageBase, _reset, );
    }

    price_t getRealBuyPrice(const Datetime& datetime, price_t planPrice) override {
        PYBIND11_OVERLOAD_PURE_NAME(price_t, SlippageBase, "get_real_buy_price", getRealBuyPrice,
                                    datetime, planPrice);
    }

    price_t getRealSellPrice(const Datetime& datetime, price_t planPrice) override {
        PYBIND11_OVERLOAD_PURE_NAME(price_t, SlippageBase, "get_real_sell_price", getRealSellPrice,
                                    datetime, planPrice);
    }
};

void export_Slippage(py::module& m) {
    py::class_<SlippageBase, SPPtr, PySlippageBase>(m, "SlippageBase", R"(移滑价差算法基类

自定义移滑价差接口：

    - getRealBuyPrice : 【必须】计算实际买入价格
    - getRealSellPrice : 【必须】计算实际卖出价格
    - _calculate : 【必须】子类计算接口
    - _clone : 【必须】克隆接口
    - _reset : 【可选】重载私有变量)")

      .def(py::init<>())
      .def(py::init<const string&>(), R"(初始化构造函数
        
    :param str name: 名称)")

      .def("__str__", to_py_str<SlippageBase>)
      .def("__repr__", to_py_str<SlippageBase>)

      .def_property("name", py::overload_cast<>(&SlippageBase::name, py::const_),
                    py::overload_cast<const string&>(&SlippageBase::name),
                    py::return_value_policy::copy, "名称")
      .def_property("to", &SlippageBase::getTO, &SlippageBase::setTO, "关联交易对象")

      .def("get_param", &SlippageBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &SlippageBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &SlippageBase::haveParam, "是否存在指定参数")

      .def("get_real_buy_price", &SlippageBase::getRealBuyPrice,
           R"(get_real_buy_price(self, datetime, price)

    【重载接口】计算实际买入价格

    :param Datetime datetime: 买入时间
    :param float price: 计划买入价格
    :return: 实际买入价格
    :rtype: float)")

      .def("get_real_sell_price", &SlippageBase::getRealSellPrice,
           R"(get_real_sell_price(self, datetime, price)

    【重载接口】计算实际卖出价格

    :param Datetime datetime: 卖出时间
    :param float price: 计划卖出价格
    :return: 实际卖出价格
    :rtype: float)")

      .def("reset", &SlippageBase::reset, "复位操作")
      .def("clone", &SlippageBase::clone, "克隆操作")
      .def("_calculate", &SlippageBase::_calculate, "【重载接口】子类计算接口")
      .def("_reset", &SlippageBase::_reset, "【重载接口】子类复位接口，复位内部私有变量")

        DEF_PICKLE(SPPtr);

    m.def("SP_FixedPercent", SP_FixedPercent, py::arg("p") = 0.001,
          R"(SP_FixedPercent([p=0.001])

    固定百分比移滑价差算法，买入实际价格 = 计划买入价格 * (1 + p)，卖出实际价格 = 计划卖出价格 * (1 - p)

    :param float p: 偏移的固定百分比
    :return: 移滑价差算法实例)");

    m.def("SP_FixedValue", SP_FixedValue, py::arg("value") = 0.01, R"(SP_FixedValuet([p=0.001])

    固定价格移滑价差算法，买入实际价格 = 计划买入价格 + 偏移价格，卖出实际价格 = 计划卖出价格 - 偏移价格

    :param float p: 偏移价格
    :return: 移滑价差算法实例)");
}
