/*
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-6-15
 *      Author: fasiondog
 */

#pragma once

#include <pybind11/pybind11.h>
#include <hikyuu/data_driver/KDataDriver.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

class PyKDataDriver : public KDataDriver {
    PY_CLONE(PyKDataDriver, KDataDriver)

public:
    PyKDataDriver() : KDataDriver() {
        m_is_python_object = true;
    }

    PyKDataDriver(const string& name) : KDataDriver(name) {
        m_is_python_object = true;
    }

    py::function get_py_override(const char* name) const {
        return py::get_override(static_cast<const KDataDriver*>(this), name);
    }

    bool _init() override {
        PYBIND11_OVERLOAD(bool, KDataDriver, _init, );
    }

    bool isIndexFirst() override {
        PYBIND11_OVERLOAD_PURE(bool, KDataDriver, isIndexFirst, );
    }

    bool canParallelLoad() override {
        return false;
    }

    size_t getCount(const string& market, const string& code, const KQuery::KType& kType) override {
        PYBIND11_OVERLOAD(size_t, KDataDriver, getCount, market, code, kType);
    }

    bool getIndexRangeByDate(const string& market, const string& code, const KQuery& query,
                             size_t& out_start, size_t& out_end) override {
        py::gil_scoped_acquire gil;
        py::function py_func = get_py_override("_getIndexRangeByDate");
        if (!py_func) {
            return KDataDriver::getIndexRangeByDate(market, code, query, out_start, out_end);
        }

        py::tuple t = py_func(market, code, query);
        if (len(t) != 2) {
            PyErr_SetObject(PyExc_ValueError,
                            py::str("expected 2-item tuple in call to _getIndexRangeByDate; got {}")
                              .format(t)
                              .ptr());
            throw py::error_already_set();
        }

        long start_ix = 0, end_ix = 0;
        if (py::isinstance<py::int_>(t[0]) && py::isinstance<py::int_>(t[1])) {
            start_ix = t[0].cast<py::int_>();
            end_ix = t[1].cast<py::int_>();
        } else {
            throw std::logic_error("expected 2-int tuple in call to _getIndexRangeByDate");
        }

        if (start_ix < 0 || end_ix < 0) {
            throw std::logic_error("startix or endix must be >= zero!");
        }

        out_start = (size_t)start_ix;
        out_end = (size_t)end_ix;
        return true;
    }

    KRecordList getKRecordList(const string& market, const string& code,
                               const KQuery& query) override {
        py::gil_scoped_acquire gil;
        py::function py_func = get_py_override("_getKRecordList");
        if (!py_func) {
            return KDataDriver::getKRecordList(market, code, query);
        }

        py::list py_list = py_func(market, code, query);
        return python_list_to_vector<KRecord>(py_list);
    }

    TimeLineList getTimeLineList(const string& market, const string& code,
                                 const KQuery& query) override {
        py::gil_scoped_acquire gil;
        py::function py_func = get_py_override("_getTimeLineList");
        if (!py_func) {
            return KDataDriver::getTimeLineList(market, code, query);
        }

        py::list py_list = py_func(market, code, query);
        return python_list_to_vector<TimeLineRecord>(py_list);
    }

    TransList getTransList(const string& market, const string& code, const KQuery& query) override {
        py::gil_scoped_acquire gil;
        py::function py_func = get_py_override("_getTransList");
        if (!py_func) {
            return KDataDriver::getTransList(market, code, query);
        }

        py::list py_list = py_func(market, code, query);
        return python_list_to_vector<TransRecord>(py_list);
    }
};