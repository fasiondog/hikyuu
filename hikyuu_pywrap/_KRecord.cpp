/*
 * _KRecord.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <hikyuu/serialization/KRecord_serialization.h>
#include "df_to_ks.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_KReord(py::module& m) {
    py::class_<KRecord>(m, "KRecord", "K线记录，组成K线数据，属性可读写")
      .def(py::init<>())
      .def(py::init<const Datetime&>())
      .def(py::init<const Datetime&, price_t, price_t, price_t, price_t, price_t, price_t>())

      .def("__str__", to_py_str<KRecord>)
      .def("__repr__", to_py_str<KRecord>)

      .def_readwrite("datetime", &KRecord::datetime, "时间")
      .def_readwrite("open", &KRecord::openPrice, "开盘价")
      .def_readwrite("high", &KRecord::highPrice, "最高价")
      .def_readwrite("low", &KRecord::lowPrice, "最低价")
      .def_readwrite("close", &KRecord::closePrice, "收盘价")
      .def_readwrite("amount", &KRecord::transAmount, "成交金额")
      .def_readwrite("volume", &KRecord::transCount, "成交量")

      .def("is_valid", &KRecord::isValid, "KRecord是否有效")

      .def(py::self == py::self)
      .def(py::self != py::self)

        DEF_PICKLE(KRecord);

    m.def("krecords_to_np", [](const KRecordList& kdata) {
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
            data[i].datetime = k.datetime.timestamp() / 1000LL;
            data[i].open = k.openPrice;
            data[i].high = k.highPrice;
            data[i].low = k.lowPrice;
            data[i].close = k.closePrice;
            data[i].amount = k.transAmount;
            data[i].volume = k.transCount;
        }

        // 定义NumPy结构化数据类型
        py::dtype dtype =
          py::dtype(vector_to_python_list<string>(
                      {"datetime", "open", "high", "low", "close", "amount", "volume"}),
                    vector_to_python_list<string>({"datetime64[ms]", "d", "d", "d", "d", "d", "d"}),
                    vector_to_python_list<int64_t>({0, 8, 16, 24, 32, 40, 48}), 56);

        return py::array(dtype, total, static_cast<RawData*>(data),
                         py::capsule(data, [](void* p) { std::free(p); }));
    });

    m.def("krecords_to_df", [](const KRecordList& kdata) {
        size_t total = kdata.size();
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

        auto* ks = kdata.data();
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
        py::dict columns;
        columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
        columns["open"] = open_arr;
        columns["high"] = high_arr;
        columns["low"] = low_arr;
        columns["close"] = close_arr;
        columns["amount"] = amount_arr;
        columns["volume"] = vol_arr;

        return py::module_::import("pandas").attr("DataFrame")(columns, py::arg("copy") = false);
    });

    m.def("df_to_krecords", df_to_krecords,
          R"(df_to_krecords(df: pd.DataFrame[, columns: dict]) -> KRecordList
          
    将DataFrame转换为KRecordList, 必须按顺序指定列名，默认为: ("datetime", "open", "high", "low", "close", "amount", "volume")

    :param DataFrame df: 输入的DataFrame
    :param dict columns: 指定DataFrame的列名，对应KRecord的成员变量名称
    :return: 转换后的KRecordList)",
          py::arg("df"),
          py::arg("columns") =
            StringList{"datetime", "open", "high", "low", "close", "amount", "volume"});
}
