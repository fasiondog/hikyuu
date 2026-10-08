/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2020-5-25
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_PYTHON_BIND_UTILS_H
#define HIKYUU_PYTHON_BIND_UTILS_H

#include <hikyuu/config.h>
#include <hikyuu/Stock.h>
#include <memory>
#include <type_traits>
#include <pybind11/pybind11.h>

#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include <string>
#include "convert_any.h"
#include "pickle_support.h"
#include "ioredirect.h"

namespace py = pybind11;

namespace hku {

// Wrap a py::object so its lifetime can safely cross worker threads (e.g. TimerManager
// entries). The deleter acquires the GIL before releasing; if the interpreter is going
// down, skip decref (leak; process is exiting). Reusable across pybind bindings.
inline std::shared_ptr<py::object> make_gil_safe(py::object obj) {
    return std::shared_ptr<py::object>(new py::object(std::move(obj)), [](py::object* p) {
        bool finalizing =
#if PY_VERSION_HEX >= 0x030D0000
          Py_IsFinalizing() != 0;
#else
            !Py_IsInitialized();
#endif
        if (finalizing) {
            return;
        }
        py::gil_scoped_acquire gil;
        delete p;
    });
}

template <typename T>
py::bytes vector_to_python_bytes(const std::vector<T>& vect) {
    return py::bytes((char*)vect.data(), vect.size() * sizeof(T));
}

template <typename T>
std::vector<T> python_bytes_to_vector(const py::bytes& obj) {
    auto bytes_len = len(obj);
    if (bytes_len % sizeof(T) != 0) {
        throw std::runtime_error("The length bytes not match!");
    }
    auto vect_len = bytes_len / sizeof(T);
    std::vector<T> result(vect_len);

    char* buffer = nullptr;
    Py_ssize_t length = 0;
    if (PyBytes_AsStringAndSize(obj.ptr(), &buffer, &length) != 0) {
        throw std::runtime_error("trans bytes to vector failed!");
    }

    if (length != static_cast<Py_ssize_t>(vect_len * sizeof(T))) {
        throw std::runtime_error("The length bytes not match!");
    }

    memcpy(result.data(), buffer, length);
    return result;
}

template <typename T>
std::vector<T> python_list_to_vector(const py::sequence& obj) {
    if constexpr (std::is_arithmetic_v<T>) {
        using ArrayType = py::array_t<T, py::array::c_style>;
        if (ArrayType::check_(obj)) {
            auto arr = py::reinterpret_borrow<ArrayType>(obj);
            if (arr.ndim() == 1) {
                const T* p = arr.data();
                return std::vector<T>(p, p + arr.size());
            }
        }
    }

    // If len(obj) is zero it may succeed even when the type is not the expected one, but there is
    // no risk
    Py_ssize_t total = len(obj);
    std::vector<T> vect(total);
    for (Py_ssize_t i = 0; i < total; ++i) {
        vect[i] = py::cast<T>(obj[i]);
    }
    return vect;
}

template <typename T>
py::list vector_to_python_list(const std::vector<T>& vect) {
    py::list obj;
    for (unsigned long i = 0; i < vect.size(); ++i)
        obj.append(vect[i]);
    return obj;
}

template <typename T>
void extend_vector_with_python_list(std::vector<T>& v, const py::sequence& l) {
    for (const auto& item : l)
        v.push_back(item.cast<T>());
}

template <typename T>
std::string to_py_str(const T& item) {
    std::stringstream out;
    out << item;
    return out.str();
}

// Using the pybind11 overload of _clone directly would lose the python type in C++
// Refer to https://github.com/pybind/pybind11/issues/1049 for the modification
// PYBIND11_OVERLOAD(IndicatorImpPtr, IndicatorImp, _clone, );
#define PY_CLONE(pyclassname, classname)                                         \
public:                                                                          \
    std::shared_ptr<classname> _clone() override {                               \
        if (isPythonObject()) {                                                  \
            py::gil_scoped_acquire acquire;                                      \
            auto self = py::cast(this);                                          \
            auto cloned = self.attr("_clone")();                                 \
            auto keep_python_state_alive = std::make_shared<py::object>(cloned); \
            auto ptr = cloned.cast<pyclassname*>();                              \
            return std::shared_ptr<classname>(keep_python_state_alive, ptr);     \
        }                                                                        \
        return this->_clone();                                                   \
    }

// The `inspect` handles used by check_pyfunction_arg_num, cached because the check runs on every
// callback registration. Held as raw PyObject* to keep pybind11 types out of the struct (they are
// compiled with hidden visibility). The references are intentionally leaked: the cache outlives
// the interpreter and must not be decref'd during finalization.
inline const auto& py_inspect_cache() {
    struct Cache {
        PyObject* inspect;
        PyObject* empty;           // inspect.Parameter.empty
        PyObject* var_positional;  // inspect.Parameter.VAR_POSITIONAL
        PyObject* var_keyword;     // inspect.Parameter.VAR_KEYWORD
        PyObject* keyword_only;    // inspect.Parameter.KEYWORD_ONLY
    };

    static const Cache* cache = new Cache{[]() {
        py::object inspect = py::module_::import("inspect");
        py::object parameter = inspect.attr("Parameter");
        py::object empty = parameter.attr("empty");
        py::object var_positional = parameter.attr("VAR_POSITIONAL");
        py::object var_keyword = parameter.attr("VAR_KEYWORD");
        py::object keyword_only = parameter.attr("KEYWORD_ONLY");
        return Cache{inspect.release().ptr(), empty.release().ptr(), var_positional.release().ptr(),
                     var_keyword.release().ptr(), keyword_only.release().ptr()};
    }()};
    return *cache;
}

// Used to check whether the callable passed as py::object accepts the expected number of
// positional arguments. Following the inspect.signature convention, the implicit `self` of a
// bound method or of a callable object is not counted. Callables without an introspectable
// signature (builtins, numpy ufuncs, ...) are accepted, as their arity cannot be determined.
inline bool check_pyfunction_arg_num(const py::object& func, size_t arg_num) {
    const auto& cache = py_inspect_cache();
    py::object params;
    try {
        params = py::handle(cache.inspect).attr("signature")(func).attr("parameters");
    } catch (py::error_already_set& e) {
        // Not introspectable: accept instead of breaking the registration
        if (e.matches(PyExc_ValueError) || e.matches(PyExc_TypeError)) {
            return true;
        }
        throw;
    }

    size_t required = 0, positional = 0;
    for (auto item : params.attr("values")()) {
        py::object kind_obj = item.attr("kind");
        PyObject* kind = kind_obj.ptr();
        if (kind == cache.var_positional) {
            return true;  // *args accepts any number of positional arguments
        }
        if (kind == cache.var_keyword) {
            continue;  // **kwargs only absorbs keyword arguments
        }
        py::object default_value = item.attr("default");
        bool has_default = default_value.ptr() != cache.empty;
        if (kind == cache.keyword_only) {
            // the caller only passes positional arguments
            HKU_IF_RETURN(!has_default, false);
            continue;
        }
        positional++;
        if (!has_default) {
            required++;
        }
    }
    return arg_num >= required && arg_num <= positional;
}

/*
 * Convert a utf8 encoded string to the utf32 encoding
 * @param utf8_str the string to be converted
 * @param out the array storing the conversion result (the memory needs to be allocated in advance
 *            by yourself)
 * @param out_len the length of the out array
 * @return the actual number of the converted code points
 */
size_t utf8_to_utf32(const std::string& utf8_str, int32_t* out, size_t out_len) noexcept;

// Get the StockList from a python object
inline StockList get_stock_list_from_python(const py::object& stks) {
    StockList ret;
    HKU_IF_RETURN(stks.is_none(), ret);

    if (py::isinstance<StockList>(stks)) {
        ret = stks.cast<StockList>();
    } else if (py::isinstance<Block>(stks)) {
        const auto& blk = stks.cast<Block&>();
        ret = blk.getStockList();
    } else if (py::isinstance<StockManager>(stks)) {
        const auto& sm = stks.cast<StockManager&>();
        ret = sm.getStockList();
    } else if (py::isinstance<py::sequence>(stks)) {
        ret = python_list_to_vector<Stock>(stks);
    } else {
        HKU_THROW("Failed get StockList! Input stks must be Block, sm or sequenc(Stock)!");
    }
    return ret;
}

inline Block get_block_from_python(const py::object& blk) {
    Block ret;
    HKU_IF_RETURN(blk.is_none(), ret);

    if (py::isinstance<StockList>(blk)) {
        ret = Block(blk.cast<StockList>());
    } else if (py::isinstance<Block>(blk)) {
        ret = blk.cast<Block>();
    } else if (py::isinstance<StockManager>(blk)) {
        const auto& sm = blk.cast<StockManager&>();
        ret = Block(sm.getStockList());
    } else if (py::isinstance<py::sequence>(blk)) {
        ret = Block(python_list_to_vector<Stock>(blk));
    } else {
        HKU_THROW("Failed get Block! Input blk must be Block, sm or sequenc(Stock)!");
    }
    return ret;
}

}  // namespace hku

#endif  // HIKYUU_PYTHON_BIND_UTILS_H
