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

    py::class_<KQuery> kquery(m, "Query", "The K-line data query condition");
    kquery.def(py::init<>())
      .def("__str__", to_py_str<KQuery>)
      .def("__repr__", to_py_str<KQuery>)
      .def_property_readonly("start", &KQuery::start, "The start index; it is invalid when created with the date query way")
      .def_property_readonly("end", &KQuery::end, "The end index; it is invalid when created with the date query way")
      .def_property_readonly("start_datetime", &KQuery::startDatetime,
                             "The start date; it is invalid when created with the index query way")
      .def_property_readonly("end_datetime", &KQuery::endDatetime,
                             "The end date; it is invalid when created with the index query way")
      .def_property_readonly("query_type", &KQuery::queryType, "The query way")
      .def_property_readonly("ktype", &KQuery::kType, py::return_value_policy::copy,
                             "The K-line type queried")
      .def_property_readonly("recover_type", py::overload_cast<>(&KQuery::recoverType, py::const_),
                             "The recovery type")
      .def_property_readonly("ktype_in_sec", &KQuery::kTypeInSeconds, "Get the number of the seconds corresponding to the ktype")
      .def("is_right_opening", &KQuery::isRightOpening, "Judge whether it is a right-open interval, i.e. the end time is not specified")
      .def_static("is_valid_ktype", &KQuery::isValidKType, "Judge whether the KType is valid")
      .def_static("is_base_ktype", &KQuery::isBaseKType, "Judge whether it is a basic KType")
      .def_static("is_extra_ktype", &KQuery::isExtraKType, "Judge whether it is an extended KType")
      .def_static("get_base_ktype_list", &KQuery::getBaseKTypeList, "Get all the basic KTypes")
      .def_static("get_extra_ktype_list", &KQuery::getExtraKTypeList, "Get all the extended KTypes")
      .def_static("get_ktype_in_min", &KQuery::getKTypeInMin, "Get the number of the minutes corresponding to the ktype")
      .def_static("get_ktype_in_seconds", &KQuery::getKTypeInSeconds, "Get the number of the seconds corresponding to the ktype")

        DEF_PICKLE(KQuery);

    py::enum_<KQuery::RecoverType>(kquery, "RecoverType")
      .value("NO_RECOVER", KQuery::RecoverType::NO_RECOVER, "No recovery")
      .value("FORWARD", KQuery::RecoverType::FORWARD, "The forward recovery")
      .value("BACKWARD", KQuery::RecoverType::BACKWARD, "The backward recovery")
      .value("EQUAL_FORWARD", KQuery::RecoverType::EQUAL_FORWARD, "The equal-ratio forward recovery")
      .value("EQUAL_BACKWARD", KQuery::RecoverType::EQUAL_BACKWARD, "The equal-ratio backward recovery")
      .value("INVALID", KQuery::RecoverType::INVALID_RECOVER_TYPE, "An invalid type")
      .export_values();

    py::enum_<KQuery::QueryType>(kquery, "QueryType")
      .value("INDEX", KQuery::QueryType::INDEX, "Query by the index way")
      .value("DATE", KQuery::QueryType::DATE, "Query by the date way")
      .value("INVALID", KQuery::QueryType::INVALID, "An invalid type")
      .export_values();

    // An internal enumeration type is used; the enumeration type needs to be registered first, otherwise an error occurs when loading
    kquery.def(py::init<int64_t, int64_t, KQuery::KType, KQuery::RecoverType>(), py::arg("start"),
               py::arg("end") = null_int, py::arg("ktype") = KQuery::DAY,
               py::arg("recover_type") = KQuery::NO_RECOVER,
               "\tBuild the condition for getting the K-line data by the index [start, end) way");

    Datetime null_date;
    kquery.def(py::init<const Datetime&, const Datetime&, KQuery::KType, KQuery::RecoverType>(),
               py::arg("start"), py::arg("end") = null_date, py::arg("ktype") = KQuery::DAY,
               py::arg("recover_type") = KQuery::NO_RECOVER,
               "\tBuild the condition for getting the K-line data by the date [start, end) way");

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
    kquery.attr("HOUR2") = "HOUR2";

    kquery.attr("DAY3") = "DAY3";
    kquery.attr("DAY5") = "DAY5";
    kquery.attr("DAY7") = "DAY7";
    kquery.attr("MIN3") = "MIN3";
    kquery.attr("HOUR4") = "HOUR4";
    kquery.attr("HOUR6") = "HOUR6";
    kquery.attr("HOUR12") = "HOUR12";
    kquery.attr("TIMELINE") = "TIMELINE";
    kquery.attr("TRANS") = "TRANS";
}
