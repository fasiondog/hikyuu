/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  PyKernel.h - the kernel abstraction behind np_apply: it turns the raw result buffers of one
 *  calculation into a call of a Python callable (or a ctypes function) and copies the results back
 */

#pragma once
#ifndef HIKYUU_PYWRAP_INDICATOR_PYFUNC_PYKERNEL_H
#define HIKYUU_PYWRAP_INDICATOR_PYFUNC_PYKERNEL_H

#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <fmt/format.h>

#include <hikyuu/indicator/IndicatorImp.h>
#include <hikyuu/utilities/datetime/Datetime.h>

#include "../../pybind_utils.h"

namespace hku {
namespace pyfunc {

typedef IndicatorImp::value_t value_t;

/** The number of the input indicators one np_apply node accepts (the WEAVE bridge limit) */
constexpr size_t MAX_INPUTS = 6;

/** The buffers of one calculation; `in_base` / `out_base` keep the buffer owners alive */
struct BufferSet {
    std::vector<const value_t*> in;
    std::vector<value_t*> out;
    size_t total{0};
    const DatetimeList* dates{nullptr};
    pybind11::object dates_obj;
    pybind11::object in_base;
    pybind11::object out_base;
};

/** The immutable description shared by every clone of one np_apply node. */
struct KernelSpec {
    std::string name{"APPLY"};
    size_t result_num{1};
    std::optional<size_t> discard;
    std::string view{"numpy"};
    bool with_dates{false};
    bool stateless{false};
    bool contiguous{false};
    bool raw{false};
};

inline bool sameSpec(const KernelSpec& a, const KernelSpec& b) {
    return a.name == b.name && a.result_num == b.result_num && a.discard == b.discard &&
           a.view == b.view && a.with_dates == b.with_dates && a.stateless == b.stateless &&
           a.contiguous == b.contiguous && a.raw == b.raw;
}

/** The kernel base; immutable and shared by all clones, `run` is called with the GIL held */
class PyKernel {
public:
    virtual ~PyKernel() = default;

    virtual void run(const BufferSet& bufs) const = 0;

    /** Whether two kernels perform the same calculation; used by PyFuncIndicatorImp::selfAlike */
    virtual bool identityEquals(const PyKernel& other) const noexcept {
        return typeid(*this) == typeid(other) && m_func->ptr() == other.m_func->ptr() &&
               m_input_num == other.m_input_num && m_output_num == other.m_output_num;
    }

    size_t inputNumber() const noexcept {
        return m_input_num;
    }

    size_t outputNumber() const noexcept {
        return m_output_num;
    }

protected:
    /** Must be constructed with the GIL held: it inspects the Python callable */
    PyKernel(pybind11::object func, size_t n_in, size_t n_out)
    : m_func(make_gil_safe(std::move(func))), m_input_num(n_in), m_output_num(n_out) {}

