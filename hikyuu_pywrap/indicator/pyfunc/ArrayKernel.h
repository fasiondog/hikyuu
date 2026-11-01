/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  ArrayKernel.h - the plain callable kernel of np_apply
 */

#pragma once
#ifndef HIKYUU_PYWRAP_INDICATOR_PYFUNC_ARRAYKERNEL_H
#define HIKYUU_PYWRAP_INDICATOR_PYFUNC_ARRAYKERNEL_H

#include "PyKernel.h"

namespace hku {
namespace pyfunc {

/** Any callable receiving the series as read only numpy arrays (optionally led by the dates) */
class ArrayKernel : public PyKernel {
public:
    ArrayKernel(pybind11::object func, size_t n_in, size_t n_out, bool with_dates)
    : PyKernel(std::move(func), n_in, n_out), m_with_dates(with_dates) {}

    void run(const BufferSet& bufs) const override;

private:
    bool m_with_dates;
};

}  // namespace pyfunc
}  // namespace hku

#endif  // HIKYUU_PYWRAP_INDICATOR_PYFUNC_ARRAYKERNEL_H
