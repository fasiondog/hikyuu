/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  PyFuncIndicatorImp.cpp
 */

#include "PyFuncIndicatorImp.h"

#include <algorithm>

#include <hikyuu/indicator/Indicator.h>

namespace hku {
namespace pyfunc {

PyFuncIndicatorImp::PyFuncIndicatorImp(PyKernelPtr kernel, const KernelSpec& spec)
: IndicatorImp(spec.name, spec.result_num), m_kernel(std::move(kernel)), m_spec(spec) {
    // not a Python object: keep the batch reuse fast path
    supportBatchReuse(m_spec.stateless);
    m_need_self_alike_compare = true;
}

IndicatorImpPtr PyFuncIndicatorImp::_clone() {
    return std::make_shared<PyFuncIndicatorImp>(m_kernel, m_spec);
}

bool PyFuncIndicatorImp::selfAlike(const IndicatorImp& other) const noexcept {
    const auto* o = dynamic_cast<const PyFuncIndicatorImp*>(&other);
    return o != nullptr && m_spec.stateless && sameSpec(m_spec, o->m_spec) &&
           m_kernel->identityEquals(*o->m_kernel);
}

void PyFuncIndicatorImp::_calculate(const Indicator& ind) {
    // the whole kernel call, views and returned objects included, lives in this GIL section
    pybind11::gil_scoped_acquire gil;

    const size_t total = ind.size();
    if (size() != total || getResultNumber() != m_spec.result_num) {
        // reached outside the unary node path, which prepares the buffer itself
        _readyBuffer(total, m_spec.result_num);
    }
    // an empty input never calls the callable
    HKU_IF_RETURN(total == 0, void());

    setDiscard(m_spec.discard ? *m_spec.discard : ind.discard());

    BufferSet bufs;
    bufs.total = total;
    bufs.in.reserve(ind.getResultNumber());
    for (size_t i = 0; i < ind.getResultNumber(); ++i) {
        const value_t* p = ind.data(i);
        HKU_CHECK(p != nullptr, "np_apply: the input result {} is not ready!", i);
        bufs.in.push_back(p);
    }
    // the raw kernel dispatches through a table fixed at construction
    HKU_CHECK(bufs.in.size() == m_kernel->inputNumber(),
              "np_apply: the input provides {} result set(s), but the kernel expects {}",
              bufs.in.size(), m_kernel->inputNumber());

    bufs.out.reserve(getResultNumber());
    for (size_t i = 0; i < getResultNumber(); ++i) {
        value_t* p = data(i);
        HKU_CHECK(p != nullptr, "np_apply: the output result {} is not ready!", i);
        bufs.out.push_back(p);
    }

    // the owners must outlive the call even if the callable keeps a view
    bufs.in_base = bufferOwnerCapsule(ind.getImp());
    bufs.out_base = bufferOwnerCapsule(shared_from_this());

    DatetimeList dates;
    if (m_spec.with_dates || m_spec.view == "pandas") {
        dates = ind.getDatetimeList();
        bufs.dates = &dates;
        if (m_spec.with_dates) {
            bufs.dates_obj = pybind11::cast(dates);
        }
    }

    // a Python exception of the callable is not wrapped
    m_kernel->run(bufs);

    // the callable may write the whole series: rewrite the declared discard prefix as Null
    const size_t prefix = discard();
    if (prefix > 0) {
        value_t null_value = Null<value_t>();
        for (size_t r = 0; r < getResultNumber(); ++r) {
            std::fill(data(r), data(r) + prefix, null_value);
        }
    }
}

}  // namespace pyfunc
}  // namespace hku
