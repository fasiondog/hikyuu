/*
 * _Stock.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <hikyuu/serialization/Stock_serialization.h>
#include <hikyuu/KData.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

// BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getIndex_overloads, getIndex, 1, 2)

KRecord (Stock::*getKRecord1)(size_t pos, const KQuery::KType& kType) const = &Stock::getKRecord;
KRecord (Stock::*getKRecord2)(const Datetime&,
                              const KQuery::KType& kType) const = &Stock::getKRecord;

void export_Stock(py::module& m) {
    py::class_<Stock>(m, "Stock", "证券对象")
      .def(py::init<>())
      .def(py::init<const string&, const string&, const string&>(), py::arg("market"),
           py::arg("code"), py::arg("name"))
      .def(py::init<const Stock&>())

      .def("__str__", &Stock::toString)
      .def("__repr__", &Stock::toString)

      .def_property_readonly("id", &Stock::id, "内部id")
      .def_property_readonly("market", py::overload_cast<>(&Stock::market, py::const_),
                             "所属市场简称，市场简称是市场的唯一标识")
      .def_property_readonly("code", py::overload_cast<>(&Stock::code, py::const_), "证券代码")
      .def_property_readonly("market_code", py::overload_cast<>(&Stock::market_code, py::const_),
                             "市场简称+证券代码，如: sh000001")
      .def_property_readonly("name", py::overload_cast<>(&Stock::name, py::const_), "证券名称")
      .def_property_readonly("type", &Stock::type, "证券类型，参见：constant")
      .def_property_readonly("valid", &Stock::valid, "该证券当前是否有效")
      .def_property_readonly("start_datetime", &Stock::startDatetime, "证券起始日期")
      .def_property_readonly("last_datetime", &Stock::lastDatetime, "证券最后日期")
      .def_property_readonly("tick", &Stock::tick, "最小跳动量")
      .def_property_readonly("tick_value", &Stock::tickValue, "最小跳动量价值")
      .def_property_readonly("unit", &Stock::unit, "每单位价值 = tickValue / tick")
      .def_property_readonly("precision", &Stock::precision, "价格精度")
      .def_property_readonly("atom", &Stock::atom, "最小交易数量，同min_tradeNumber")
      .def_property_readonly("min_trade_number", &Stock::minTradeNumber, "最小交易数量")
      .def_property_readonly("max_trade_number", &Stock::maxTradeNumber, "最大交易数量")

      .def("is_null", &Stock::isNull, R"(is_null(self)

        是否为Null

        :rtype: bool)")

      .def("is_buffer", &Stock::isBuffer, R"(指定类型的K线数据是否被缓存)")

      .def("get_kdata", &Stock::getKData, R"(get_kdata(self, query)

        获取K线数据

        :param Query query: 查询条件
        :return: 满足查询条件的K线数据
        :rtype: KData)")

      .def("get_timeline_list", &Stock::getTimeLineList, R"(get_timeline_list(self, query)

        获取分时线

        :param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）
        :rtype: TimeLineList)")

      .def("get_trans_list", &Stock::getTransList, R"(get_trans_list(self, query)

        获取历史分笔数据

        :param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）
        :rtype: TransList)")

      .def("get_count", &Stock::getCount, py::arg("ktype") = KQuery::DAY,
           R"(get_count(self, [ktype=Query.DAY])

        获取不同类型K线数据量

        :param Query.KType ktype: K线数据类别
        :return: K线记录数
        :rtype: int)")

      .def("get_market_value", &Stock::getMarketValue, R"(get_market_value(self, date, ktype)

        获取指定时刻的市值，即小于等于指定时刻的最后一条记录的收盘价

        :param Datetime date: 指定时刻
        :param Query.KType ktype: K线数据类别
        :return: 指定时刻的市值
        :rtype: float)")

      .def("get_krecord", getKRecord1, py::arg("pos"), py::arg("ktype") = KQuery::DAY,
           R"(get_krecord(self, pos[, ktype=Query.DAY])

        获取指定索引的K线数据记录，未作越界检查

        :param int pos: 指定的索引位置
        :param Query.KType ktype: K线数据类别
        :return: K线记录
        :rtype: KRecord)")

      .def("get_krecord", getKRecord2, py::arg("date"), py::arg("ktype") = KQuery::DAY,
           R"(get_krecord(self, datetime[, ktype=Query.DAY])

        根据数据类型（日线/周线等），获取指定时刻的KRecord

        :param Datetime datetime: 指定日期时刻
        :param Query.KType ktype: K线数据类别
        :return: K线记录
        :rtype: KRecord)")

      .def("get_krecord_list", &Stock::getKRecordList, R"(get_krecord_list(self, start, end,
          ktype)

        获取K线记录 [start, end)，一般不直接使用.

        :param int start: 起始位置
        :param int end: 结束位置
        :param Query.KType ktype: K线类别
        :return: K线记录列表
        :rtype: KRecordList)")

      .def("get_datetime_list", &Stock::getDatetimeList, R"(get_datetime_list(self, query)

        获取日期列表

        :param Query query: 查询条件
        :rtype: DatetimeList)")

      .def("get_finance_info", &Stock::getFinanceInfo, R"(get_finance_info(self)

        获取当前财务信息

        :rtype: Parameter)")

      .def("get_history_finance_info", &Stock::getHistoryFinanceInfo,
           R"(get_history_finance_info(self, date)

        获取历史财务信息, 字段含义参见：https://hikyuu.org/finance_fields.html

        :param Datetime date: 指定日期必须是0331、0630、0930、1231，如 Datetime(201109300000)
        :rtype: list)")

      .def("realtime_update", &Stock::realtimeUpdate, py::arg("krecord"),
           py::arg("ktype") = KQuery::DAY,
           R"(realtime_update(self, krecord)

        只用于更新缓存中的日线数据

        :param KRecord krecord: 新增的实时K线记录
        :param KQuery.KType ktype: K 线类型)")

      .def("get_weight", &Stock::getWeight, py::arg("start") = Datetime::min(),
           py::arg("end") = Datetime(),
           R"(get_weight(self, [start, end])

        获取指定时间段[start,end)内的权息信息。未指定起始、结束时刻时，获取全部权息记录。

        :param Datetime start: 起始时刻
        :param Datetime end: 结束时刻
        :rtype: StockWeightList)")

      .def("load_kdata_to_buffer", &Stock::loadKDataToBuffer, R"(load_kdata_to_buffer(self,
          ktype)

        将指定类别的K线数据加载至内存缓存

        :param Query.KType ktype: K线类型)")

      .def("release_kdata_buffer", &Stock::releaseKDataBuffer, R"(release_kdata_buffer(self,
          ktype)

        释放指定类别的内存K线数据

        :param Query.KType ktype: K线类型)")

      .def(py::self == py::self)
      .def(py::self != py::self)

        DEF_PICKLE(Stock);
}
