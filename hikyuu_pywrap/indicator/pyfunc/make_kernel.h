/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  make_kernel.h - the construction time dispatch of np_apply
 */

#pragma once
#ifndef HIKYUU_PYWRAP_INDICATOR_PYFUNC_MAKEKERNEL_H
#define HIKYUU_PYWRAP_INDICATOR_PYFUNC_MAKEKERNEL_H

#include <optional>
#include <string>

#include <hikyuu/indicator/Indicator.h>

#include "PyKernel.h"

namespace hku {
namespace pyfunc {

/** Build the kernel of one callable (the caller holds the GIL); a bad shape is rejected here */
PyKernelPtr makeKernel(const pybind11::object& func, size_t input_num, const KernelSpec& spec);

/** Wrap a single series callable as an indicator: the entry point of `Indicator.apply`.
 *  @param hints {"contiguous": bool, "raw": bool}; "raw" calls a numba cfunc through its address
 *  and drops the GIL, refused for a ctypes function, a ufunc, the pandas view and withDates */
Indicator npApply(const Indicator& ref, const pybind11::object& func, int64_t resultNum,
                  std::optional<int64_t> discard, const std::string& view, bool withDates,
                  bool stateless, const std::string& name, const pybind11::object& hints);

/** The np_apply entry point: the input is an Indicator or up to MAX_INPUTS Indicators (WEAVE) */
Indicator npApply(const pybind11::object& ref, const pybind11::object& func, int64_t resultNum,
                  std::optional<int64_t> discard, const std::string& view, bool withDates,
                  bool stateless, const std::string& name, const pybind11::object& hints);

}  // namespace pyfunc
}  // namespace hku

#endif  // HIKYUU_PYWRAP_INDICATOR_PYFUNC_MAKEKERNEL_H
