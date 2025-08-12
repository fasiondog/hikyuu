/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-12
 *      Author: fasiondog
 */

#include "df_to_ks.h"

namespace hku {

KRecordList df_to_krecords(const py::object& df, const StringList& cols) {
    HKU_CHECK(cols.size() == 7, "cols size must equal to 7!");

    KRecordList ret;
    py::module_ pandas = py::module_::import("pandas");
    py::object datetime_col = pandas.attr("to_datetime")(df.attr("__getitem__")(cols[0]));
    py::array np_dates = datetime_col.attr("to_numpy")().attr("view")("int64");

    py::array_t<double> open_array =
      df.attr("__getitem__")(cols[1]).attr("to_numpy")().attr("astype")(py::dtype("float64"));
    py::array_t<double> high_array =
      df.attr("__getitem__")(cols[2]).attr("to_numpy")().attr("astype")(py::dtype("float64"));
    py::array_t<double> low_array =
      df.attr("__getitem__")(cols[3]).attr("to_numpy")().attr("astype")(py::dtype("float64"));
    py::array_t<double> close_array =
      df.attr("__getitem__")(cols[4]).attr("to_numpy")().attr("astype")(py::dtype("float64"));
    py::array_t<double> amount_array =
      df.attr("__getitem__")(cols[5]).attr("to_numpy")().attr("astype")(py::dtype("float64"));
    py::array_t<double> volume_array =
      df.attr("__getitem__")(cols[6]).attr("to_numpy")().attr("astype")(py::dtype("float64"));

    auto date = static_cast<int64_t*>(np_dates.request().ptr);
    auto open = open_array.data();
    auto close = close_array.data();
    auto high = high_array.data();
    auto low = low_array.data();
    auto amount = amount_array.data();
    auto volume = volume_array.data();

    size_t total = open_array.size();
    ret.resize(total);
    for (size_t i = 0; i < total; i++) {
        ret[i].datetime = Datetime::fromTimestamp(date[i] / 1000LL);
        ret[i].openPrice = open[i];
        ret[i].closePrice = close[i];
        ret[i].highPrice = high[i];
        ret[i].lowPrice = low[i];
        ret[i].transAmount = amount[i];
        ret[i].transCount = volume[i];
    }

    return ret;
}

}  // namespace hku