    const pybind11::object& func() const noexcept {
        return *m_func;
    }

private:
    std::shared_ptr<pybind11::object> m_func;
    size_t m_input_num;
    size_t m_output_num;
};

typedef std::shared_ptr<const PyKernel> PyKernelPtr;

/** The numpy format char of the build's value_t: "d" (float64) or "f" (low precision) */
inline char valueFormatChar() {
    return sizeof(value_t) == sizeof(double) ? 'd' : 'f';
}

/** The numpy dtype of the build's value_t */
inline pybind11::dtype valueDtype() {
    return pybind11::dtype::of<value_t>();
}

/** A numpy view over one indicator result buffer; input views are read only */
inline pybind11::array bufferView(const value_t* p, size_t total, const pybind11::handle& base,
                                  bool writable) {
    pybind11::array arr(
      valueDtype(), std::vector<pybind11::ssize_t>{static_cast<pybind11::ssize_t>(total)},
      std::vector<pybind11::ssize_t>{static_cast<pybind11::ssize_t>(sizeof(value_t))},
      const_cast<value_t*>(p), base);
    // numpy exposes its flags only through the C struct: with a capsule base pybind11 starts from
    // the writeable flag alone, so the alignment is added and the write access is dropped here
    auto* proxy = pybind11::detail::array_proxy(arr.ptr());
    proxy->flags |= pybind11::detail::npy_api::NPY_ARRAY_ALIGNED_;
    if (!writable) {
        proxy->flags &= ~pybind11::detail::npy_api::NPY_ARRAY_WRITEABLE_;
    }
    return arr;
}

inline pybind11::array inputView(const BufferSet& bufs, size_t idx) {
    return bufferView(bufs.in[idx], bufs.total, bufs.in_base, false);
}

inline pybind11::array outputView(const BufferSet& bufs, size_t idx) {
    return bufferView(bufs.out[idx], bufs.total, bufs.out_base, true);
}

/** A numpy base object keeping the buffer owner alive; a capsule just holds one reference */
inline pybind11::capsule bufferOwnerCapsule(IndicatorImpPtr owner) {
    return pybind11::capsule(new IndicatorImpPtr(std::move(owner)),
                             [](void* p) { delete static_cast<IndicatorImpPtr*>(p); });
}

inline pybind11::object callPython(const pybind11::object& func, const pybind11::tuple& args,
                                   const pybind11::dict& kwargs = pybind11::dict()) {
    PyObject* result = PyObject_Call(func.ptr(), args.ptr(), kwargs.ptr());
    if (result == nullptr) {
        throw pybind11::error_already_set();
    }
    return pybind11::reinterpret_steal<pybind11::object>(result);
}

inline std::string shapeText(const pybind11::array& arr) {
    std::string text = "(";
    for (pybind11::ssize_t i = 0; i < arr.ndim(); ++i) {
        if (i > 0) {
            text += ", ";
        }
        text += std::to_string(arr.shape(i));
    }
    if (arr.ndim() == 1) {
        text += ",";
    }
    return text + ")";
}

/** The idx-th item of a sequence; `object::operator[]` only accepts a key, not an index */
inline pybind11::object sequenceItem(const pybind11::object& seq, size_t idx) {
    PyObject* item = PySequence_GetItem(seq.ptr(), static_cast<Py_ssize_t>(idx));
    if (item == nullptr) {
        throw pybind11::error_already_set();
    }
    return pybind11::reinterpret_steal<pybind11::object>(item);
}

/** Split a returned object into `result_num` results; a one element tuple is unwrapped, a list is
 *  taken as it is (its length may legitimately be 1 as the single result) */
inline std::vector<pybind11::object> splitResults(const pybind11::object& res, size_t result_num) {
    std::vector<pybind11::object> values;
    values.reserve(result_num);

    const bool is_tuple = pybind11::isinstance<pybind11::tuple>(res);
    const bool is_list = pybind11::isinstance<pybind11::list>(res);

    if (result_num > 1) {
        const size_t got = (is_tuple || is_list) ? static_cast<size_t>(pybind11::len(res)) : 1;
        if ((!is_tuple && !is_list) || got != result_num) {
            throw pybind11::value_error(
              fmt::format("np_apply: func returned {} result(s), but "
                          "result_num={}",
                          got, result_num));
        }
        for (size_t i = 0; i < result_num; ++i) {
            values.push_back(pybind11::reinterpret_steal<pybind11::object>(
              PySequence_GetItem(res.ptr(), static_cast<Py_ssize_t>(i))));
        }
        return values;
    }

    if (is_tuple) {
        const size_t got = static_cast<size_t>(pybind11::len(res));
        if (got != 1) {
            throw pybind11::value_error(
              fmt::format("np_apply: func returned {} result(s), but result_num=1", got));
        }
        values.push_back(
          pybind11::reinterpret_steal<pybind11::object>(PySequence_GetItem(res.ptr(), 0)));
        return values;
    }

    values.push_back(res);
    return values;
}

/** Validate one returned result (length == input length) and copy it into the result buffer */
inline void writeResult(const pybind11::handle& value, size_t idx, size_t total, value_t* dst) {
    auto arr =
      pybind11::array_t<value_t, pybind11::array::c_style | pybind11::array::forcecast>::ensure(
        value);
    if (!arr) {
        throw pybind11::type_error(
          fmt::format("np_apply: result {} is not convertible to an array", idx));
    }
    if (arr.ndim() != 1 || static_cast<size_t>(arr.shape(0)) != total) {
        throw pybind11::value_error(fmt::format("np_apply: result {} has shape {}, expected ({},)",
                                                idx, shapeText(arr), total));
    }
    std::memcpy(dst, arr.data(), total * sizeof(value_t));
}

/** The `__name__` of the type of an object, e.g. "vectorize" */
inline std::string typeName(const pybind11::handle& obj) {
    return pybind11::str(pybind11::type::handle_of(obj).attr("__name__")).cast<std::string>();
}

}  // namespace pyfunc
}  // namespace hku

#endif  // HIKYUU_PYWRAP_INDICATOR_PYFUNC_PYKERNEL_H
