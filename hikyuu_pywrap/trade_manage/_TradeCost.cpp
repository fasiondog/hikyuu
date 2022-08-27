/*
 * _TradeCose.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/TradeCostBase.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

class TradeCostWrap : public TradeCostBase, public wrapper<TradeCostBase> {
public:
    TradeCostWrap(const string& name) : TradeCostBase(name) {}

    CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                          double num) const {
        return this->get_override("getBuyCost")(datetime, stock, price, num);
    }

    CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                           double num) const {
        return this->get_override("getSellCost")(datetime, stock, price, num);
    }

    TradeCostPtr _clone() {
        return this->get_override("_clone")();
    }

    CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const {
        if (override getBorrowStockCost = get_override("getBorrowStockCost")) {
            return getBorrowStockCost(datetime, stock, price, num);
        }
        return TradeCostBase::getBorrowStockCost(datetime, stock, price, num);
    }

    CostRecord default_getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                          price_t price, double num) const {
        return this->TradeCostBase::getBorrowStockCost(datetime, stock, price, num);
    }

    CostRecord getReturnStockCost(const Datetime& borrow_datetime, const Datetime& return_datetime,
                                  const Stock& stock, price_t price, double num) const {
        if (override getReturnStockCost = get_override("getReturnStockCost")) {
            return getReturnStockCost(borrow_datetime, return_datetime, stock, price, num);
        }
        return TradeCostBase::getReturnStockCost(borrow_datetime, return_datetime, stock, price,
                                                 num);
    }

    CostRecord default_getReturnStockCost(const Datetime& borrow_datetime,
                                          const Datetime& return_datetime, const Stock& stock,
                                          price_t price, double num) const {
        return this->TradeCostBase::getReturnStockCost(borrow_datetime, return_datetime, stock,
                                                       price, num);
    }
};

void export_TradeCost() {
    class_<TradeCostWrap, boost::noncopyable>("TradeCostBase", R"(交易成本算法基类

    自定义交易成本算法接口：

    :py:meth:`TradeCostBase.getBuyCost` - 【必须】获取买入成本
    :py:meth:`TradeCostBase.getSellCost` - 【必须】获取卖出成本
    :py:meth:`TradeCostBase._clone` - 【必须】子类克隆接口)",

                                              init<const string&>())

      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property(
        "name", make_function(&TradeCostBase::name, return_value_policy<copy_const_reference>()),
        "成本算法名称")

      .def("get_param", &TradeCostBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &TradeCostBase::setParam<object>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("clone", &TradeCostBase::clone, "克隆操作")

      .def("get_buy_cost", pure_virtual(&TradeCostBase::getBuyCost),
           R"(get_buy_cost(self, datetime, stock, price, num)
    
        【重载接口】获取买入成本
        
        :param Datetime datetime: 买入时刻
        :param Stock stock: 买入对象
        :param float price: 买入价格
        :param int num: 买入数量
        :return: 交易成本记录
        :rtype: CostRecord)")

      .def("get_sell_cost", pure_virtual(&TradeCostBase::getSellCost),
           R"(get_sell_cost(self, datetime, stock, price, num)
    
        【重载接口】获取卖出成本
        
        :param Datetime datetime: 卖出时刻
        :param Stock stock: 卖出对象
        :param float price: 卖出价格
        :param int num: 卖出数量
        :return: 交易成本记录
        :rtype: CostRecord)")

      //.def("getBorrowStockCost", &TradeCostBase::getBorrowStockCost,
      //&TradeCostWrap::default_getBorrowStockCost) .def("getReturnStockCost",
      //&TradeCostBase::getReturnStockCost, &TradeCostWrap::default_getReturnStockCost)

      .def("_clone", pure_virtual(&TradeCostBase::_clone), "【重载接口】子类克隆接口")

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(name_init_pickle_suite<TradeCostBase>())
#endif
      ;
    register_ptr_to_python<TradeCostPtr>();
}
