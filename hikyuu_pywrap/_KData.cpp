/*
 * _KData.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <hikyuu/serialization/KData_serialization.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

const KRecord& (KData::*KData_getKRecord1)(size_t pos) const = &KData::getKRecord;
const KRecord& (KData::*KData_getKRecord2)(Datetime datetime) const = &KData::getKRecord;

void export_KData(py::module& m) {
    py::class_<KData>(
      m, "KData",
      "通过 Stock.getKData 获取的K线数据，由 KRecord 组成的数组，可象 list 一样进行遍历")
      .def(py::init<>())
      .def("__str__", &KData::toString)
      .def("__repr__", &KData::toString)

      .def_property_readonly("start_pos", &KData::startPos,
                             "获取在原始K线记录中对应的起始位置，如果KData为空返回0")
      .def_property_readonly(
        "end_pos", &KData::endPos,
        "获取在原始K线记录中对应范围的下一条记录的位置，如果为空返回0,其他等于lastPos + 1")
      .def_property_readonly(
        "last_pos", &KData::lastPos,
        "获取在原始K线记录中对应的最后一条记录的位置，如果为空返回0,其他等于endPos - 1")

      .def_property_readonly("open", &KData::open,
                             "返回包含开盘价的 Indicator 实例，相当于 OPEN(k)")
      .def_property_readonly("close", &KData::close,
                             "返回包含收盘价的 Indicator 实例，相当于 CLOSE(k)")
      .def_property_readonly("high", &KData::high,
                             "返回包含最高价的 Indicator 实例，相当于 HIGH(k)")
      .def_property_readonly("low", &KData::low, "返回包含最低价的 Indicator 实例，相当于 LOW(k)")
      .def_property_readonly("amo", &KData::amo, "返回包含成交金额的 Indicator 实例，相当于 AMO(k)")
      .def_property_readonly("vol", &KData::vol, "返回包含成交量的 Indicator 实例，相当于 VOL(k)")

      .def("get_datetime_list", &KData::getDatetimeList, R"(get_datetime_list(self)

        返回交易日期列表

        :rtype: DatetimeList)")

      .def("get", KData_getKRecord1, py::return_value_policy::copy, R"(get(self, pos)

        获取指定索引位置的K线记录

        :param int pos: 位置索引
        :rtype: KRecord)")

      .def("get_by_datetime", KData_getKRecord2, py::return_value_policy::copy,
           R"(get_by_datetime(self, datetime)

        获取指定时间的K线记录。

        :param Datetime datetime: 指定的日期
        :rtype: KRecord)")

      .def(
        "get_pos",
        [](const KData& self, const Datetime& d) {
            size_t pos = self.getPos(d);
            py::object ret = py::none();
            if (pos != Null<size_t>()) {
                ret = py::int_(pos);
            }
            return ret;
        },
        R"(get_pos(self, datetime)

        获取指定时间的K线记录的索引位置, 如果不在数据范围内，则返回 None
        
        :param Datetime datetime: 指定的日期
        :rtype: int)")

      .def(
        "get_pos_in_stock",
        [](const KData& self, Datetime datetime) {
            size_t pos = self.getPosInStock(datetime);
            py::object ret = py::none();
            if (pos != Null<size_t>()) {
                ret = py::int_(pos);
            }
            return ret;
        },
        R"(get_pos_in_stock(self, datetime) 
        
        获取指定时间对应的原始K线中的索引位置

        :param Datetime datetime: 指定的时间
        :return: 对应的索引位置，如果不在数据范围内，则返回 None)")

      .def("empty", &KData::empty, R"(empty(self)

        判断是否为空

        :rtype: bool)")

      .def("get_query", &KData::getQuery, py::return_value_policy::copy, R"(get_query(self)

        获取关联的查询条件

        :rtype: KQuery)")

      .def("get_stock", &KData::getStock, py::return_value_policy::copy, R"(get_stock(self)

        获取关联的Stock

        :rtype: Stock)")

      .def("get_kdata", &KData::getKData, R"(get_kdata(self, start_date, end_date)
      
        通过当前 KData 获取一个保持数据类型、复权类型不变的新的 KData（注意，不是原 KData 的子集）

        :param Datetime start: 新的起始日期
        :param Datetime end: 新的结束日期
        :rtype: KData)")

      .def("tocsv", &KData::tocsv, R"(tocsv(self, filename)

        将数据保存至CSV文件

        :param str filename: 指定保存的文件名称)")

      .def("__len__", &KData::size)

      .def(py::self == py::self)
      .def(py::self != py::self)

      .def("__getitem__",
           [](const KData& self, py::object obj) {
               py::object ret;
               if (py::isinstance<py::int_>(obj)) {
                   int64_t i = obj.cast<int64_t>();
                   int64_t length = self.size();
                   int64_t index = i < 0 ? length + i : i;
                   if (index < 0 || index >= length)
                       throw std::out_of_range(fmt::format("index out of range: {}", i));
                   ret = py::cast(self.getKRecord(index));
                   return ret;
               } else if (py::isinstance<Datetime>(obj)) {
                   Datetime dt = py::cast<Datetime>(obj);
                   auto krecord = self.getKRecord(dt);
                   if (krecord == Null<KRecord>()) {
                       throw std::out_of_range(fmt::format("datetime out of range: {}", dt));
                   }
                   ret = py::cast(krecord);
                   return ret;
               } else if (py::isinstance<py::str>(obj)) {
                   Datetime dt = Datetime(py::cast<std::string>(obj));
                   auto krecord = self.getKRecord(dt);
                   if (krecord == Null<KRecord>()) {
                       throw std::out_of_range(fmt::format("datetime out of range: {}", dt));
                   }
                   ret = py::cast(krecord);
                   return ret;
               } else if (py::isinstance<py::slice>(obj)) {
                   py::slice slice = py::cast<py::slice>(obj);
                   size_t start, stop, step, length;

                   if (!slice.compute(self.size(), &start, &stop, &step, &length)) {
                       throw std::invalid_argument("无效的切片参数");
                   }

                   KRecordList result;
                   result.reserve(length);
                   for (size_t i = 0; i < length; ++i) {
                       size_t index = start + i * step;
                       result.push_back(self[static_cast<size_t>(index)]);
                   }

                   ret = py::cast(result);
                   return ret;
               }

               throw std::out_of_range("Error index type");
           })

      .def(
        "__iter__",
        [](const KData& self) {
            return py::make_iterator<py::return_value_policy::reference_internal>(self.cbegin(),
                                                                                  self.cend());
        },
        py::keep_alive<0, 1>())

      .def(
        "to_np",
        [](const KData& kdata) {
            struct RawData {
                int64_t datetime;  // 转换后的毫秒时间戳
                double open;
                double high;
                double low;
                double close;
                double amount;
                double volume;
            };
            std::vector<RawData> data;
            data.resize(kdata.size());

            py::dtype dtype;
            auto minutes = KQuery::getKTypeInMin(kdata.getQuery().kType());
            if (minutes >= KQuery::getKTypeInMin(KQuery::DAY)) {
                for (size_t i = 0, total = kdata.size(); i < total; i++) {
                    const KRecord& k = kdata[i];
                    data[i].datetime = (k.datetime - Datetime(1970, 1, 1)).days();
                    data[i].open = k.openPrice;
                    data[i].high = k.highPrice;
                    data[i].low = k.lowPrice;
                    data[i].close = k.closePrice;
                    data[i].amount = k.transAmount;
                    data[i].volume = k.transCount;
                }

                // 定义NumPy结构化数据类型
                dtype = py::dtype(
                  vector_to_python_list<string>(
                    {"datetime", "open", "high", "low", "close", "amount", "volume"}),
                  vector_to_python_list<string>({"datetime64[D]", "d", "d", "d", "d", "d", "d"}),
                  vector_to_python_list<int64_t>({0, 8, 16, 24, 32, 40, 48}), 56);
            } else {
                for (size_t i = 0, total = kdata.size(); i < total; i++) {
                    const KRecord& k = kdata[i];
                    data[i].datetime = k.datetime.timestamp() / 1000LL;
                    data[i].open = k.openPrice;
                    data[i].high = k.highPrice;
                    data[i].low = k.lowPrice;
                    data[i].close = k.closePrice;
                    data[i].amount = k.transAmount;
                    data[i].volume = k.transCount;
                }

                // 定义NumPy结构化数据类型
                dtype = py::dtype(
                  vector_to_python_list<string>(
                    {"datetime", "open", "high", "low", "close", "amount", "volume"}),
                  vector_to_python_list<string>({"datetime64[ms]", "d", "d", "d", "d", "d", "d"}),
                  vector_to_python_list<int64_t>({0, 8, 16, 24, 32, 40, 48}), 56);
            }

            return py::array(dtype, data.size(), data.data());
        },
        "将 KData 转换为 NumPy 数组")

        DEF_PICKLE(KData);
}
