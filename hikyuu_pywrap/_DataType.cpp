/*
 * _DataType.cpp
 *
 *  Created on: 2012-9-29
 *      Author: fasiondog
 */

#include <cmath>
#include <hikyuu/DataType.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if !defined(_MSVC_VER)
bool (*isnan_func)(price_t) = std::isnan;
bool (*isinf_func)(price_t) = std::isinf;
#endif

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_DataType(py::module& m) {
#if defined(_MSVC_VER)
    m.def("isnan", std::isnan<price_t>, "Whether it is not a number");
    m.def("isinf", std::isinf<price_t>, "Whether it is the infinity or the negative infinity");
#else
    m.def("isnan", isnan_func, "Whether it is not a number");
    m.def("isinf", isinf_func, "Whether it is the infinity or the negative infinity");
#endif

    m.def(
      "toPriceList", [](const py::sequence obj) { return python_list_to_vector<price_t>(obj); },
      "Convert a python list/tuple/np.array object to a PriceList object");
}
