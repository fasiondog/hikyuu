/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  RawKernel.h - the raw pointer kernel of np_apply
 */

#pragma once
#ifndef HIKYUU_PYWRAP_INDICATOR_PYFUNC_RAWKERNEL_H
#define HIKYUU_PYWRAP_INDICATOR_PYFUNC_RAWKERNEL_H

#include <cstdint>
#include <utility>

#include "PyKernel.h"

namespace hku {
namespace pyfunc {
namespace detail {

/** Call a native function through its address; a mismatched signature crashes (opt-in by design) */
template <typename... Args>
inline void invokeRawFunction(uintptr_t address, Args... args) {
    reinterpret_cast<void (*)(Args...)>(address)(args...);
}

template <size_t NIn, size_t I>
inline auto rawArgument(const BufferSet& bufs) {
    if constexpr (I < NIn) {
        return bufs.in[I];
    } else {
        return bufs.out[I - NIn];
    }
}

template <size_t NIn, size_t NOut, size_t... I>
inline void callRawOrdered(uintptr_t address, const BufferSet& bufs, std::index_sequence<I...>) {
    invokeRawFunction(address, rawArgument<NIn, I>(bufs)..., bufs.total);
}

template <size_t NIn, size_t NOut>
inline void callRaw(uintptr_t address, const BufferSet& bufs) {
    callRawOrdered<NIn, NOut>(address, bufs, std::make_index_sequence<NIn + NOut>{});
}

/** The raw call of one (input number, output number) pair, selected once at construction */
typedef void (*RawDispatch)(uintptr_t, const BufferSet&);
typedef RawDispatch (*RawDispatchPicker)(size_t);

template <size_t NIn, size_t... I>
RawDispatch rawDispatchOut(size_t n_out, std::index_sequence<I...>) {
    static const RawDispatch table[] = {(&callRaw<NIn, I + 1>)...};
    return n_out >= 1 && n_out <= sizeof...(I) ? table[n_out - 1] : nullptr;
}

template <size_t NIn>
RawDispatch rawDispatchOut(size_t n_out) {
    return rawDispatchOut<NIn>(n_out, std::make_index_sequence<MAX_RESULT_NUM>{});
}

template <size_t... I>
RawDispatch rawDispatchIn(size_t n_in, size_t n_out, std::index_sequence<I...>) {
    static const RawDispatchPicker table[] = {(&rawDispatchOut<I + 1>)...};
    return n_in >= 1 && n_in <= sizeof...(I) ? table[n_in - 1](n_out) : nullptr;
}

inline RawDispatch rawDispatch(size_t n_in, size_t n_out) {
    return rawDispatchIn(n_in, n_out, std::make_index_sequence<MAX_INPUTS>{});
}

}  // namespace detail

/** A numba `cfunc` called through its address with `in0, ..., out0, ..., total`; the GIL is
 *  dropped for the call and the signature is not validated (`hints={"raw": True}` opt-in) */
class RawKernel : public PyKernel {
public:
    RawKernel(pybind11::object func, size_t n_in, size_t n_out);

    void run(const BufferSet& bufs) const override;

private:
    uintptr_t m_address{0};
    detail::RawDispatch m_dispatch{nullptr};
};

}  // namespace pyfunc
}  // namespace hku

#endif  // HIKYUU_PYWRAP_INDICATOR_PYFUNC_RAWKERNEL_H
