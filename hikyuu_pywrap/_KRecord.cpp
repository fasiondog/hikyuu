/*
 * _KRecord.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/KRecord_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

bool (*krecord_eq)(const KRecord&, const KRecord&) = operator==;

void export_KReord() {
    docstring_options doc_options(false);

    class_<KRecord>("KRecord", init<>())
            .def(init<const Datetime&>())
            .def(init<const Datetime&, price_t, price_t, price_t, price_t, price_t, price_t>())
            .def(self_ns::str(self))
            .def_readwrite("datetime", &KRecord::datetime)
            .def_readwrite("openPrice", &KRecord::openPrice)
            .def_readwrite("highPrice", &KRecord::highPrice)
            .def_readwrite("lowPrice", &KRecord::lowPrice)
            .def_readwrite("closePrice", &KRecord::closePrice)
            .def_readwrite("transAmount", &KRecord::transAmount)
            .def_readwrite("transCount", &KRecord::transCount)
            .def("__eq__", krecord_eq)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<KRecord>())
#endif
            ;

    KRecordList::const_reference (KRecordList::*KRecordList_at)(KRecordList::size_type) const = &KRecordList::at;
    void (KRecordList::*append)(const KRecord&) = &KRecordList::push_back;
    class_<KRecordList>("KRecordList")
            .def("__iter__", iterator<KRecordList>())
            .def("size", &KRecordList::size)
            .def("__len__", &KRecordList::size)
            .def("__getitem__", KRecordList_at, return_value_policy<copy_const_reference>())
            .def("append", append)
            ;

    register_ptr_to_python<KRecordListPtr>();
}

