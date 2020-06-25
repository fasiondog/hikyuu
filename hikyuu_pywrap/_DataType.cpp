/*
 * _DataType.cpp
 *
 *  Created on: 2012-9-29
 *      Author: fasiondog
 */

#include <cmath>
#include <boost/python.hpp>
#include <hikyuu/DataType.h>
#include <hikyuu/serialization/Datetime_serialization.h>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include "pybind_utils.h"
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

PriceList toPriceList(object o) {
    PriceList result;
    size_t total = extract<size_t>(o.attr("__len__")());
    for (size_t i = 0; i < total; ++i) {
        result.push_back(extract<price_t>(o.attr("__getitem__")(i)));
    }
    return result;
}

#if !defined(_MSVC_VER)
bool (*isnan_func)(price_t) = std::isnan;
bool (*isinf_func)(price_t) = std::isinf;
#endif

void export_DataType() {
    VECTOR_TO_PYTHON_CONVERT(DatetimeList);
    VECTOR_TO_PYTHON_CONVERT(PriceList);
    VECTOR_TO_PYTHON_CONVERT(StringList);

    def("toPriceList", toPriceList);

#if defined(_MSVC_VER)
    def("isnan", std::isnan<price_t>);
    def("isinf", std::isinf<price_t>);
#else
    def("isnan", isnan_func);
    def("isinf", isinf_func);
#endif
}
