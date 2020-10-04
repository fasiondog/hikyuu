/*
 * _StockTypeInfo.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/StockTypeInfo_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_StockTypeInfo() {
    class_<StockTypeInfo>("StockTypeInfo", "股票类型详情记录", init<>())
      .def(init<uint32_t, const string&, price_t, price_t, int, double, double>())

      .def("__str__", &StockTypeInfo::toString)
      .def("__repr__", &StockTypeInfo::toString)

      .add_property("type", &StockTypeInfo::type, "证券类型")

      .add_property(
        "description",
        make_function(&StockTypeInfo::description, return_value_policy<copy_const_reference>()),
        "描述信息")

      .add_property("tick", &StockTypeInfo::tick, "最小跳动量")
      .add_property("tick_value", &StockTypeInfo::tickValue, "每一个tick价格")
      .add_property("unit", &StockTypeInfo::unit, "每最小变动量价格，即单位价格 = tick_value/tick")
      .add_property("precision", &StockTypeInfo::precision, "价格精度")
      .add_property("min_trade_num", &StockTypeInfo::minTradeNumber, "每笔最小交易量")
      .add_property("max_trade_num", &StockTypeInfo::maxTradeNumber, "每笔最大交易量")
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<StockTypeInfo>())
#endif
      ;
}
