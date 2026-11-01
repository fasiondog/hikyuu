/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  PandasKernel.h - the pandas view kernel of np_apply
 */

#pragma once
#ifndef HIKYUU_PYWRAP_INDICATOR_PYFUNC_PANDASKERNEL_H
#define HIKYUU_PYWRAP_INDICATOR_PYFUNC_PANDASKERNEL_H

#include "PyKernel.h"

namespace hku {
namespace pyfunc {

/** The callable receives a pandas Series (one input) or a DataFrame (several inputs), date indexed
 *  (a RangeIndex when the input carries no date) */
class PandasKernel : public PyKernel {
public:
    PandasKernel(pybind11::object func, size_t n_in, size_t n_out)
    : PyKernel(std::move(func), n_in, n_out) {}

    void run(const BufferSet& bufs) const override;
};

}  // namespace pyfunc
}  // namespace hku

#endif  // HIKYUU_PYWRAP_INDICATOR_PYFUNC_PANDASKERNEL_H
