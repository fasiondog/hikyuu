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
struct vector_to_python_list {
    static PyObject* convert(T const& x) {
        py::list pylist;
        for (auto const& d : x) {
            pylist.append(d);
        }
        return py::incref(pylist.ptr());
    }
};  // namespace boost::pythontemplate<classT>structvector_to_python_list

#define VECTOR_TO_PYTHON_CONVERT(vectorname) \
    py::to_python_converter<vectorname, vector_to_python_list<vectorname>, false>()

template <class T>
T python_list_to_vector(py::list pylist) {
    size_t total = py::len(pylist);
    T result(total);
    for (size_t i = 0; i < total; i++) {
        result[i] = py::extract<T::value_type>(pylist[i])();
    }
    return result;
}

#endif  // HIKYUU_PYBIND_UTILS_H