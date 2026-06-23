/*
 * _BlockInfoDriver.cpp
 *
 *  Created on: 2017年10月7日
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
                                                                       R"(板块数据驱动基类

    子类接口：
        - _init(self) (必须)
        - getAllCategory(self) (必须)
        - getBlock(self, category, name) (必须)
        - getBlockList(self, category=None) (必须)
        - save(self, block) (必须)
        - remove(self, category, name) (必须)
    )")
      .def(py::init<const string&>(), R"(初始化

    :param str name: 驱动名称)")
      .def_property_readonly("name", &BlockInfoDriver::name, py::return_value_policy::copy,
                             "驱动名称")
      .def("__str__", BlockInfoDriver_to_str)
      .def("__repr__", BlockInfoDriver_to_str)

      .def("get_param", &BlockInfoDriver::getParam<boost::any>, "获取指定参数")
      .def("set_param",
           static_cast<void (BlockInfoDriver::*)(const std::string&, const boost::any&)>(
             &BlockInfoDriver::setParam),
           "设置指定参数")
      .def("have_param", &BlockInfoDriver::haveParam, "指定参数是否存在")

      .def("_init", &BlockInfoDriver::_init, "【子类接口（必须）】驱动初始化")
      .def("getAllCategory", &BlockInfoDriver::getAllCategory,
           "【子类接口（必须）】获取所有板块分类")
      .def("getBlock", &BlockInfoDriver::getBlock, py::arg("category"), py::arg("name"),
           R"(【子类接口（必须）】获取指定板块

    :param str category: 指定的板块分类
    :param str name: 板块名称)")
      .def("_getBlockList",
           (BlockList (BlockInfoDriver::*)(const string&))&BlockInfoDriver::getBlockList,
           py::arg("category"),
           R"(【子类接口（必须）】获取指定分类的板块列表

    :param str category: 板块分类)")
      .def("getBlockList", get_block_list_1, py::arg("category"), "获取指定分类的板块列表")
      .def("getBlockList", get_block_list_2, "获取所有板块列表")
      .def("save", &BlockInfoDriver::save, py::arg("block"),
           R"(【子类接口（必须）】保存指定的板块

    :param Block block: 板块对象
    :note: 如果已存在同名板块，则覆盖；如果板块分类或名称存在修改，需要手工在修改前删除原板块)")
      .def("remove", &BlockInfoDriver::remove, py::arg("category"), py::arg("name"),
           R"(【子类接口（必须）】删除指定的板块

    :param str category: 板块分类
    :param str name: 板块名称)");
}