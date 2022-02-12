/*
 * _MarketInfo.cpp
 *
 *  Created on: 2012-9-27
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/MarketInfo_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_MarketInfo() {
    class_<MarketInfo>("MarketInfo", "市场信息记录", init<>())
      .def(init<const string&, const string&, const string&, const string&, const Datetime&,
                TimeDelta, TimeDelta, TimeDelta, TimeDelta>())
      //.def(self_ns::str(self))
      .def("__str__", &MarketInfo::toString)

      .add_property("market",
                    make_function(&MarketInfo::market, return_value_policy<copy_const_reference>()),
                    "市场标识（如：沪市“SH”, 深市“SZ”）")

      .add_property("name",
                    make_function(&MarketInfo::name, return_value_policy<copy_const_reference>()),
                    "市场全称")

      .add_property(
        "description",
        make_function(&MarketInfo::description, return_value_policy<copy_const_reference>()),
        "描述说明")

      .add_property("code",
                    make_function(&MarketInfo::code, return_value_policy<copy_const_reference>()),
                    "该市场对应的主要指数代码，用于获取交易日历")

      .add_property("last_datetime", &MarketInfo::lastDate, "该市场K线数据最后交易日期")
      .add_property("open_time1", &MarketInfo::openTime1, "开市时间1")
      .add_property("close_time1", &MarketInfo::closeTime1, "闭市时间1")
      .add_property("open_time2", &MarketInfo::openTime2, "开市时间2")
      .add_property("close_time2", &MarketInfo::closeTime2, "闭市时间2")

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<MarketInfo>())
#endif
      ;
}
