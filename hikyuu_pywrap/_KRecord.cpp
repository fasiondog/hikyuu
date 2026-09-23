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
    py::class_<KRecord>(m, "KRecord", "The K-line record, composing the K-line data; the attributes are readable and writable")
      .def(py::init<>())
      .def(py::init<const Datetime&>())
      .def(py::init<const Datetime&, price_t, price_t, price_t, price_t, price_t, price_t>())

      .def("__str__", to_py_str<KRecord>)
      .def("__repr__", to_py_str<KRecord>)

      .def_readwrite("datetime", &KRecord::datetime, "The time")
      .def_readwrite("open", &KRecord::openPrice, "The open price")
      .def_readwrite("high", &KRecord::highPrice, "The high price")
      .def_readwrite("low", &KRecord::lowPrice, "The low price")
      .def_readwrite("close", &KRecord::closePrice, "The close price")
      .def_readwrite("amount", &KRecord::transAmount, "The amount")
      .def_readwrite("volume", &KRecord::transCount, "The volume")

      .def("is_valid", &KRecord::isValid, "Whether the KRecord is valid")

      .def(py::self == py::self)
      .def(py::self != py::self)

        DEF_PICKLE(KRecord);

    m.def("krecords_to_np", [](const KRecordList& kdata) {
        size_t total = kdata.size();
        HKU_IF_RETURN(total == 0, py::array());

        struct RawData {
            int64_t datetime;  // The converted millisecond timestamp
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

        // Define the NumPy structured data type
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

        // Create the array
        py::array_t<int64_t> datetime_arr(total);
        py::array_t<double> open_arr(total);
        py::array_t<double> high_arr(total);
        py::array_t<double> low_arr(total);
        py::array_t<double> close_arr(total);
        py::array_t<double> amount_arr(total);
        py::array_t<double> vol_arr(total);

        // Get the buffer and fill the data
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

        // Build the DataFrame
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
          
    Convert a DataFrame to a KRecordList; the column names must be specified in order, defaulting to: ("datetime", "open", "high", "low", "close", "amount", "volume")

    :param DataFrame df: the input DataFrame
    :param dict columns: specify the column names of the DataFrame, corresponding to the member variable names of the KRecord
    :return: the converted KRecordList)",
          py::arg("df"),
          py::arg("columns") =
            StringList{"datetime", "open", "high", "low", "close", "amount", "volume"});
}
