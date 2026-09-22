/*
 * _BlockInfoDriver.cpp
 *
 *  Created on: 2017-10-07
 *      Author: fasiondog
 */

#include <hikyuu/data_driver/BlockInfoDriver.h>
#include "_BlockInfoDriver.h"

using namespace hku;
namespace py = pybind11;

static string BlockInfoDriver_to_str(const BlockInfoDriver& v) {
    std::stringstream out;
    out << v;
    return out.str();
}

BlockList (BlockInfoDriver::*get_block_list_1)(const string&) = &BlockInfoDriver::getBlockList;
BlockList (BlockInfoDriver::*get_block_list_2)() = &BlockInfoDriver::getBlockList;

void export_BlockInfoDriver(py::module& m) {
    py::class_<BlockInfoDriver, BlockInfoDriverPtr, PyBlockInfoDriver>(m, "BlockInfoDriver",
                                                                       R"(The block data driver base class

    The subclass interfaces:
        - _init(self) (Required)
        - getAllCategory(self) (Required)
        - getBlock(self, category, name) (Required)
        - getBlockList(self, category=None) (Required)
        - save(self, block) (Required)
        - remove(self, category, name) (Required)
    )")
      .def(py::init<const string&>(), R"(Initialize

    :param str name: the driver name)")
      .def_property_readonly("name", &BlockInfoDriver::name, py::return_value_policy::copy,
                             "The driver name")
      .def("__str__", BlockInfoDriver_to_str)
      .def("__repr__", BlockInfoDriver_to_str)

      .def("get_param", &BlockInfoDriver::getParam<boost::any>, "Get the specified parameter")
      .def("set_param",
           static_cast<void (BlockInfoDriver::*)(const std::string&, const boost::any&)>(
             &BlockInfoDriver::setParam),
           "Set the specified parameter")
      .def("have_param", &BlockInfoDriver::haveParam, "Whether the specified parameter exists")

      .def("_init", &BlockInfoDriver::_init, "[Subclass interface (Required)] Initialize the driver")
      .def("getAllCategory", &BlockInfoDriver::getAllCategory,
           "[Subclass interface (Required)] Get all the block categories")
      .def("getBlock", &BlockInfoDriver::getBlock, py::arg("category"), py::arg("name"),
           R"([Subclass interface (Required)] Get the specified block

    :param str category: the specified block category
    :param str name: the block name)")
      .def("_getBlockList",
           (BlockList (BlockInfoDriver::*)(const string&))&BlockInfoDriver::getBlockList,
           py::arg("category"),
           R"([Subclass interface (Required)] Get the block list of the specified category

    :param str category: the block category)")
      .def("getBlockList", get_block_list_1, py::arg("category"), "Get the block list of the specified category")
      .def("getBlockList", get_block_list_2, "Get all the block lists")
      .def("save", &BlockInfoDriver::save, py::arg("block"),
           R"([Subclass interface (Required)] Save the specified block

    :param Block block: the block object
    :note: if a block with the same name already exists, it will be overwritten; if the block category or the name has been modified, you need to delete the original block manually before the modification)")
      .def("remove", &BlockInfoDriver::remove, py::arg("category"), py::arg("name"),
           R"([Subclass interface (Required)] Delete the specified block

    :param str category: the block category
    :param str name: the block name)");
}