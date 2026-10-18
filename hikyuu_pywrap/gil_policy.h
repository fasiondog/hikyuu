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

// Whether any of the parts is implemented in Python
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

inline bool sys_list_has_python_part(const vector<SystemPtr>& sys_list) {
    for (const auto& sys : sys_list) {
        if (system_has_python_part(sys)) {
            return true;
        }
    }
    return false;
}

inline bool tm_list_has_python_part(const vector<TMPtr>& tm_list) {
    for (const auto& tm : tm_list) {
        if (tm && tm->isPythonObject()) {
            return true;
        }
    }
    return false;
}

// Keep the C++ logging off the Python streams and release the GIL for the current scope.
// The GIL is always released: the callees may run the tasks on the pool workers, and the
// pybind11 (>= 3.0) trampolines acquire the GIL per callback, so the calling thread holding it
// would deadlock the workers against itself. The log streams are redirected to Python only for
// the python-free inputs: with Python parts the worker threads may log without holding the GIL,
// and the logs stay on the native streams.
// pybind11 types are compiled with hidden visibility, so the class holding them must be marked
// the same way, otherwise GCC warns that it is declared with greater visibility than its fields.
#if HKU_OS_LINUX
class __attribute__((visibility("hidden"))) ScopeGilRelease {
#else
class ScopeGilRelease {
#endif
public:
    explicit ScopeGilRelease(bool python_free) {
        if (python_free) {
            m_stream_guard = std::make_unique<OStreamToPython>(false);
        }
        m_gil_guard = std::make_unique<pybind11::gil_scoped_release>();
    }

    ScopeGilRelease(const ScopeGilRelease&) = delete;
    ScopeGilRelease& operator=(const ScopeGilRelease&) = delete;

private:
    std::unique_ptr<OStreamToPython> m_stream_guard;
    std::unique_ptr<pybind11::gil_scoped_release> m_gil_guard;
};

}  // namespace hku

#endif  // HKU_PYWRAP_GIL_POLICY_H
