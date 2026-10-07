/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-01-04
 *      Author: fasiondog
 */
#pragma once

#include <hikyuu/trade_sys/system/System.h>
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>
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
    PySystem() : System() {
        m_is_python_object = true;
    }

    PySystem(const string& name) : System(name) {
        m_is_python_object = true;
    }

    using System::System;
    PySystem(const System& base);
    virtual ~PySystem() override;

    virtual void run(const KData& kdata, bool reset, bool resetAll) override;
    virtual MomentResult runMoment(const Datetime& datetime) override;
    virtual MomentResult runMomentOnOpen(const Datetime& datetime) override;
    virtual MomentResult runMomentOnClose(const Datetime& datetime) override;
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
};

class HIDDEN PyMultiSystem : public MultiSystem {
    PY_CLONE(PyMultiSystem, System)

public:
    PyMultiSystem() : MultiSystem() {
        m_is_python_object = true;
    }
    PyMultiSystem(const string& name) : MultiSystem(name) {
        m_is_python_object = true;
    }
    using MultiSystem::MultiSystem;
    PyMultiSystem(const MultiSystem& base);
    virtual ~PyMultiSystem() override;

    virtual void run(const KData& kdata, bool reset, bool resetAll) override;
    virtual MomentResult runMoment(const Datetime& datetime) override;
    virtual MomentResult runMomentOnOpen(const Datetime& datetime) override;
    virtual MomentResult runMomentOnClose(const Datetime& datetime) override;
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
};