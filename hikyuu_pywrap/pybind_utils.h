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
    // If len(obj) is zero it may succeed even when the type is not the expected one, but there is
    // no risk
    auto total = len(obj);
    std::vector<T> vect(total);
    for (auto i = 0; i < total; ++i) {
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

// Used to check whether the number of the function arguments passed as py::object is as expected
inline bool check_pyfunction_arg_num(const py::object& func, size_t arg_num) {
    py::module_ inspect = py::module_::import("inspect");
    py::object sig = inspect.attr("signature")(func);
    py::object params = sig.attr("parameters");
    return len(params) == arg_num;
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
