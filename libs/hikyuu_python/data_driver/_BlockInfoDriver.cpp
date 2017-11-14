/*
 * _DataDriverFactory.cpp
 *
 *  Created on: 2017年10月7日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/data_driver/BlockInfoDriver.h>
#include "../_Parameter.h"

using namespace hku;
using namespace boost::python;

class BlockInfoDriverWrap: public BlockInfoDriver, public wrapper<BlockInfoDriver> {
public:
    BlockInfoDriverWrap(const string& name): BlockInfoDriver(name) {}

    bool _init() {
        return this->get_override("_init")();
    }

    Block getBlock(const string& category, const string& name) {
        return this->get_override("getBlock")(category, name);
    }

    BlockList getBlockList(const string& category) {
        return this->get_override("getBlockList")(category);
    }

    BlockList getBlockList() {
        return this->get_override("getBlockList")();
    }
};

BlockList (BlockInfoDriver::*get_block_list_1)(const string&) = &BlockInfoDriver::getBlockList;
BlockList (BlockInfoDriver::*get_block_list_2)() = &BlockInfoDriver::getBlockList;


void export_BlockInfoDriver() {

    class_<BlockInfoDriverWrap, boost::noncopyable>("BlockInfoDriver",
                init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", make_function(&BlockInfoDriver::name,
                    return_value_policy<copy_const_reference>()))
            .def("getParam", &BlockInfoDriver::getParam<boost::any>)

            .def("init", &BlockInfoDriver::init)
            .def("_init", pure_virtual(&BlockInfoDriver::_init))
            .def("getBlock", pure_virtual(&BlockInfoDriver::getBlock))

            .def("getBlockList", pure_virtual(get_block_list_1))
            .def("getBlockList", pure_virtual(get_block_list_2))
            ;

    register_ptr_to_python<BlockInfoDriverPtr>();
}


