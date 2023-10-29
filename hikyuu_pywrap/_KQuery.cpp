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

KQuery::RecoverType (KQuery::*get_recoverType)() const = &KQuery::recoverType;
void (KQuery::*set_recoverType)(KQuery::RecoverType recoverType) = &KQuery::recoverType;

bool (*kquery_eq)(const KQuery&, const KQuery&) = operator==;
bool (*kquery_ne)(const KQuery&, const KQuery&) = operator!=;

void export_KQuery() {
    scope in_Query =
      class_<KQuery>("Query", "K线数据查询条件", init<>())
        .def(init<int64_t, optional<int64_t, KQuery::KType, KQuery::RecoverType> >())
        .def(init<Datetime, optional<Datetime, KQuery::KType, KQuery::RecoverType> >())

        .def(self_ns::str(self))
        .def(self_ns::repr(self))

        .add_property("start", &KQuery::start,
                      "起始索引，当按日期查询方式创建时无效，为 constant.null_int64_t")
        .add_property("end", &KQuery::end,
                      "结束索引，当按日期查询方式创建时无效，为 constant.null_int64_t")
        .add_property("start_datetime", &KQuery::startDatetime,
                      "起始日期，当按索引查询方式创建时无效，为 constant.null_datetime")
        .add_property("end_datetime", &KQuery::endDatetime,
                      "结束日期，当按索引查询方式创建时无效，为 constant.null_datetime")
        .add_property("query_type", &KQuery::queryType, "查询方式 Query.QueryType")
        .add_property("ktype", &KQuery::kType, "查询的K线类型 Query.KType")
        .add_property("recover_type", get_recoverType, set_recoverType,
                      "查询的复权类型 Query.RecoverType")

        .def("__eq__", kquery_eq)
        .def("__ne__", kquery_ne)

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
