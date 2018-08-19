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
    docstring_options doc_options(false);

    class_<KData>("KData", init<>())
            .def(init<const KData&>())
            .def(init<const Stock&, const KQuery&>())
            //.def(self_ns::str(self))
            .def("__str__", &KData::toString)
            .add_property("startPos", &KData::startPos)
            .add_property("endPos", &KData::endPos)
            .add_property("lastPos", &KData::lastPos)

            .def("getDatetimeList", &KData::getDatetimeList)
            .def("getKRecord", &KData::getKRecord)
            .def("get", &KData::getKRecord)
            .def("getKRecordByDate", &KData::getKRecordByDate)
            .def("getByDate", &KData::getKRecordByDate)
            .def("_getPos", &KData::getPos) //python中需要将Null的情况改写为None

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


