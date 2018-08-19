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

BOOST_PYTHON_FUNCTION_OVERLOADS(KQueryByDate_overload, KQueryByDate, 0, 4);
BOOST_PYTHON_FUNCTION_OVERLOADS(KQueryByIndex_overload, KQueryByIndex, 0, 4);

void export_KQuery() {
    docstring_options doc_options(false);

    def("KQueryByDate", KQueryByDate, KQueryByDate_overload());
    def("KQueryByIndex", KQueryByIndex, KQueryByIndex_overload());

    scope in_Query = class_<KQuery>("KQuery", init<>())
            .def(init<hku_int64, optional<hku_int64, KQuery::KType, KQuery::RecoverType> >())
            .def(self_ns::str(self))
            .add_property("start", &KQuery::start)
            .add_property("end", &KQuery::end)
            .add_property("startDatetime", &KQuery::startDatetime)
            .add_property("endDatetime", &KQuery::endDatetime)
            .add_property("queryType", &KQuery::queryType)
            .add_property("kType", &KQuery::kType)
            .add_property("recoverType", &KQuery::recoverType)
            .def("getQueryTypeName", &KQuery::getQueryTypeName).staticmethod("getQueryTypeName")
            .def("getKTypeName", &KQuery::getKTypeName).staticmethod("getKTypeName")
            .def("getRecoverTypeName", &KQuery::getRecoverTypeName).staticmethod("getRecoverTypeName")
            .def("getQueryTypeEnum", &KQuery::getQueryTypeEnum).staticmethod("getQueryTypeEnum")
            .def("getKTypeEnum", &KQuery::getKTypeEnum).staticmethod("getKTypeEnum")
            .def("getRecoverTypeEnum", &KQuery::getRecoverTypeEnum).staticmethod("getRecoverTypeEnum")
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<KQuery>())
#endif
            ;

    enum_<KQuery::QueryType>("QueryType")
            .value("INDEX", KQuery::INDEX)
            .value("DATE", KQuery::DATE)
            ;

    enum_<KQuery::KType>("KType")
            .value("MIN", KQuery::MIN)
            .value("MIN5", KQuery::MIN5)
            .value("MIN15", KQuery::MIN15)
            .value("MIN30", KQuery::MIN30)
            .value("MIN60", KQuery::MIN60)
            .value("DAY", KQuery::DAY)
            .value("WEEK", KQuery::WEEK)
            .value("MONTH", KQuery::MONTH)
            .value("QUARTER", KQuery::QUARTER)
            .value("HALFYEAR", KQuery::HALFYEAR)
            .value("YEAR", KQuery::YEAR)

            //BTCÀ©Õ¹
            .value("MIN3", KQuery::MIN3)
            .value("HOUR2", KQuery::HOUR2)
            .value("HOUR4", KQuery::HOUR4)
            .value("HOUR6", KQuery::HOUR6)
            .value("HOUR12", KQuery::HOUR12)

            .value("INVALID_KTYPE", KQuery::INVALID_KTYPE)
            ;

    enum_<KQuery::RecoverType>("RecoverType")
            .value("NO_RECOVER", KQuery::NO_RECOVER)
            .value("FORWARD", KQuery::FORWARD)
            .value("BACKWARD", KQuery::BACKWARD)
            .value("EQUAL_FORWARD", KQuery::EQUAL_FORWARD)
            .value("EQUAL_BACKWARD", KQuery::EQUAL_BACKWARD)
            .value("INVALID_RECOVER_TYPE", KQuery::INVALID_RECOVER_TYPE)
            ;

}


