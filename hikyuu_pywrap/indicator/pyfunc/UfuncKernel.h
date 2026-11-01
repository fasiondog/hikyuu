/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  UfuncKernel.h - the numpy ufunc kernel of np_apply
 */

#pragma once
#ifndef HIKYUU_PYWRAP_INDICATOR_PYFUNC_UFUNCKERNEL_H
#define HIKYUU_PYWRAP_INDICATOR_PYFUNC_UFUNCKERNEL_H

#include "PyKernel.h"

namespace hku {
namespace pyfunc {

/** The loop shapes a ufunc can offer for the build's value_t */
enum class UfuncLoopKind {
    DTYPE,     ///< at least one loop only uses the value_t char on both sides
    OBJECT,    ///< only object loops (np.frompyfunc), it runs Python per element
    NO_ARROW,  ///< the signatures are not of the `in->out` form (numba DUFunc)
};

/** Inspect `ufunc.types` for a loop matching the arity and the build's value_t; a numba DUFunc
 *  reports non format-char names and is reported as NO_ARROW (callable as a plain function) */
UfuncLoopKind ufuncLoopKind(const pybind11::object& ufunc, size_t n_in, size_t n_out);

/** A numpy ufunc; the results are written through the `out=` argument, no temporary is allocated */
class UfuncKernel : public PyKernel {
public:
    UfuncKernel(pybind11::object func, size_t n_in, size_t n_out);

    void run(const BufferSet& bufs) const override;
};

}  // namespace pyfunc
}  // namespace hku

#endif  // HIKYUU_PYWRAP_INDICATOR_PYFUNC_UFUNCKERNEL_H
