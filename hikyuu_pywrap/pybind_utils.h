/*
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-6-25
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_PYBIND_UTILS_H
#define HIKYUU_PYBIND_UTILS_H

#include <boost/python.hpp>

template <class T>
struct vector_to_python_list {
    static PyObject* convert(T const& x) {
        boost::python::list pylist;
        for (auto const& d : x) {
            pylist.append(d);
        }
        return boost::python::incref(pylist.ptr());
    }
};

#define VECTOR_TO_PYTHON_CONVERT(vectorname) \
    to_python_converter<vectorname, vector_to_python_list<vectorname>, false>()

#endif  // HIKYUU_PYBIND_UTILS_H