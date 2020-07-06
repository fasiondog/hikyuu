/*
 * _KQuery.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/KQuery_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

// BOOST_PYTHON_FUNCTION_OVERLOADS(KQueryByDate_overload, KQueryByDate, 0, 4);
BOOST_PYTHON_FUNCTION_OVERLOADS(KQueryByIndex_overload, KQueryByIndex, 0, 4);

void export_KQuery() {
    // def("QueryByDate", KQueryByDate, KQueryByDate_overload());

    def("QueryByIndex", KQueryByIndex, KQueryByIndex_overload());

    scope in_Query =
      class_<KQuery>("Query", init<>())
        .def(init<int64, optional<int64, KQuery::KType, KQuery::RecoverType> >())
        .def(init<Datetime, optional<Datetime, KQuery::KType, KQuery::RecoverType> >())
        .def(self_ns::str(self))
        .add_property("start", &KQuery::start)
        .add_property("end", &KQuery::end)
        .add_property("startDatetime", &KQuery::startDatetime)
        .add_property("endDatetime", &KQuery::endDatetime)
        .add_property("queryType", &KQuery::queryType)
        .add_property("kType", &KQuery::kType)
        .add_property("recoverType", &KQuery::recoverType)
        .def("getQueryTypeName", &KQuery::getQueryTypeName)
        .staticmethod("getQueryTypeName")
        .def("getKTypeName", &KQuery::getKTypeName)
        .staticmethod("getKTypeName")
        .def("getRecoverTypeName", &KQuery::getRecoverTypeName)
        .staticmethod("getRecoverTypeName")
        .def("getQueryTypeEnum", &KQuery::getQueryTypeEnum)
        .staticmethod("getQueryTypeEnum")
        .def("getKTypeEnum", &KQuery::getKTypeEnum)
        .staticmethod("getKTypeEnum")
        .def("getRecoverTypeEnum", &KQuery::getRecoverTypeEnum)
        .staticmethod("getRecoverTypeEnum")
#if HKU_PYTHON_SUPPORT_PICKLE
        .def_pickle(normal_pickle_suite<KQuery>())
#endif
      ;

    enum_<KQuery::QueryType>("QueryType").value("INDEX", KQuery::INDEX).value("DATE", KQuery::DATE);

    enum_<KQuery::RecoverType>("RecoverType")
      .value("NO_RECOVER", KQuery::NO_RECOVER)
      .value("FORWARD", KQuery::FORWARD)
      .value("BACKWARD", KQuery::BACKWARD)
      .value("EQUAL_FORWARD", KQuery::EQUAL_FORWARD)
      .value("EQUAL_BACKWARD", KQuery::EQUAL_BACKWARD)
      .value("INVALID_RECOVER_TYPE", KQuery::INVALID_RECOVER_TYPE);
}
