/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  PyFuncIndicatorImp.h - the indicator node behind np_apply
 */

#pragma once
#ifndef HIKYUU_PYWRAP_INDICATOR_PYFUNC_PYFUNCINDICATORIMP_H
#define HIKYUU_PYWRAP_INDICATOR_PYFUNC_PYFUNCINDICATORIMP_H

#include <memory>

#include <hikyuu/indicator/IndicatorImp.h>

#include "PyKernel.h"

namespace hku {
namespace pyfunc {

/** An indicator whose values are produced by one Python call over the whole series; not picklable
 *  and not incrementally calculable (the callable lives in the Python process) */
class PyFuncIndicatorImp : public IndicatorImp {
public:
    PyFuncIndicatorImp(PyKernelPtr kernel, const KernelSpec& spec);

    bool hasPythonPart() const noexcept override {
        return true;
    }

    IndicatorImpPtr _clone() override;

    /** Equivalent only when both declare a pure kernel: a stateful callable must never be shared */
    bool selfAlike(const IndicatorImp& other) const noexcept override;

    void _calculate(const Indicator& ind) override;

    const PyKernelPtr& kernel() const noexcept {
        return m_kernel;
    }

    const KernelSpec& spec() const noexcept {
        return m_spec;
    }

private:
    PyKernelPtr m_kernel;
    KernelSpec m_spec;
};

typedef std::shared_ptr<PyFuncIndicatorImp> PyFuncIndicatorImpPtr;

}  // namespace pyfunc
}  // namespace hku

#endif  // HIKYUU_PYWRAP_INDICATOR_PYFUNC_PYFUNCINDICATORIMP_H
