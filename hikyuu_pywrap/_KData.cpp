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
    int64_t null_int64 = Null<int64_t>();
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

      .def("get_kdata", py::overload_cast<const KQuery::KType&>(&KData::getKData, py::const_),
           py::arg("ktype"), R"(get_kdata(self, ktype
           
        获取相同时间范围内的其他类型K线数据，如日线下对应的分钟线数据

        :param KQuery::KType ktype: 指定需要的K线类型)")

      .def("get_kdata",
           py::overload_cast<const Datetime&, const Datetime&>(&KData::getKData, py::const_),
           R"(get_kdata(self, start_date, end_date)
      
        通过当前 KData 获取一个保持数据类型、复权类型不变的新的 KData（注意，不是原 KData 的子集）

        :param Datetime start: 新的起始日期
        :param Datetime end: 新的结束日期
        :rtype: KData)")

      .def("get_kdata", py::overload_cast<const KQuery&>(&KData::getKData, py::const_),
           R"(get_kdata(query)

        通过当前 KData 获取获取另一个 KData，不一定是其子集

        :rtype: KData)")

      .def("get_sub_kdata", &KData::getSubKData, py::arg("start"), py::arg("end") = null_int64,
           R"(get_sub_kdata(start, end = Null<int64_t>)

        通过索引获取自身子集

        :param int start: 起始索引
        :param int end: 结束索引
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
                   if (!krecord.isValid()) {
                       throw std::out_of_range(fmt::format("datetime out of range: {}", dt));
                   }
                   ret = py::cast(krecord);
                   return ret;
               } else if (py::isinstance<py::str>(obj)) {
                   Datetime dt = Datetime(py::cast<std::string>(obj));
                   auto krecord = self.getKRecord(dt);
                   if (!krecord.isValid()) {
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
            size_t total = kdata.size();
            HKU_IF_RETURN(total == 0, py::array());

            struct RawData {
                int64_t datetime;  // 转换后的毫秒时间戳
                double open;
                double high;
                double low;
                double close;
                double amount;
                double volume;
            };

            RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
            for (size_t i = 0; i < total; i++) {
                const KRecord& k = kdata[i];
                data[i].datetime = k.datetime.timestamp() * 1000LL;
                data[i].open = k.openPrice;
                data[i].high = k.highPrice;
                data[i].low = k.lowPrice;
                data[i].close = k.closePrice;
                data[i].amount = k.transAmount;
                data[i].volume = k.transCount;
            }

            // 定义NumPy结构化数据类型
            auto dtype = py::dtype(
              vector_to_python_list<string>(
                {"datetime", "open", "high", "low", "close", "amount", "volume"}),
              vector_to_python_list<string>({"datetime64[ns]", "d", "d", "d", "d", "d", "d"}),
              vector_to_python_list<int64_t>({0, 8, 16, 24, 32, 40, 48}), 56);

            return py::array(dtype, total, static_cast<RawData*>(data),
                             py::capsule(data, [](void* p) { std::free(p); }));
        },
        "将 KData 转换为 NumPy 数组")

      .def(
        "to_df",
        [](const KData& self, bool with_stock) {
            size_t total = self.size();
            if (total == 0) {
                return py::module_::import("pandas").attr("DataFrame")();
            }

            // 创建数组
            py::array_t<int64_t> datetime_arr(total);
            py::array_t<double> open_arr(total);
            py::array_t<double> high_arr(total);
            py::array_t<double> low_arr(total);
            py::array_t<double> close_arr(total);
            py::array_t<double> amount_arr(total);
            py::array_t<double> vol_arr(total);

            // 获取缓冲区并填充数据
            auto datetime_buf = datetime_arr.request();
            auto open_buf = open_arr.request();
            auto high_buf = high_arr.request();
            auto low_buf = low_arr.request();
            auto close_buf = close_arr.request();
            auto amount_buf = amount_arr.request();
            auto vol_buf = vol_arr.request();

            int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
            double* open_ptr = static_cast<double*>(open_buf.ptr);
            double* high_ptr = static_cast<double*>(high_buf.ptr);
            double* low_ptr = static_cast<double*>(low_buf.ptr);
            double* close_ptr = static_cast<double*>(close_buf.ptr);
            double* amount_ptr = static_cast<double*>(amount_buf.ptr);
            double* vol_ptr = static_cast<double*>(vol_buf.ptr);

            auto* ks = self.data();
            for (size_t i = 0; i < total; i++) {
                datetime_ptr[i] = ks[i].datetime.timestamp() * 1000LL;
                open_ptr[i] = ks[i].openPrice;
                high_ptr[i] = ks[i].highPrice;
                low_ptr[i] = ks[i].lowPrice;
                close_ptr[i] = ks[i].closePrice;
                amount_ptr[i] = ks[i].transAmount;
                vol_ptr[i] = ks[i].transCount;
            }

            // 构建 DataFrame
            auto pandas = py::module_::import("pandas");
            py::dict columns;
            if (with_stock) {
                py::list code_list(total);
                py::list name_list(total);
                auto stk = self.getStock();
                auto code = py::str(stk.code());
                auto name = py::str(stk.name());
                for (size_t i = 0; i < total; i++) {
                    code_list[i] = code;
                    name_list[i] = name;
                }
                columns["market_code"] =
                  pandas.attr("Series")(code_list, py::arg("dtype") = "string");
                columns["name"] = pandas.attr("Series")(name_list, py::arg("dtype") = "string");
            }

            columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
            columns["open"] = open_arr;
            columns["high"] = high_arr;
            columns["low"] = low_arr;
            columns["close"] = close_arr;
            columns["amount"] = amount_arr;
            columns["volume"] = vol_arr;

            return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
        },
        py::arg("with_stock") = false, R"(to_df(self, with_stock=False) -> pandas.DataFrame

    转化为pandas的DataFrame
        
    :param bool with_stock: 包含Stock的代码与名称
    :rtype: pandas.DataFrame)")

        DEF_PICKLE(KData);
}
