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
    docstring_options doc_options(false);

    class_<Block>("Block", init<>())
            .def(init<const string&, const string&>())
            .def(init<const Block&>())
            .def(self_ns::str(self))
            .add_property("category", getCategory, setCategory)
            .add_property("name", getName, setName)

            .def("size", &Block::size)
            .def("empty", &Block::empty)
            .def("get", &Block::get)
            .def("add", add_1)
            .def("add", add_2)
            .def("remove", remove_1)
            .def("remove", remove_2)
            .def("clear", &Block::clear)

            .def("__len__", &Block::size)
            .def("__getitem__", &Block::get)
            .def("__iter__", iterator<const Block>())

#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<Block>())
#endif
            ;

    BlockList::const_reference (BlockList::*BlockList_at)(BlockList::size_type) const = &BlockList::at;
    class_<BlockList>("BlockList")
            .def("__iter__", iterator<BlockList>())
            .def("size", &BlockList::size)
            .def("__len__", &BlockList::size)
            .def("get", BlockList_at, return_value_policy<copy_const_reference>())
#if HKU_PYTHON_SUPPORT_PICKLE
            .def_pickle(normal_pickle_suite<BlockList>())
#endif
            ;
}



