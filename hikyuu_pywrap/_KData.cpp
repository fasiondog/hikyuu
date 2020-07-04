/*
 * _KData.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/KData_serialization.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_KData() {
    class_<KData>(
      "KData", "通过 Stock.getKData 获取的K线数据，由 KRecord 组成的数组，可象 list 一样进行遍历",
      init<>())
      //.def(self_ns::str(self)) //无法正常处理 utf8
      .def("__str__", &KData::toString)
      .def("__repr__", &KData::toString)
      .add_property("startPos", &KData::startPos,
                    "获取在原始K线记录中对应的起始位置，如果KData为空返回0")
      .add_property(
        "endPos", &KData::endPos,
        "获取在原始K线记录中对应范围的下一条记录的位置，如果为空返回0,其他等于lastPos + 1")
      .add_property("lastPos", &KData::lastPos,
                    "获取在原始K线记录中对应的最后一条记录的位置，如果为空返回0,其他等于endPos - 1")
      .add_property("open", &KData::open, "返回包含开盘价的 Indicator 实例，相当于 OPEN(k)")
      .add_property("close", &KData::close, "返回包含收盘价的 Indicator 实例，相当于 CLOSE(k)")
      .add_property("high", &KData::high, "返回包含最高价的 Indicator 实例，相当于 HIGH(k)")
      .add_property("low", &KData::low, "返回包含最低价的 Indicator 实例，相当于 LOW(k)")
      .add_property("amo", &KData::amo, "返回包含成交金额的 Indicator 实例，相当于 AMO(k)")
      .add_property("vol", &KData::vol, "返回包含成交量的 Indicator 实例，相当于 VOL(k)")

      .def("getDatetimeList", &KData::getDatetimeList, R"(getDatetimeList(self)

    返回交易日期列表

    :rtype: DatetimeList)")

      .def("get", &KData::getKRecord, R"(get(self, pos)

    获取指定索引位置的K线记录

    :param int pos: 位置索引
    :rtype: KRecord)")

      .def("getByDate", &KData::getKRecordByDate, R"(getByDate(self, datetime)    

    获取指定时间的K线记录。

    :param Datetime datetime: 指定的日期
    :rtype: KRecord)")

      .def("_getPos", &KData::getPos)  // python中需要将Null的情况改写为None

      .def("empty", &KData::empty, R"(empty(self)

    判断是否为空

    :rtype: bool)")

      .def("getQuery", &KData::getQuery, R"(getQuery(self)

    获取关联的查询条件

    :rtype: KQuery)")

      .def("getStock", &KData::getStock, R"(getStock(self)

    获取关联的Stock

    :rtype: Stock)")

      .def("tocsv", &KData::tocsv, R"(tocsv(self, filename)

    将数据保存至CSV文件

    :param str filename: 指定保存的文件名称)")

      .def("__len__", &KData::size)
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<KData>())
#endif
      ;
}
