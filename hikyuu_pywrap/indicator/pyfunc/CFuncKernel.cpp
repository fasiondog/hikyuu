/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  CFuncKernel.cpp
 */

#include "CFuncKernel.h"

namespace hku {
namespace pyfunc {

CFuncKernel::CFuncKernel(pybind11::object func, size_t n_in, size_t n_out)
: PyKernel(std::move(func), n_in, n_out) {
    if (!pybind11::hasattr(this->func(), "argtypes")) {
        throw pybind11::type_error("np_apply: the cfunc must expose argtypes for validation");
    }
    pybind11::object argtypes = this->func().attr("argtypes");
    if (argtypes.is_none() || !pybind11::isinstance<pybind11::sequence>(argtypes)) {
        throw pybind11::type_error("np_apply: the cfunc must expose argtypes for validation");
    }

    pybind11::object pointer_type = pybind11::module_::import("ctypes").attr("_Pointer");
    pybind11::object void_pointer = pybind11::module_::import("ctypes").attr("c_void_p");
    const size_t count = static_cast<size_t>(pybind11::len(argtypes));
    if (count != n_in + n_out + 1) {
        throw pybind11::value_error(
          fmt::format("np_apply: the cfunc expects {} argument(s), but {} input(s) + {} result(s) "
                      "+ 1 were declared",
                      count, n_in, n_out));
    }

    m_pointer_args.resize(count);
    for (size_t i = 0; i < count; ++i) {
        pybind11::object argtype = sequenceItem(argtypes, i);
        int is_pointer = PyObject_IsSubclass(argtype.ptr(), pointer_type.ptr());
        if (is_pointer == -1) {
            // Not a class at all: a value argument, passed as an integer
            PyErr_Clear();
            is_pointer = 0;
        }
        m_pointer_args[i] = is_pointer != 0;
        if (i < count - 1) {
            // a value type would silently convert the address into its own value
            if (!m_pointer_args[i] && !argtype.equal(void_pointer)) {
                throw pybind11::type_error(fmt::format(
                  "np_apply: the cfunc argument {} must be a ctypes pointer or c_void_p, got {}", i,
                  typeName(argtype)));
            }
        } else if (m_pointer_args[i]) {
            // the length is passed as an integer, a pointer would turn it into an address
            throw pybind11::type_error(
              "np_apply: the cfunc length argument must not be declared as a pointer");
        }
    }
    m_argtypes = make_gil_safe(std::move(argtypes));
}

pybind11::object CFuncKernel::argument(size_t idx, const value_t* ptr,
                                       const pybind11::object& castFn) const {
    pybind11::object address = pybind11::int_(reinterpret_cast<uintptr_t>(ptr));
    if (!m_pointer_args[idx]) {
        // A c_void_p buffer slot or the length: passed as the plain integer address
        return address;
    }
    return castFn(address, sequenceItem(*m_argtypes, idx));
}

void CFuncKernel::run(const BufferSet& bufs) const {
    pybind11::object castFn = pybind11::module_::import("ctypes").attr("cast");

    pybind11::tuple args(bufs.in.size() + bufs.out.size() + 1);
    size_t k = 0;
    for (size_t i = 0; i < bufs.in.size(); ++i) {
        args[k] = argument(k, bufs.in[i], castFn);
        ++k;
    }
    for (size_t i = 0; i < bufs.out.size(); ++i) {
        args[k] = argument(k, bufs.out[i], castFn);
        ++k;
    }
    args[k] = pybind11::int_(bufs.total);

    callPython(func(), args);
}

}  // namespace pyfunc
}  // namespace hku
