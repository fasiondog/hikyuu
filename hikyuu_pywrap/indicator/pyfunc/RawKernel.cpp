/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  RawKernel.cpp
 */

#include "RawKernel.h"

namespace hku {
namespace pyfunc {

RawKernel::RawKernel(pybind11::object func, size_t n_in, size_t n_out)
: PyKernel(std::move(func), n_in, n_out) {
    // only an object exposing its address (a numba cfunc) qualifies
    if (!pybind11::hasattr(this->func(), "address")) {
        throw pybind11::type_error(
          "np_apply: the raw call mode needs a native function exposing address (a numba cfunc); "
          "drop the hint to call a ctypes function");
    }
    m_address = pybind11::cast<uintptr_t>(this->func().attr("address"));
    if (m_address == 0) {
        throw pybind11::value_error("np_apply: the raw call mode needs a non zero address");
    }

    // the argument count is still checked when the ctypes signature is readable
    if (pybind11::hasattr(this->func(), "ctypes")) {
        pybind11::object argtypes = this->func().attr("ctypes").attr("argtypes");
        if (!argtypes.is_none()) {
            const size_t count = static_cast<size_t>(pybind11::len(argtypes));
            if (count != n_in + n_out + 1) {
                throw pybind11::value_error(
                  fmt::format("np_apply: the cfunc expects {} argument(s), but {} input(s) + {} "
                              "result(s) + 1 were declared",
                              count, n_in, n_out));
            }
        }
    }

    m_dispatch = detail::rawDispatch(n_in, n_out);
    if (m_dispatch == nullptr) {
        throw pybind11::value_error(
          fmt::format("np_apply: the raw call mode supports at most {} input series, got {} with "
                      "{} result set(s)",
                      MAX_INPUTS, n_in, n_out));
    }
}

void RawKernel::run(const BufferSet& bufs) const {
    // the call is entirely native: the GIL is dropped for its whole duration
    pybind11::gil_scoped_release release;
    m_dispatch(m_address, bufs);
}

}  // namespace pyfunc
}  // namespace hku
