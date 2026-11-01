/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  UfuncKernel.cpp
 */

#include "UfuncKernel.h"

namespace hku {
namespace pyfunc {

UfuncLoopKind ufuncLoopKind(const pybind11::object& ufunc, size_t n_in, size_t n_out) {
    const char value_char = valueFormatChar();
    bool has_arrow = false;

    pybind11::object types = ufunc.attr("types");
    for (auto item : types) {
        std::string signature = pybind11::cast<std::string>(item);
        const size_t arrow = signature.find("->");
        if (arrow == std::string::npos) {
            continue;
        }
        has_arrow = true;

        const std::string left = signature.substr(0, arrow);
        const std::string right = signature.substr(arrow + 2);
        if (left.size() != n_in || right.size() != n_out) {
            continue;
        }
        if (left.find_first_not_of(value_char) == std::string::npos &&
            right.find_first_not_of(value_char) == std::string::npos) {
            return UfuncLoopKind::DTYPE;
        }
    }

    return has_arrow ? UfuncLoopKind::OBJECT : UfuncLoopKind::NO_ARROW;
}

UfuncKernel::UfuncKernel(pybind11::object func, size_t n_in, size_t n_out)
: PyKernel(std::move(func), n_in, n_out) {
    const size_t nin = pybind11::cast<size_t>(this->func().attr("nin"));
    if (nin != n_in) {
        throw pybind11::value_error(
          fmt::format("np_apply: the ufunc takes {} input(s), but {} were given", nin, n_in));
    }
    const size_t nout = pybind11::cast<size_t>(this->func().attr("nout"));
    if (nout != n_out) {
        throw pybind11::value_error(
          fmt::format("np_apply: the ufunc produces {} result(s), but result_num={}", nout, n_out));
    }
    if (ufuncLoopKind(this->func(), n_in, n_out) != UfuncLoopKind::DTYPE) {
        throw pybind11::type_error(
          fmt::format("np_apply: the ufunc {} only provides object loops, it cannot be vectorized",
                      typeName(this->func())));
    }
}

void UfuncKernel::run(const BufferSet& bufs) const {
    pybind11::tuple args(bufs.in.size());
    for (size_t i = 0; i < bufs.in.size(); ++i) {
        args[i] = inputView(bufs, i);
    }

    pybind11::object out;
    if (bufs.out.size() == 1) {
        out = outputView(bufs, 0);
    } else {
        pybind11::tuple outs(bufs.out.size());
        for (size_t i = 0; i < bufs.out.size(); ++i) {
            outs[i] = outputView(bufs, i);
        }
        out = outs;
    }

    pybind11::dict kwargs;
    kwargs["out"] = out;
    callPython(func(), args, kwargs);
}

}  // namespace pyfunc
}  // namespace hku
