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
    DatetimeList::const_reference (DatetimeList::*datetimeList_at)(DatetimeList::size_type) const =
      &DatetimeList::at;
    void (DatetimeList::*datetimelist_append)(const DatetimeList::value_type& val) =
      &DatetimeList::push_back;
    class_<DatetimeList>("DatetimeList", "日期序列，对应C++中的std::vector<Datetime>")
      .def("__iter__", iterator<DatetimeList>())
      .def("size", &DatetimeList::size)
      .def("__len__", &DatetimeList::size)
      .def("append", datetimelist_append, "向列表末端加入元素")
      .def("get", datetimeList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<DatetimeList>())
#endif
      ;

    PriceList::const_reference (PriceList::*PriceList_at)(PriceList::size_type) const =
      &PriceList::at;
    void (PriceList::*PriceList_append)(const PriceList::value_type& val) = &PriceList::push_back;
    class_<PriceList>("PriceList")
      .def("__iter__", iterator<PriceList>())
      .def("size", &PriceList::size)
      .def("__len__", &PriceList::size)
      .def("append", PriceList_append)
      .def("get", PriceList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<PriceList>())
#endif
      ;

    StringList::const_reference (StringList::*StringList_at)(StringList::size_type) const =
      &StringList::at;
    void (StringList::*StringList_append)(const StringList::value_type& val) =
      &StringList::push_back;
    class_<StringList>("StringList")
      .def("__iter__", iterator<StringList>())
      .def("size", &StringList::size)
      .def("__len__", &StringList::size)
      .def("append", StringList_append)
      .def("get", StringList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<StringList>())
#endif
      ;

    def("toPriceList", toPriceList);

#if defined(_MSVC_VER)
    def("isnan", std::isnan<price_t>);
    def("isinf", std::isinf<price_t>);
#else
    def("isnan", isnan_func);
    def("isinf", isinf_func);
#endif
}
