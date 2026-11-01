/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  ArrayKernel.cpp
 */

#include "ArrayKernel.h"

namespace hku {
namespace pyfunc {

void ArrayKernel::run(const BufferSet& bufs) const {
    pybind11::tuple args(bufs.in.size() + (m_with_dates ? 1 : 0));
    size_t k = 0;
    if (m_with_dates) {
        args[k++] = bufs.dates_obj;
    }
    for (size_t i = 0; i < bufs.in.size(); ++i) {
        args[k++] = inputView(bufs, i);
    }

    pybind11::object res = callPython(func(), args);
    auto values = splitResults(res, bufs.out.size());
    for (size_t i = 0; i < values.size(); ++i) {
        writeResult(values[i], i, bufs.total, bufs.out[i]);
    }
}

}  // namespace pyfunc
}  // namespace hku
