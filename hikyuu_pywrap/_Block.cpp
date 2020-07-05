/*
 * _Block.cpp
 *
 *  Created on: 2015年2月10日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/serialization/Block_serialization.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

string (Block::*getCategory)() const = &Block::category;
void (Block::*setCategory)(const string&) = &Block::category;
string (Block::*getName)() const = &Block::name;
void (Block::*setName)(const string&) = &Block::name;

bool (Block::*add_1)(const Stock&) = &Block::add;
bool (Block::*add_2)(const string&) = &Block::add;
bool (Block::*remove_1)(const Stock&) = &Block::remove;
bool (Block::*remove_2)(const string&) = &Block::remove;

void export_Block() {
    class_<Block>("Block", "板块类，可视为证券的容器", init<>())
      .def(init<const string&, const string&>())
      .def(init<const Block&>())

      .def(self_ns::str(self))
      .def(self_ns::repr(self))

      .add_property("category", getCategory, setCategory, "板块所属分类")
      .add_property("name", getName, setName, "板块名称")

      .def("empty", &Block::empty, R"(empty(self)
    
    是否为空)")

      .def("add", add_1, R"(add(self, stock)

    加入指定的证券

    :param Stock stock: 待加入的证券
    :return: 是否成功加入
    :rtype: bool)")

      .def("add", add_2, R"(add(self, market_code)

    根据"市场简称证券代码"加入指定的证券

    :param str market_code: 市场简称证券代码
    :return: 是否成功加入
    :rtype: bool)")

      .def("remove", remove_1, R"(remove(self, stock)

    移除指定证券

    :param Stock stock: 指定的证券
    :return: 是否成功
    :rtype: bool)")

      .def("remove", remove_2, R"(remove(market_code)

    移除指定证券

    :param str market_code: 市场简称证券代码
    :return: 是否成功
    :rtype: bool)")

      .def("clear", &Block::clear, "移除包含的所有证券")

      .def("__len__", &Block::size)
      .def("__getitem__", &Block::get)
      .def("__iter__", iterator<const Block>())

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<Block>())
#endif
      ;

    BlockList::const_reference (BlockList::*BlockList_at)(BlockList::size_type) const =
      &BlockList::at;
    class_<BlockList>("BlockList", "C++ std::vector<Block>包装")
      .def("__iter__", iterator<BlockList>())
      .def("size", &BlockList::size)
      .def("__len__", &BlockList::size)
      .def("get", BlockList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<BlockList>())
#endif
      ;
}
