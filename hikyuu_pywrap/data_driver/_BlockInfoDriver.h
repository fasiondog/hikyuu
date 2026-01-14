/*
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-6-18
 *      Author: fasiondog
 */

#pragma once

#include <pybind11/pybind11.h>
#include <hikyuu/data_driver/BlockInfoDriver.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

class PyBlockInfoDriver : public BlockInfoDriver {
public:
    PyBlockInfoDriver() : BlockInfoDriver("PyBlockInfoDriver") {
        m_is_python_object = true;
    }

    PyBlockInfoDriver(const string& name) : BlockInfoDriver(name) {
        m_is_python_object = true;
    }

    bool _init() override {
        PYBIND11_OVERLOAD_PURE(bool, BlockInfoDriver, _init, );
    }

    StringList getAllCategory() override {
        PYBIND11_OVERLOAD_PURE(StringList, BlockInfoDriver, getAllCategory, );
    }

    Block getBlock(const string& category, const string& name) override {
        PYBIND11_OVERLOAD_PURE(Block, BlockInfoDriver, getBlock, category, name);
    }

    BlockList getBlockList(const string& category) override {
        auto self = py::cast(this);
        auto py_list = self.attr("_getBlockList")(category);
        return python_list_to_vector<Block>(py_list);
    }

    BlockList getBlockList() override {
        auto self = py::cast(this);
        auto py_list = self.attr("_getBlockList")(py::none());
        return python_list_to_vector<Block>(py_list);
    }

    void save(const Block& block) override {
        PYBIND11_OVERLOAD_PURE(void, BlockInfoDriver, save, block);
    }

    void remove(const string& category, const string& name) override {
        PYBIND11_OVERLOAD_PURE(void, BlockInfoDriver, remove, category, name);
    }
};