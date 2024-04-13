/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2020-5-25
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_PYTHON_BIND_UTILS_H
#define HIKYUU_PYTHON_BIND_UTILS_H

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
    auto total = len(obj);
    std::vector<T> vect(total);
    for (auto i = 0; i < total; ++i) {
        vect[i] = obj[i].cast<T>();
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

// 直接使用 pybind11 重载 _clone，在 C++ 中会丢失 python 中的类型
// 参考：https://github.com/pybind/pybind11/issues/1049 进行修改
// PYBIND11_OVERLOAD(IndicatorImpPtr, IndicatorImp, _clone, );
#define PY_CLONE(pyclassname, classname)                                     \
public:                                                                      \
    std::shared_ptr<classname> _clone() override {                           \
        auto self = py::cast(this);                                          \
        auto cloned = self.attr("_clone")();                                 \
                                                                             \
        auto keep_python_state_alive = std::make_shared<py::object>(cloned); \
        auto ptr = cloned.cast<pyclassname*>();                              \
                                                                             \
        return std::shared_ptr<classname>(keep_python_state_alive, ptr);     \
    }

}  // namespace hku

#endif  // HIKYUU_PYTHON_BIND_UTILS_H
