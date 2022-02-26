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
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
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

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_DataType() {
    class_<DatetimeList>("DatetimeList")
      .def(vector_indexing_suite<DatetimeList>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<DatetimeList>())
#endif
      ;

    class_<PriceList>("PriceList")
      .def(vector_indexing_suite<PriceList>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<PriceList>())
#endif
      ;

    class_<StringList>("StringList")
      .def(vector_indexing_suite<StringList>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<StringList>())
#endif
      ;

    def("toPriceList", toPriceList, "将 python list/tuple/np.arry 对象转化为 PriceList 对象");

#if defined(_MSVC_VER)
    def("isnan", std::isnan<price_t>, "是否为非数字");
    def("isinf", std::isinf<price_t>, "是否是无穷大或无穷小");
#else
    def("isnan", isnan_func, "是否为非数字");
    def("isinf", isinf_func, "是否是无穷大或无穷小");
#endif
}
