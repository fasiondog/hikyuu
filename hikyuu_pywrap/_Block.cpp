/*
 * _Block.cpp
 *
 *  Created on: 2015-02-10
 *      Author: fasiondog
 */

#include <hikyuu/serialization/Block_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

string (Block::*getCategory)() const = &Block::category;
void (Block::*setCategory)(const string&) = &Block::category;
string (Block::*getName)() const = &Block::name;
void (Block::*setName)(const string&) = &Block::name;

void export_Block(py::module& m) {
    py::class_<Block>(m, "Block", "The block class, which can be regarded as a container of the securities")
      .def(py::init<>())
      .def(py::init<const string&, const string&>(), py::arg("category"), py::arg("name"))
      .def(py::init<const string&, const string&, const string&>(), py::arg("category"),
           py::arg("name"), py::arg("index_code"))
      .def(py::init<const Block&>())
      .def(py::init<const StockList&>())
      .def(py::init<const StringList&>())

      .def("__str__", to_py_str<Block>)
      .def("__repr__", to_py_str<Block>)

      .def_property("category", getCategory, setCategory, "The block category")
      .def_property("name", getName, setName, "The block name")
      .def_property("index_stock", &Block::getIndexStock, &Block::setIndexStock,
                    py::return_value_policy::copy, "The corresponding index")

      .def("is_null", &Block::isNull, R"(is_null(self)
        
    Whether it is a null value)")

      .def("empty", &Block::empty, R"(empty(self)
    
    Whether it is empty)")

      .def("add", py::overload_cast<const Stock&>(&Block::add), R"(add(self, stock)

    Add the specified security

    :param Stock stock: the security to add
    :return: whether it was added successfully
    :rtype: bool)")

      .def("add", py::overload_cast<const string&>(&Block::add), R"(add(self, market_code)

    Add the specified security by "market abbreviation + security code"

    :param str market_code: the market abbreviation + the security code
    :return: whether it was added successfully
    :rtype: bool)")

      .def(
        "add",
        [](Block& blk, py::sequence stks) {
            auto total = len(stks);
            HKU_IF_RETURN(total == 0, true);
            StockList stks_list = get_stock_list_from_python(stks);
            return blk.add(stks_list);
        },
        R"(add(self, sequence)

    Add the specified security list

    :param sequence stks: a sequence composed entirely of the Stocks, or a sequence composed entirely of the string "market abbreviation + security code"
    :return: True all succeed | False some fail)")

      .def("remove", py::overload_cast<const Stock&>(&Block::remove), R"(remove(self, stock)

    Remove the specified security

    :param Stock stock: the specified security
    :return: whether it was successful
    :rtype: bool)")

      .def("remove", py::overload_cast<const string&>(&Block::remove), R"(remove(market_code)

    Remove the specified security

    :param str market_code: the market abbreviation + the security code
    :return: True success | False failure
    :rtype: bool)")

      .def("clear", &Block::clear, "Remove all the contained securities")

      .def("__len__", &Block::size, "The number of the contained securities")

      .def("__getitem__", &Block::get, R"(__getitem__(self, market_code)

    :param str market_code: the security code
    :return: the Stock instance)")

      .def(
        "__iter__",
        [](const Block& blk) {
            return py::make_iterator<py::return_value_policy::reference_internal, StockMapIterator,
                                     StockMapIterator, const Stock&>(blk.begin(), blk.end());
        },
        py::keep_alive<0, 1>())

      .def(
        "get_stock_list",
        [](const Block& self, py::object filter) {
            StockList ret;
            if (filter.is_none()) {
                ret = self.getStockList();
            } else {
                HKU_CHECK(py::hasattr(filter, "__call__"), "filter not callable!");
                py::object filter_func = filter.attr("__call__");
                ret = self.getStockList(
                  [&](const Stock& stk) { return filter_func(stk).cast<bool>(); });
            }
            return ret;
        },
        py::arg("filter") = py::none(), R"(get_stock_list(self[, filter=None])
        
    Get the security list

    :param func filter: a filter function whose input parameter is the stock and which returns True | False)")

      .def(py::hash(py::self))
      .def(py::self == py::self)
      .def(py::self != py::self)

        DEF_PICKLE(Block);
}
