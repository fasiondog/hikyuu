/*
 * _Block.cpp
 *
 *  Created on: 2015年2月10日
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
    py::class_<Block>(m, "Block", "板块类，可视为证券的容器")
      .def(py::init<>())
      .def(py::init<const string&, const string&>())
      .def(py::init<const Block&>())

      .def("__str__", to_py_str<Block>)
      .def("__repr__", to_py_str<Block>)

      .def_property("category", setCategory, getCategory, "板块所属分类")
      .def_property("name", getName, setName, "板块名称")
      .def_property("index_stock", &Block::getIndexStock, &Block::setIndexStock,
                    py::return_value_policy::copy, "对应指数")

      .def("empty", &Block::empty, R"(empty(self)
    
    是否为空)")

      .def("add", py::overload_cast<const Stock&>(&Block::add), R"(add(self, stock)

    加入指定的证券

    :param Stock stock: 待加入的证券
    :return: 是否成功加入
    :rtype: bool)")

      .def("add", py::overload_cast<const string&>(&Block::add), R"(add(self, market_code)

    根据"市场简称证券代码"加入指定的证券

    :param str market_code: 市场简称证券代码
    :return: 是否成功加入
    :rtype: bool)")

      .def(
        "add",
        [](Block& blk, py::sequence stks) {
            auto total = len(stks);
            HKU_IF_RETURN(total == 0, true);

            if (py::isinstance<Stock>(stks[0])) {
                StockList cpp_stks = python_list_to_vector<Stock>(stks);
                return blk.add(cpp_stks);
            }

            if (py::isinstance<string>(stks[0])) {
                StringList codes = python_list_to_vector<string>(stks);
                return blk.add(codes);
            }

            HKU_ERROR("Not support type!");
            return false;
        },
        R"(add(self, sequence)

    加入定的证券列表

    :param sequence stks: 全部由 Stock 组成的序列或全部由字符串市场简称证券代码组成的序列
    :return: True 全部成功 | False 存在失败)")

      .def("remove", py::overload_cast<const Stock&>(&Block::remove), R"(remove(self, stock)

    移除指定证券

    :param Stock stock: 指定的证券
    :return: 是否成功
    :rtype: bool)")

      .def("remove", py::overload_cast<const string&>(&Block::remove), R"(remove(market_code)

    移除指定证券

    :param str market_code: 市场简称证券代码
    :return: True 成功 | False 失败
    :rtype: bool)")

      .def("clear", &Block::clear, "移除包含的所有证券")

      .def("__len__", &Block::size, "包含的证券数量")

      .def("__getitem__", &Block::get, R"(__getitem__(self, market_code)

    :param str market_code: 证券代码
    :return: Stock 实例)")

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
        
    获取证券列表

    :param func filter: 输入参数为 stock, 返回 True | False 的过滤函数)")

        DEF_PICKLE(Block);
}
