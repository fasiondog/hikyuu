/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  CFuncKernel.h - the ctypes kernel of np_apply
 */

#pragma once
#ifndef HIKYUU_PYWRAP_INDICATOR_PYFUNC_CFUNCKERNEL_H
#define HIKYUU_PYWRAP_INDICATOR_PYFUNC_CFUNCKERNEL_H

#include "PyKernel.h"

namespace hku {
namespace pyfunc {

/** A ctypes function (including a numba `cfunc.ctypes`) receiving `in0, ..., out0, ..., total`;
 *  ctypes releases the GIL for the call */
class CFuncKernel : public PyKernel {
public:
    CFuncKernel(pybind11::object func, size_t n_in, size_t n_out);

    void run(const BufferSet& bufs) const override;

private:
    pybind11::object argument(size_t idx, const value_t* ptr, const pybind11::object& castFn) const;

    std::shared_ptr<pybind11::object> m_argtypes;  ///< the ctypes argument types
    std::vector<bool> m_pointer_args;              ///< whether the argument is a pointer
};

}  // namespace pyfunc
}  // namespace hku

#endif  // HIKYUU_PYWRAP_INDICATOR_PYFUNC_CFUNCKERNEL_H
