/*
    pybind11/iostream.h -- Tools to assist with redirecting cout and cerr to Python

    Copyright (c) 2017 Henry F. Schreiner

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/
/*
 *      modified by: fasiondog
 *      modified from https://github.com/pybind/pybind11/include/pybind11/iostream.h
 */

#pragma once

#include <atomic>
#include <pybind11/iostream.h>
#include <hikyuu/DataType.h>

namespace hku {

void open_ostream_to_python();
void close_ostream_to_python();

class OStreamToPython final {
    friend void open_ostream_to_python();
    friend void close_ostream_to_python();

public:
    explicit OStreamToPython(bool open) : m_old_opened(ms_opened.load(std::memory_order_acquire)) {
        _switchRedirect(open);
    }

    OStreamToPython(const OStreamToPython&) = delete;
    OStreamToPython& operator=(const OStreamToPython&) = delete;

    ~OStreamToPython() {
        _switchRedirect(m_old_opened);
    }

private:
    static void _switchRedirect(bool open) {
        bool expected = !open;
        if (ms_opened.compare_exchange_strong(expected, open, std::memory_order_acq_rel,
                                              std::memory_order_acquire)) {
            if (open) {
                ms_io_redirect.enter();
            } else {
                ms_io_redirect.exit();
            }
        }
    }

    bool m_old_opened;

private:
    static pybind11::detail::OstreamRedirect ms_io_redirect;
    static std::atomic<bool> ms_opened;
};

}  // namespace hku
