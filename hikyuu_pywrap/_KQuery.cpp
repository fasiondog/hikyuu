/*
 * _KQuery.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <hikyuu/serialization/Datetime_serialization.h>
#include <hikyuu/serialization/KQuery_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_KQuery(py::module& m) {
    int64_t null_int = Null<int64_t>();

    py::class_<KQuery> kquery(m, "Query", "K线数据查询条件");
    kquery.def(py::init<>())
      .def("__str__", to_py_str<KQuery>)
      .def("__repr__", to_py_str<KQuery>)
      .def_property_readonly("start", &KQuery::start, "起始索引，当按日期查询方式创建时无效")
      .def_property_readonly("end", &KQuery::end, "结束索引，当按日期查询方式创建时无效")
      .def_property_readonly("start_datetime", &KQuery::startDatetime,
                             "起始日期，当按索引查询方式创建时无效")
      .def_property_readonly("end_datetime", &KQuery::endDatetime,
                             "结束日期，当按索引查询方式创建时无效")
      .def_property_readonly("query_type", &KQuery::queryType, "查询方式")
      .def_property_readonly("ktype", &KQuery::kType, "查询的K线类型")
      .def_property_readonly("recover_type", py::overload_cast<>(&KQuery::recoverType, py::const_),
                             "复权类别")
      .def("get_all_ktype", &KQuery::getAllKType, "获取所有KType")
      .def("get_ktype_in_min", &KQuery::getKTypeInMin, "获取ktype对应的分钟数")

        DEF_PICKLE(KQuery);

    py::enum_<KQuery::RecoverType>(kquery, "RecoverType")
      .value("NO_RECOVER", KQuery::RecoverType::NO_RECOVER, "不复权")
      .value("FORWARD", KQuery::RecoverType::FORWARD, "前向复权")
      .value("BACKWARD", KQuery::RecoverType::BACKWARD, "后向复权")
      .value("EQUAL_FORWARD", KQuery::RecoverType::EQUAL_FORWARD, "等比前向复权")
      .value("EQUAL_BACKWARD", KQuery::RecoverType::EQUAL_BACKWARD, "等比后向复权")
      .value("INVALID", KQuery::RecoverType::INVALID_RECOVER_TYPE, "无效类型")
      .export_values();

    py::enum_<KQuery::QueryType>(kquery, "QueryType")
      .value("INDEX", KQuery::QueryType::INDEX, "按索引方式查询")
      .value("DATE", KQuery::QueryType::DATE, "按日期方式查询")
      .value("INVALID", KQuery::QueryType::INVALID, "无效类型")
      .export_values();

    // 使用了内部枚举类型，需要枚举类型先注册，否则加载报错
    kquery.def(py::init<int64_t, int64_t, KQuery::KType, KQuery::RecoverType>(), py::arg("start"),
               py::arg("end") = null_int, py::arg("ktype") = KQuery::DAY,
               py::arg("recover_type") = KQuery::NO_RECOVER,
               "\t构建按索引 [start, end) 方式获取K线数据条件");

    Datetime null_date;
    kquery.def(py::init<const Datetime&, const Datetime&, KQuery::KType, KQuery::RecoverType>(),
               py::arg("start"), py::arg("end") = null_date, py::arg("ktype") = KQuery::DAY,
               py::arg("recover_type") = KQuery::NO_RECOVER,
               "\t构建按日期 [start, end) 方式获取K线数据条件");

    kquery.attr("DAY") = "DAY";
    kquery.attr("WEEK") = "WEEK";
    kquery.attr("MONTH") = "MONTH";
    kquery.attr("QUARTER") = "QUARTER";
    kquery.attr("HALFYEAR") = "HALFYEAR";
    kquery.attr("YEAR") = "YEAR";
    kquery.attr("MIN") = "MIN";
    kquery.attr("MIN5") = "MIN5";
    kquery.attr("MIN15") = "MIN15";
    kquery.attr("MIN30") = "MIN30";
    kquery.attr("MIN60") = "MIN60";
    kquery.attr("MIN3") = "MIN3";
    kquery.attr("HOUR2") = "HOUR2";
    kquery.attr("HOUR4") = "HOUR4";
    kquery.attr("HOUR6") = "HOUR6";
    kquery.attr("HOUR12") = "HOUR12";
}
