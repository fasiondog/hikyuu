/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  bind.cpp - the np_apply binding
 */

#include <optional>

#include <hikyuu/indicator/Indicator.h>

#include "../../pybind_utils.h"
#include "make_kernel.h"

namespace py = pybind11;
using namespace hku;

void export_Indicator_pyfunc(py::module& m) {
    m.def(
      "np_apply",
      [](const py::object& ref, const py::object& func, int64_t resultNum,
         std::optional<int64_t> discard, const std::string& view, bool withDates, bool stateless,
         const std::string& name, const py::object& hints) {
          return pyfunc::npApply(ref, func, resultNum, discard, view, withDates, stateless, name,
                                 hints);
      },
      py::arg("ref"), py::arg("func"), py::kw_only(), py::arg("result_num") = 1,
      py::arg("discard") = py::none(), py::arg("view") = "numpy", py::arg("with_dates") = false,
      py::arg("stateless") = false, py::arg("name") = "APPLY", py::arg("hints") = py::none(),
      R"(np_apply(ref, func, *, result_num=1, discard=None, view="numpy", with_dates=False,
             stateless=False, name="APPLY", hints=None) -> Indicator

    Wrap a numpy / pandas / ctypes callable as an indicator operating on the whole series.

    The callable runs once per calculation with the whole series, never per element.

    :param ref: the input Indicator, or a list / tuple of at most 6 Indicators (bridged by
        WEAVE); every input result set is passed as one array
    :param func: the callable: a numpy ufunc (including a numba DUFunc), a plain callable
        receiving read only numpy arrays, a pandas callable when ``view="pandas"``, or a ctypes /
        numba cfunc receiving the raw buffers (in..., out..., total); the buffer arguments must be
        declared as ctypes pointers or c_void_p and the length as a non pointer
    :param int result_num: the number of the result sets, it must match the returned results
    :param int discard: the number of the leading points to discard; None inherits the input
    :param str view: "numpy" (the default) or "pandas" (Series / DataFrame input)
    :param bool with_dates: pass the date list as the first argument of a plain callable
    :param bool stateless: declare the callable as a pure function; otherwise two nodes wrapping
        it are never shared
    :param str name: the indicator name
    :param dict hints: {"contiguous": bool, "raw": bool}; "raw" calls a numba cfunc through its
        address with "in..., out..., total" and drops the GIL for the whole call. Only the
        argument count is checked (when the cfunc exposes its ctypes signature), the pointer types
        and the order are not, so a mismatch crashes the interpreter; the hint is refused for a
        ctypes function, a ufunc, the pandas view and with_dates
    :rtype: Indicator
    :raises TypeError: the callable shape is not supported (np.vectorize, object loop ufunc, ...)
    :raises ValueError: result_num, discard, view or the returned results are invalid
    :note The output length must equal the input length, an empty input never calls func, NaN is
        Null and is not replaced, and pickling / incremental calculation are not supported
    :example:

        >>> ind = np_apply(CLOSE(), np.log)
        >>> ind = CLOSE().apply(lambda a: a - a.mean())
        >>> ind = np_apply((HIGH(), LOW()), np.maximum, view="numpy")
    )");
}
