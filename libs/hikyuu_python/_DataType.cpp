/*
 * _DataType.cpp
 *
 *  Created on: 2012-9-29
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/DataType.h>
#include <hikyuu/serialization/Datetime_serialization.h>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

PriceList toPriceList(object o) {
    PriceList result;
    size_t total = extract<size_t>(o.attr("__len__")());
    for (size_t i = 0; i < total ; ++i) {
        result.push_back(extract<price_t>(o.attr("__getitem__")(i)));
    }
    return result;
}

void export_DataType() {
    DatetimeList::const_reference (DatetimeList::*datetimeList_at)(DatetimeList::size_type) const = &DatetimeList::at;
    void (DatetimeList::*datetimelist_append)(const DatetimeList::value_type& val) = &DatetimeList::push_back;
    class_<DatetimeList>("DatetimeList")
            .def("__iter__", iterator<DatetimeList>())
            .def("size", &DatetimeList::size)
            .def("__len__", &DatetimeList::size)
            .def("append", datetimelist_append)
            .def("get", datetimeList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<DatetimeList>())
#endif
            ;

    PriceList::const_reference (PriceList::*PriceList_at)(PriceList::size_type) const = &PriceList::at;
    class_<PriceList>("PriceList")
            .def("__iter__", iterator<PriceList>())
            .def("size", &PriceList::size)
            .def("__len__", &PriceList::size)
            .def("get", PriceList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<PriceList>())
#endif
            ;

    StringList::const_reference (StringList::*StringList_at)(StringList::size_type) const = &StringList::at;
    class_<StringList>("StringList")
            .def("__iter__", iterator<StringList>())
            .def("size", &StringList::size)
            .def("__len__", &StringList::size)
            .def("get", StringList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<StringList>())
#endif
            ;


    def("toPriceList", toPriceList);

}


