/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-01-04
 *      Author: fasiondog
 */

#pragma once

#include <hikyuu/trade_sys/portfolio/Portfolio.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

#ifdef _MSC_VER
#define HIDDEN
#else
#define HIDDEN __attribute__((visibility("hidden")))
#endif

class HIDDEN PyPortfolio : public Portfolio {
    PY_CLONE(PyPortfolio, Portfolio)

public:
    using Portfolio::Portfolio;
    PyPortfolio(const Portfolio& base);
    virtual ~PyPortfolio() override;

    virtual string str() const override;
    virtual void _reset() override;

    virtual void _readyForRun() override;

    virtual void _runMoment(const Datetime& date, const Datetime& nextCycle, bool adjust) override;
    virtual void _runMomentOnOpen(const Datetime& date, const Datetime& nextCycle,
                                  bool adjust) override;
    virtual void _runMomentOnClose(const Datetime& date, const Datetime& nextCycle,
                                   bool adjust) override;

    virtual json lastSuggestion() const override;

public:
    void set_tm(py::object tm);
    void set_se(py::object se);
    void set_af(py::object af);
};