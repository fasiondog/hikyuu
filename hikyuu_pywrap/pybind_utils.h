/*
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-6-25
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_PYBIND_UTILS_H
#define HIKYUU_PYBIND_UTILS_H

#include <vector>
#include <boost/python.hpp>

namespace py = boost::python;

template <class T>
py::list vector_to_py_list(const T& v) {
    py::list ret;
    for (size_t i = 0, total = v.size(); i < total; i++) {
        ret.append(v[i]);
    }
    return ret;
}

template <class T>
T python_list_to_vector(py::list pylist) {
    size_t total = py::len(pylist);
    T result(total);
    for (size_t i = 0; i < total; i++) {
        result[i] = py::extract<typename T::value_type>(pylist[i])();
    }
    return result;
}

#endif  // HIKYUU_PYBIND_UTILS_H