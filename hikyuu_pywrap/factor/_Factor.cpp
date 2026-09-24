/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include <hikyuu/factor/Factor.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_Factor(py::module& m) {
    py::class_<Factor>(m, "Factor", "The factor metadata")
      .def(py::init<>(), R"(__init__(self)
    
    The default constructor, creating an empty factor object)")

      .def(py::init<const string&, const KQuery::KType&>(), py::arg("name"),
           py::arg("ktype") = KQuery::DAY,
           R"(__init__(self, name[, ktype=KQuery.DAY])
    
    The constructor specifying only the factor name and the K-line type, which will try to load the factor from the database automatically

    :param str name: the factor name
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line)")

      .def(
        py::init([](const string& name, const Indicator& formula, const KQuery::KType& ktype,
                    const string& brief, const string& details, bool save_value,
                    const Datetime& start_date, const py::object& block,
                    KQuery::RecoverType recover_type) {
            Block c_block = get_block_from_python(block);
            return Factor(name, formula, ktype, brief, details, save_value, start_date, c_block,
                          recover_type);
        }),
        py::arg("name"), py::arg("formula"), py::arg("ktype") = KQuery::DAY, py::arg("brief") = "",
        py::arg("details") = "", py::arg("need_save_value") = false,
        py::arg("start_date") = Datetime::min(), py::arg("block") = Block(),
        py::arg("recover_type") = KQuery::NO_RECOVER,
        R"(__init__(self, name, formula[, ktype=KQuery.DAY[, brief=""[, details=""[, need_save_value=False[, start_date=Datetime.min()[, block=Block()]]]]]])
    
    The constructor creating a new factor object (the factor name + the K-line type is the unique identifier of the factor)

    :param str name: the factor name
    :param Indicator formula: the calculation formula indicator, which cannot be changed once created
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :param str brief: the brief description, defaulting to empty
    :param str details: the detailed description, defaulting to empty
    :param bool need_save_value: whether the factor value data needs to be persisted, defaulting to False
    :param Datetime start_date: the start date, the starting date when storing the data, defaulting to the minimum date
    :param Block block: the block information, the security set; if it is empty, it is all, defaulting to empty
    :param KQuery.RecoverType recover_type: the recovery type, defaulting to NO_RECOVER
    :note: the factor name is not case-sensitive, with name + ktype as the unique identifier)")

      .def("__str__", &Factor::str)
      .def("__repr__", &Factor::str)

      .def_property("name", py::overload_cast<>(&Factor::name, py::const_),
                    py::overload_cast<const string&>(&Factor::name), py::return_value_policy::copy,
                    "The factor name")
      .def_property("ktype", py::overload_cast<>(&Factor::ktype, py::const_),
                    py::overload_cast<const string&>(&Factor::ktype), py::return_value_policy::copy,
                    "The factor frequency type")
      .def_property("create_at", py::overload_cast<>(&Factor::createAt, py::const_),
                    py::overload_cast<const Datetime&>(&Factor::createAt),
                    py::return_value_policy::copy, "The creation date")
      .def_property("update_at", py::overload_cast<>(&Factor::updateAt, py::const_),
                    py::overload_cast<const Datetime&>(&Factor::updateAt),
                    py::return_value_policy::copy, "The modification date")
      .def_property("formula", py::overload_cast<>(&Factor::formula, py::const_),
                    py::overload_cast<const Indicator&>(&Factor::formula),
                    py::return_value_policy::copy, "The factor formula")
      .def_property("start_date", py::overload_cast<>(&Factor::startDate, py::const_),
                    py::overload_cast<const Datetime&>(&Factor::startDate), "The start date of the data storage")
      .def_property("block", py::overload_cast<>(&Factor::block, py::const_),
                    py::overload_cast<const Block&>(&Factor::block), py::return_value_policy::copy,
                    "The security set")
      .def_property("brief", py::overload_cast<>(&Factor::brief, py::const_),
                    py::overload_cast<const string&>(&Factor::brief), py::return_value_policy::copy,
                    "The basic description")
      .def_property("details", py::overload_cast<>(&Factor::details, py::const_),
                    py::overload_cast<const string&>(&Factor::details),
                    py::return_value_policy::copy, "The detailed description")
      .def_property("need_save_value", py::overload_cast<>(&Factor::needSaveValue, py::const_),
                    py::overload_cast<bool>(&Factor::needSaveValue), "Whether to persist the factor value data")
      .def_property("recover_type", py::overload_cast<>(&Factor::recoverType, py::const_),
                    py::overload_cast<KQuery::RecoverType>(&Factor::recoverType),
                    py::return_value_policy::copy, "The recovery type")

      .def("is_null", &Factor::isNull, "Whether it is an empty factor")

      .def("save_to_db", &Factor::save_to_db, py::arg("update_before") = true,
           R"(save_to_db(self[, update_before=True])
    
    Save the factor metadata to the database; if the factor already exists, update it, otherwise insert a new record
    
    :note: the factor name is not case-sensitive, with name + ktype as the unique identifier
    
    :param bool update_before: whether to check and update the existing factor before saving, defaulting to True). Note: it usually must be true, otherwise it will cause the data errors, unless you are certain that all the factor values have been updated)")

      .def("save_special_values_to_db",
           py::overload_cast<const Stock&, const Indicator&, bool>(&Factor::save_special_values_to_db),
           py::arg("stock"), py::arg("values"), py::arg("replace") = false)

      .def("save_special_values_to_db",
           py::overload_cast<const Stock&, const DatetimeList&, const PriceList&, bool>(
             &Factor::save_special_values_to_db),
           py::arg("stock"), py::arg("dates"), py::arg("values"), py::arg("replace") = false,
           R"(save_special_values_to_db(self, stock, values[, replace=False])
save_special_values_to_db(self, stock, dates, values[, replace=False])

    Save the special factor values to the database, supporting two input formats:
    1. Save the result data of an Indicator object directly (usually a PRICELIST), extracting the dates and the values from the Indicator automatically
    2. Save the pre-calculated date-value pair data, applicable when there are already independent date and price lists
       (such as the externally imported finance data or the machine learning prediction results), without needing to be wrapped into an Indicator first

    Overload 1 - save the Indicator object:
    :param Stock stock: the security object
    :param Indicator values: the already calculated indicator object (it must have been bound to the K-line data)
    :param bool replace: whether to replace the existing data, defaulting to False

    Overload 2 - save the pre-calculated data:
    :param Stock stock: the security object
    :param DatetimeList dates: the special factor date list
    :param PriceList values: the special factor value list
    :param bool replace: whether to replace the existing data, defaulting to False

    Usage scenarios:
    - Save the composite indicator calculation results
    - Save the externally imported finance data
    - Save the machine learning model prediction results
    - Save the manually annotated special factor values)")

      .def("remove_from_db", &Factor::remove_from_db,
           R"(remove_from_db(self)
    
    Delete the factor and its data from the database. Note: to prevent the misoperations, the values of the special factors will not be deleted; you need to delete them manually yourself.
    
    :note: delete with name + ktype as the unique identifier)")

      .def(
        "get_all_values", &Factor::getAllValues, py::arg("query"), py::arg("align") = false,
        py::arg("fill_null") = false, py::arg("tovalue") = false,
        py::arg("align_dates") = DatetimeList{},
        R"(get_all_values(self, query[, align=False[, fill_null=False[, tovalue=False[, align_dates=DatetimeList()]]]])
    
    Get all the calculation results of the specified query parameters

    :param Query query: the query parameters
    :param bool align: whether to align the dates (e.g. by the specified align_dates or the default trading calendar), defaulting to False
    :param bool fill_null: whether to fill the empty values, defaulting to False
    :param bool tovalue: whether to convert to the values, defaulting to False
    :param DatetimeList align_dates: the aligned date list, defaulting to empty
    :return: the list of the calculation results of all the stocks
    :rtype: list)")

      .def("get_value",
           py::overload_cast<const KData&, bool, bool, bool, bool, const DatetimeList&>(
             &Factor::getValue, py::const_),
           py::arg("kdata"), py::arg("align") = false, py::arg("fill_null") = false,
           py::arg("tovalue") = false, py::arg("check") = false,
           py::arg("align_dates") = DatetimeList{})
      .def(
        "get_value",
        py::overload_cast<const Stock&, const KQuery&, bool, bool, bool, bool, const DatetimeList&>(
          &Factor::getValue, py::const_),
        py::arg("stock"), py::arg("query"), py::arg("align") = false, py::arg("fill_null") = false,
        py::arg("tovalue") = false, py::arg("check") = false,
        py::arg("align_dates") = DatetimeList{},
        R"(get_value(self, stock, query[, align=False[, fill_null=False[, tovalue=False[, check=False[, align_dates=DatetimeList()]]]]])
    
    Get the calculation result of the specified stock with the specified query parameters

    :param Stock stock: the security object
    :param Query query: the query parameters
    :param bool align: whether to align the dates (e.g. by the specified align_dates or the default trading calendar), defaulting to False
    :param bool fill_null: whether to fill the empty values, defaulting to False
    :param bool tovalue: whether to convert to the values, defaulting to False
    :param bool check: whether to check that the stock belongs to the block specified by itself, defaulting to False
    :param DatetimeList align_dates: the aligned date list, defaulting to empty
    :return: the calculation result indicator
    :rtype: Indicator)")

      .def(
        "get_values",
        [](Factor& self, const py::object& stks, const KQuery& query, bool align, bool fill_null,
           bool tovalue, bool check, const DatetimeList& align_dates) {
            return self.getValues(get_stock_list_from_python(stks), query, align, fill_null,
                                  tovalue, check, align_dates);
        },
        py::arg("stocks"), py::arg("query"), py::arg("align") = false, py::arg("fill_null") = false,
        py::arg("tovalue") = false, py::arg("check") = false,
        py::arg("align_dates") = DatetimeList{},
        R"(get_values(self, stocks, query[, align=False[, fill_null=False[, tovalue=False[, check=False[, align_dates=DatetimeList()]]]]])
    
    Get the calculation results of the specified stock list with the specified query parameters

    :param list stocks: the security list
    :param Query query: the query parameters
    :param bool align: whether to align the dates (e.g. by the specified align_dates or the default trading calendar), defaulting to False
    :param bool fill_null: whether to fill the empty values, defaulting to False
    :param bool tovalue: whether to convert to the values, defaulting to False
    :param bool check: whether to check that the stock list belongs to the block specified by itself, defaulting to False
    :param DatetimeList align_dates: the aligned date list, defaulting to empty
    :return: the list of the calculation results arranged by the stock order
    :rtype: list)")

      .def(py::hash(py::self))

        DEF_PICKLE(Factor);
}
