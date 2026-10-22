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
#include <hikyuu/global/sysinfo.h>
#include "ioredirect.h"

namespace hku {

// Whether any system of the list may call back into Python, either by itself, by one of its
// parts, or through a sub-system at any depth
inline bool sys_list_has_python_part(const vector<SystemPtr>& sys_list) {
    for (const auto& sys : sys_list) {
        if (sys && sys->hasPythonPart()) {
            return true;
        }
    }
    return false;
}

inline bool tm_list_has_python_part(const vector<TMPtr>& tm_list) {
    for (const auto& tm : tm_list) {
        if (tm && tm->hasPythonPart()) {
            return true;
        }
    }
    return false;
}

// Always release the GIL: the callees may run tasks on the pool workers and the pybind11 (>= 3.0)
// trampolines re-acquire it per callback, so holding it here would deadlock them. In Jupyter the
// C++ log streams are switched to the native ones for the python-free inputs only.
// pybind11 types are compiled with hidden visibility, so the class holding them must be marked
// the same way, otherwise GCC warns that it is declared with greater visibility than its fields.
#if HKU_OS_LINUX
class __attribute__((visibility("hidden"))) ScopeGilRelease {
#else
class ScopeGilRelease {
#endif
public:
    explicit ScopeGilRelease(bool python_free) {
        if (python_free && pythonInJupyter()) {
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
