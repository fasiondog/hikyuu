/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-24
 *      Author: fasiondog
 */

#include <hikyuu/factor/FactorSet.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_FactorSet(py::module& m) {
    py::class_<FactorSet>(m, "FactorSet", "The factor metadata")
      .def(py::init<>(), R"(__init__(self)
    
    The default constructor, creating an empty factor set)")

      .def(py::init<const string&, const KQuery::KType&, const Block&>(), py::arg("name"),
           py::arg("ktype") = KQuery::DAY, py::arg("block") = Block(),
           R"(__init__(self, name[, ktype=KQuery.DAY[, block=Block()]])
    
    The constructor creating the factor set with the specified name and type

    :param str name: the factor set name
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :param Block block: the block information, the security set, defaulting to empty)")

      .def(py::init<const IndicatorList&, const KQuery::KType&>(), py::arg("inds"),
           py::arg("ktype") = KQuery::DAY,
           R"(__init__(self, inds[, ktype=KQuery.DAY])
    
    The constructor creating the factor set with the specified indicator list, where the factor names default to the indicator names

    :note: the indicators with the same name will be overwritten, and finally the last indicator with the same name is kept
    :param list inds: the indicator list
    :param KQuery.KType ktype: the K-line type of the factor set, defaulting to the daily line)")

      .def(py::init([](const py::sequence& factors, const KQuery::KType& ktype, const Block& block,
                       const string& name) {
               FactorList factors_list = python_list_to_vector<Factor>(factors);
               return FactorSet(factors_list, ktype, block, name);
           }),
           py::arg("factors"), py::arg("ktype") = KQuery::DAY, py::arg("block") = Block(),
           py::arg("name") = "",
           R"(__init__(self, factors[, ktype=KQuery.DAY[, block=Block(), name='']])
    
    The constructor creating the factor set with the specified factor list

    :note: the factors with the same name will be overwritten, and finally the last factor with the same name is kept
    :param list factors: the factor list
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :param Block block: the block, defaulting to empty
    :param str name: the factor set name, defaulting to empty)")

      .def("__str__", &FactorSet::str)
      .def("__repr__", &FactorSet::str)

      .def_property("name", py::overload_cast<>(&FactorSet::name, py::const_),
                    py::overload_cast<const string&>(&FactorSet::name),
                    py::return_value_policy::copy, "The factor name")
      .def_property("ktype", py::overload_cast<>(&FactorSet::ktype, py::const_),
                    py::overload_cast<const string&>(&FactorSet::ktype),
                    py::return_value_policy::copy, "The factor frequency type")

      .def_property("block", py::overload_cast<>(&FactorSet::block, py::const_),
                    py::overload_cast<const Block&>(&FactorSet::block),
                    py::return_value_policy::copy, "The block corresponding to the factor set")

      .def("is_null", &FactorSet::isNull, R"(is_null(self)
        
    Whether it is a null value)")

      .def("empty", &FactorSet::empty, R"(empty(self)
    
    Whether it is empty)")

      .def("clear", &FactorSet::clear, R"(clear(self))

    Clear the factor metadata)")

      .def("have", &FactorSet::have)
      .def("remove", &FactorSet::remove)
      .def("add", py::overload_cast<const Factor&>(&FactorSet::add))
      .def("add", py::overload_cast<const string&, const Indicator&>(&FactorSet::add))
      .def("add", py::overload_cast<const Indicator&>(&FactorSet::add))
      .def("add", py::overload_cast<const IndicatorList&>(&FactorSet::add))
      .def("add", py::overload_cast<const FactorList&>(&FactorSet::add))
      .def("add", py::overload_cast<const std::map<string, Indicator>&>(&FactorSet::add))

      .def("get_factors", &FactorSet::getAllFactors, py::return_value_policy::copy, "Get the factor list")

      .def(
        "get_all_values", &FactorSet::getAllValues, py::arg("query"), py::arg("align") = false,
        py::arg("fill_null") = false, py::arg("tovalue") = true,
        py::arg("align_dates") = DatetimeList{},
        R"(get_all_values(self, query[, align=False[, fill_null=False[, tovalue=True[, align_dates=DatetimeList()]]]])
    
    Get the calculation results of all the factors with the specified query parameters

    :param Query query: the query parameters
    :param bool align: whether to align the dates (e.g. by the specified align_dates or the default trading calendar), defaulting to False
    :param bool fill_null: whether to fill the empty values, defaulting to False
    :param bool tovalue: whether to convert to the values, defaulting to True
    :param DatetimeList align_dates: the aligned date list, defaulting to empty
    :return: the list of the calculation results of all the factors
    :rtype: list)")

      .def(
        "get_values",
        [](FactorSet& self, const py::object& stks, const KQuery& query, bool align = false,
           bool fill_null = false, bool tovalue = true, bool check = false,
           const DatetimeList& align_dates = DatetimeList{}) {
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

      .def("save_to_db", &FactorSet::save_to_db,
           R"(save_to_db(self)
    
    Save the factor set to the database
    
    :note: with name + ktype as the unique identifier)")

      .def("remove_from_db", &FactorSet::remove_from_db,
           R"(remove_from_db(self)
    
    Delete the factor set from the database
    
    :note: with name + ktype as the unique identifier)")

      .def("load_from_db", &FactorSet::load_from_db,
           R"(load_from_db(self)
    
    Load the factor set from the database
    
    :note: with name + ktype as the unique identifier; if it does not exist, the current object is not modified)")

      .def("__getitem__", py::overload_cast<const string&>(&FactorSet::get, py::const_),
           py::return_value_policy::copy)
      .def("__getitem__", py::overload_cast<size_t>(&FactorSet::get, py::const_),
           py::return_value_policy::copy)
      .def("__len__", &FactorSet::size, "The number of the contained factors")
      .def(
        "__iter__",
        [](const FactorSet& self) { return py::make_iterator(self.begin(), self.end()); },
        py::keep_alive<0, 1>())

        DEF_PICKLE(FactorSet);
}