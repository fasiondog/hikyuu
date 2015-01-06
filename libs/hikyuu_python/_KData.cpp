/*
 * _KData.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/KData_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

void export_KData() {
    class_<KData>("KData", init<>())
            .def(init<const KData&>())
            .def(init<const Stock&, const KQuery&>())
            .def(self_ns::str(self))
            .add_property("startPos", &KData::startPos)
            .add_property("endPos", &KData::endPos)
            .add_property("lastPos", &KData::lastPos)

            .def("getDatetimeList", &KData::getDatetimeList)
            .def("getKRecord", &KData::getKRecord)
            .def("getKRecordByDate", &KData::getKRecordByDate)

            .def("size", &KData::size)
            .def("empty", &KData::empty)
            .def("getQuery", &KData::getQuery)
            .def("getStock", &KData::getStock)
            .def("tocsv", &KData::tocsv)

            .def("__len__", &KData::size)
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<KData>())
#endif
            ;

}


