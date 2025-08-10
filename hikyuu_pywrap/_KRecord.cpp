/*
 * _KRecord.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <hikyuu/serialization/KRecord_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

bool (*krecord_eq)(const KRecord&, const KRecord&) = operator==;
bool (*krecord_ne)(const KRecord&, const KRecord&) = operator!=;

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

      .def("__eq__", krecord_eq)
      .def("__ne__", krecord_ne)

        DEF_PICKLE(KRecord);

    m.def("krecords_to_np", [](const KRecordList& kdata) {
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
        py::dtype dtype =
          py::dtype(vector_to_python_list<string>(
                      {"datetime", "open", "high", "low", "close", "amount", "volume"}),
                    vector_to_python_list<string>({"datetime64[ms]", "d", "d", "d", "d", "d", "d"}),
                    vector_to_python_list<int64_t>({0, 8, 16, 24, 32, 40, 48}), 56);

        return py::array(dtype, data.size(), data.data());
    });

    m.def("krecords_to_df", [](const KRecordList& kdata) {
        size_t total = kdata.size();
        if (total == 0) {
            return py::module_::import("pandas").attr("DataFrame")();
        }

        std::vector<int64_t> datetime;
        std::vector<double> open, high, low, close, amount, vol;

        datetime.resize(total);
        open.resize(total);
        high.resize(total);
        low.resize(total);
        close.resize(total);
        amount.resize(total);
        vol.resize(total);

        auto* ks = kdata.data();
        for (size_t i = 0, len = kdata.size(); i < len; i++) {
            datetime[i] = ks[i].datetime.timestamp() * 1000LL;
            open[i] = ks[i].openPrice;
            high[i] = ks[i].highPrice;
            low[i] = ks[i].lowPrice;
            close[i] = ks[i].closePrice;
            amount[i] = ks[i].transAmount;
            vol[i] = ks[i].transCount;
        }

        py::dict columns;
        columns["datetime"] =
          py::array_t<int64_t>(total, datetime.data()).attr("astype")("datetime64[ns]");
        columns["open"] = py::array_t<price_t>(total, open.data(), py::dtype("float64"));
        columns["high"] = py::array_t<price_t>(total, high.data(), py::dtype("float64"));
        columns["low"] = py::array_t<price_t>(total, low.data(), py::dtype("float64"));
        columns["close"] = py::array_t<price_t>(total, close.data(), py::dtype("float64"));
        columns["amount"] = py::array_t<price_t>(total, amount.data(), py::dtype("float64"));
        columns["volume"] = py::array_t<price_t>(total, vol.data(), py::dtype("float64"));
        return py::module_::import("pandas").attr("DataFrame")(columns);
    });
}
