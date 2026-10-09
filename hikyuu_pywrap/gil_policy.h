/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-10-09
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_PYWRAP_GIL_POLICY_H
#define HKU_PYWRAP_GIL_POLICY_H

#include <memory>
#include <pybind11/pybind11.h>
#include <hikyuu/trade_sys/system/System.h>
#include <hikyuu/trade_manage/TradeManagerBase.h>
#include "ioredirect.h"

namespace hku {

// pybind trampolines call Python overrides without acquiring the GIL, so the GIL may only be
// released when no Python-implemented part is involved.
template <typename... Parts>
inline bool any_python_part(const Parts&... parts) {
    return ((parts && parts->isPythonObject()) || ...);
}

// A system may call back into Python via any of its parts, or being a Python subclass itself.
inline bool system_has_python_part(const SystemPtr& sys) {
    if (!sys) {
        return false;
    }
    return sys->isPythonObject() ||
           any_python_part(sys->getSG(), sys->getST(), sys->getTP(), sys->getMM(), sys->getPG(),
                           sys->getSP(), sys->getCN(), sys->getEV(), sys->getTM());
}

inline bool tm_list_has_python_part(const vector<TMPtr>& tm_list) {
    for (const auto& tm : tm_list) {
        if (tm && tm->isPythonObject()) {
            return true;
        }
    }
    return false;
}

// Keep the C++ logging off the Python streams and release the GIL for the current scope when
// the inputs are python-free; otherwise keep the GIL held (pybind trampolines require it).
class ScopeGilRelease {
public:
    explicit ScopeGilRelease(bool python_free) {
        if (python_free) {
            m_stream_guard = std::make_unique<OStreamToPython>(false);
            m_gil_guard = std::make_unique<py::gil_scoped_release>();
        }
    }

    ScopeGilRelease(const ScopeGilRelease&) = delete;
    ScopeGilRelease& operator=(const ScopeGilRelease&) = delete;

private:
    std::unique_ptr<OStreamToPython> m_stream_guard;
    std::unique_ptr<py::gil_scoped_release> m_gil_guard;
};

}  // namespace hku

#endif  // HKU_PYWRAP_GIL_POLICY_H
