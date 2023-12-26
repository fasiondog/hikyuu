/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2020-5-24
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_PYTHON_CONVERT_ANY_H
#define HIKYUU_PYTHON_CONVERT_ANY_H

#include <string>
#include <pybind11/pybind11.h>
#include <hikyuu/utilities/Parameter.h>

namespace pybind11 {
namespace detail {
template <>
struct type_caster<boost::any> {
public:
    PYBIND11_TYPE_CASTER(boost::any, _("any"));

    /**
     * Conversion part 1 (Python->C++)
     */
    bool load(handle source, bool) {
        if (source.is_none()) {
            return false;
        }

        /* Extract PyObject from handle */
        PyObject* src = source.ptr();

        if (PyBool_Check(src)) {
            value = bool(PyLong_AsLong(src));
            return true;
        }

        if (PyLong_Check(src)) {
            long tmp = PyLong_AsLong(src);
            if (tmp > std::numeric_limits<int>::max()) {
                HKU_THROW_EXCEPTION(std::out_of_range,
                                    "This number is over limit.Paramter only support int32!");
            }
            value = int(tmp);
            return !(tmp == -1 && !PyErr_Occurred());
        }

        if (PyFloat_Check(src)) {
            value = PyFloat_AsDouble(src);
            return true;
        }

        if (PyUnicode_Check(src)) {
            object temp = reinterpret_borrow<object>(PyUnicode_AsUTF8String(src));
            if (!temp)
                pybind11_fail("Unable to extract string contents! (encoding issue)");
            char* buffer;
            ssize_t length;
            if (PYBIND11_BYTES_AS_STRING_AND_SIZE(temp.ptr(), &buffer, &length))
                pybind11_fail("Unable to extract string contents! (invalid type)");
            value = std::string(buffer, (size_t)length);
            return true;
        }

        try {
            value = pydatetime_to_Datetime(source);
            return true;
        } catch (...) {
            // do noting;
        }

        HKU_THROW_EXCEPTION(std::logic_error,
                            "Faile convert this value to boost::any, it may be not supported!");
        return false;
    }

    /**
     * Conversion part 2 (C++ -> Python)
     */
    static handle cast(boost::any x, return_value_policy /* policy */, handle /* parent */) {
        if (x.type() == typeid(bool)) {
            bool tmp = boost::any_cast<bool>(x);
            return tmp ? Py_True : Py_False;

        } else if (x.type() == typeid(int)) {
            return Py_BuildValue("n", boost::any_cast<int>(x));

        } else if (x.type() == typeid(double)) {
            return Py_BuildValue("d", boost::any_cast<double>(x));

        } else if (x.type() == typeid(std::string)) {
            std::string s(boost::any_cast<std::string>(x));
            return Py_BuildValue("s", s.c_str());

        } else {
            HKU_ERROR("convert failed! Unkown type! Will return None!");
            return Py_BuildValue("s", (char*)0);
        }
    }
};
}  // namespace detail
}  // namespace pybind11

#endif /* HIKYUU_PYTHON_CONVERT_ANY_H */