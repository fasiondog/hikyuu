/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-01-04
 *      Author: fasiondog
 */
#pragma once

#include <hikyuu/trade_sys/system/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

#ifdef _MSC_VER
#define HIDDEN
#else
#define HIDDEN __attribute__((visibility("hidden")))
#endif

class HIDDEN PySystem : public System {
    PY_CLONE(PySystem, System)

public:
    using System::System;
    PySystem(const System& base);

    virtual void run(const KData& kdata, bool reset, bool resetAll) override;
    virtual TradeRecord runMoment(const Datetime& datetime) override;
    virtual TradeRecord runMomentOnOpen(const Datetime& datetime) override;
    virtual TradeRecord runMomentOnClose(const Datetime& datetime) override;
    virtual void readyForRun() override;
    virtual void _reset() override;
    virtual void _forceResetAll() override;
    virtual string str() const override;

public:
    void set_mm(py::object mm);
    void set_ev(py::object ev);
    void set_cn(py::object cn);
    void set_sg(py::object sg);
    void set_st(py::object st);
    void set_tp(py::object tp);
    void set_pg(py::object pg);
    void set_sp(py::object sp);
    void set_tm(py::object tm);

private:
    py::object m_py_mm;
    py::object m_py_ev;
    py::object m_py_cn;
    py::object m_py_sg;
    py::object m_py_st;
    py::object m_py_tp;
    py::object m_py_pg;
    py::object m_py_sp;
    py::object m_py_tm;  // 不能放最前面？
};