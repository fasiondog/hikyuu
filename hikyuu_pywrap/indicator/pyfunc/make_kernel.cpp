/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  make_kernel.cpp
 */

#include "make_kernel.h"

#include <set>

#include "ArrayKernel.h"
#include "CFuncKernel.h"
#include "PandasKernel.h"
#include "PyFuncIndicatorImp.h"
#include "RawKernel.h"
#include "UfuncKernel.h"

namespace hku {
namespace pyfunc {

/** Reject the hints that are not supported; "raw" switches to the native call mode */
static void checkHints(const pybind11::object& hints, KernelSpec& spec) {
    if (hints.is_none()) {
        return;
    }
    if (!pybind11::isinstance<pybind11::dict>(hints)) {
        throw pybind11::type_error("np_apply: hints must be a dict");
    }
    pybind11::dict options = pybind11::cast<pybind11::dict>(hints);

    static const std::set<std::string> allowed{"contiguous", "raw"};
    std::set<std::string> unknown;
    for (const auto& item : options) {
        const std::string name = pybind11::cast<std::string>(item.first);
        if (allowed.find(name) == allowed.end()) {
            unknown.insert(name);
        }
    }
    if (!unknown.empty()) {
        std::string names;
        for (const auto& name : unknown) {
            names += (names.empty() ? "" : ", ") + name;
        }
        throw pybind11::value_error(fmt::format("np_apply: unknown hint(s): {}", names));
    }

    if (options.contains("raw")) {
        spec.raw = pybind11::cast<bool>(options["raw"]);
    }
    if (options.contains("contiguous")) {
        spec.contiguous = pybind11::cast<bool>(options["contiguous"]);
    }
}

/** The input indicators of one call: a single one, or a sequence bridged by WEAVE */
static std::vector<Indicator> asRefs(const pybind11::object& ref) {
    if (pybind11::isinstance<Indicator>(ref)) {
        return {pybind11::cast<Indicator>(ref)};
    }
    if (!pybind11::isinstance<pybind11::list>(ref) && !pybind11::isinstance<pybind11::tuple>(ref)) {
        throw pybind11::type_error(
          "np_apply: ref must be an Indicator or a list / tuple of Indicators");
    }

    const size_t count = static_cast<size_t>(pybind11::len(ref));
    if (count == 0) {
        throw pybind11::value_error("np_apply: ref must not be an empty list / tuple");
    }
    if (count > MAX_INPUTS) {
        throw pybind11::value_error(
          fmt::format("np_apply: at most {} inputs are supported", MAX_INPUTS));
    }

    std::vector<Indicator> refs;
    refs.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        pybind11::object item = sequenceItem(ref, i);
        if (!pybind11::isinstance<Indicator>(item)) {
            throw pybind11::type_error(fmt::format("np_apply: ref[{}] is not an Indicator", i));
        }
        refs.push_back(pybind11::cast<Indicator>(item));
    }
    return refs;
}

static Indicator weaveRefs(const std::vector<Indicator>& refs) {
    switch (refs.size()) {
        case 2:
            return WEAVE(refs[0], refs[1]);
        case 3:
            return WEAVE(refs[0], refs[1], refs[2]);
        case 4:
            return WEAVE(refs[0], refs[1], refs[2], refs[3]);
        case 5:
            return WEAVE(refs[0], refs[1], refs[2], refs[3], refs[4]);
        case 6:
            return WEAVE(refs[0], refs[1], refs[2], refs[3], refs[4], refs[5]);
        default:
            break;
    }
    HKU_THROW("np_apply: unsupported input number {}", refs.size());
}

PyKernelPtr makeKernel(const pybind11::object& func, size_t input_num, const KernelSpec& spec) {
    // A per element Python callback holds the GIL for the whole call and cannot be vectorized
    if (typeName(func) == "vectorize") {
        throw pybind11::type_error(
          "np_apply: np.vectorize/np.frompyfunc run a Python callback per element and hold the "
          "GIL");
    }

    pybind11::object ufunc_type = pybind11::module_::import("numpy").attr("ufunc");
    if (spec.raw) {
        if (pybind11::isinstance(func, ufunc_type)) {
            throw pybind11::type_error("np_apply: the raw call mode cannot wrap a ufunc");
        }
        if (spec.view == "pandas") {
            throw pybind11::type_error(
              "np_apply: view=\"pandas\" is not supported by the raw call mode");
        }
        if (spec.with_dates) {
            throw pybind11::type_error(
              "np_apply: with_dates is not supported by the raw call mode");
        }
        return std::make_shared<RawKernel>(func, input_num, spec.result_num);
    }

    if (pybind11::isinstance(func, ufunc_type)) {
        if (spec.with_dates) {
            throw pybind11::type_error("np_apply: a ufunc cannot receive dates, remove with_dates");
        }
        if (ufuncLoopKind(func, input_num, spec.result_num) == UfuncLoopKind::NO_ARROW) {
            // a numba DUFunc without format-char signatures is still callable as a plain function
            return std::make_shared<ArrayKernel>(func, input_num, spec.result_num, false);
        }
        return std::make_shared<UfuncKernel>(func, input_num, spec.result_num);
    }

    pybind11::object cfunc_type = pybind11::module_::import("ctypes").attr("_CFuncPtr");
    if (pybind11::isinstance(func, cfunc_type)) {
        if (spec.view == "pandas") {
            throw pybind11::type_error(
              "np_apply: view=\"pandas\" is not supported by the cfunc kernel");
        }
        if (spec.with_dates) {
            throw pybind11::type_error("np_apply: with_dates is not supported by the cfunc kernel");
        }
        return std::make_shared<CFuncKernel>(func, input_num, spec.result_num);
    }

    if (!PyCallable_Check(func.ptr())) {
        throw pybind11::type_error("np_apply: func must be callable");
    }

    if (spec.view == "pandas") {
        return std::make_shared<PandasKernel>(func, input_num, spec.result_num);
    }
    return std::make_shared<ArrayKernel>(func, input_num, spec.result_num, spec.with_dates);
}

static Indicator buildApply(std::vector<Indicator> refs, const pybind11::object& func,
                            int64_t resultNum, std::optional<int64_t> discard,
                            const std::string& view, bool withDates, bool stateless,
                            const std::string& name, const pybind11::object& hints) {
    if (view != "numpy" && view != "pandas") {
        throw pybind11::value_error(
          fmt::format("np_apply: view must be \"numpy\" or \"pandas\", got {}", view));
    }
    if (resultNum < 1 || resultNum > static_cast<int64_t>(MAX_RESULT_NUM)) {
        throw pybind11::value_error(
          fmt::format("np_apply: result_num must be 1..{}, got {}", MAX_RESULT_NUM, resultNum));
    }
    if (discard && *discard < 0) {
        throw pybind11::value_error(
          fmt::format("np_apply: discard must be non-negative, got {}", *discard));
    }

    KernelSpec spec;
    spec.name = name;
    spec.result_num = static_cast<size_t>(resultNum);
    if (discard) {
        spec.discard = static_cast<size_t>(*discard);
    }
    spec.view = view;
    spec.with_dates = withDates;
    spec.stateless = stateless;
    checkHints(hints, spec);

    // Every result set of every input is one series of the call
    size_t input_num = 0;
    for (const auto& item : refs) {
        input_num += item.getResultNumber();
    }

    PyKernelPtr kernel = makeKernel(func, input_num, spec);
    auto node = std::make_shared<PyFuncIndicatorImp>(kernel, spec);
    Indicator ind(node);
    return refs.size() == 1 ? ind(refs[0]) : ind(weaveRefs(refs));
}

Indicator npApply(const Indicator& ref, const pybind11::object& func, int64_t resultNum,
                  std::optional<int64_t> discard, const std::string& view, bool withDates,
                  bool stateless, const std::string& name, const pybind11::object& hints) {
    return buildApply({ref}, func, resultNum, discard, view, withDates, stateless, name, hints);
}

Indicator npApply(const pybind11::object& ref, const pybind11::object& func, int64_t resultNum,
                  std::optional<int64_t> discard, const std::string& view, bool withDates,
                  bool stateless, const std::string& name, const pybind11::object& hints) {
    return buildApply(asRefs(ref), func, resultNum, discard, view, withDates, stateless, name,
                      hints);
}

}  // namespace pyfunc
}  // namespace hku
