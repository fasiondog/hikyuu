/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  PandasKernel.cpp
 */

#include "PandasKernel.h"

namespace hku {
namespace pyfunc {

/** The index of the input series: the dates as datetime64[us], a RangeIndex without dates */
static pybind11::object pandasIndex(const BufferSet& bufs, const pybind11::object& pd) {
    if (bufs.dates == nullptr || bufs.dates->empty() || bufs.dates->size() != bufs.total) {
        return pd.attr("RangeIndex")(pybind11::int_(bufs.total));
    }

    pybind11::array_t<int64_t> ticks(static_cast<pybind11::ssize_t>(bufs.total));
    auto* dst = ticks.mutable_data();
    for (size_t i = 0; i < bufs.total; ++i) {
        dst[i] = (*bufs.dates)[i].timestamp();
    }
    return pd.attr("DatetimeIndex")(ticks.attr("astype")(pybind11::str("datetime64[us]")));
}

/** Split a returned DataFrame into one array per column, in the column order */
static pybind11::tuple pandasFrameColumns(const pybind11::object& frame) {
    const auto shape = pybind11::cast<std::pair<size_t, size_t>>(frame.attr("shape"));
    pybind11::tuple values(shape.second);
    for (size_t i = 0; i < shape.second; ++i) {
        auto column = frame.attr("iloc").attr("__getitem__")(pybind11::make_tuple(
          pybind11::slice(0, static_cast<pybind11::ssize_t>(shape.first), 1), pybind11::int_(i)));
        values[i] = column.attr("to_numpy")();
    }
    return values;
}

void PandasKernel::run(const BufferSet& bufs) const {
    pybind11::object pd = pybind11::module_::import("pandas");
    pybind11::object index = pandasIndex(bufs, pd);

    pybind11::object arg;
    if (bufs.in.size() == 1) {
        pybind11::dict kwargs;
        kwargs["index"] = index;
        arg = callPython(pd.attr("Series"), pybind11::make_tuple(inputView(bufs, 0)), kwargs);
    } else {
        pybind11::tuple arrays(bufs.in.size());
        pybind11::list columns;
        for (size_t i = 0; i < bufs.in.size(); ++i) {
            arrays[i] = inputView(bufs, i);
            columns.append(pybind11::str(fmt::format("value{}", i)));
        }
        pybind11::object stacked = pybind11::module_::import("numpy").attr("column_stack")(arrays);
        pybind11::dict kwargs;
        kwargs["index"] = index;
        kwargs["columns"] = columns;
        arg = callPython(pd.attr("DataFrame"), pybind11::make_tuple(stacked), kwargs);
    }

    pybind11::object res = callPython(func(), pybind11::make_tuple(arg));
    if (pybind11::isinstance(res, pd.attr("Series"))) {
        res = res.attr("to_numpy")();
    } else if (pybind11::isinstance(res, pd.attr("DataFrame"))) {
        res = pandasFrameColumns(res);
    }

    auto values = splitResults(res, bufs.out.size());
    for (size_t i = 0; i < values.size(); ++i) {
        writeResult(values[i], i, bufs.total, bufs.out[i]);
    }
}

}  // namespace pyfunc
}  // namespace hku
