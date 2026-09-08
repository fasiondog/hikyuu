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

    py::function get_py_override(const char* name) const {
        return py::get_override(static_cast<const BlockInfoDriver*>(this), name);
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

    // 必须实现的子类接口
    BlockList getBlockList(const string& category) override {
        py::gil_scoped_acquire gil;
        py::function py_func = get_py_override("_getBlockList");
        if (!py_func) {
            pybind11::pybind11_fail(
              "Tried to call pure virtual function \"BlockInfoDriver::_getBlockList\"");
        }

        auto py_list = py_func(category);
        return python_list_to_vector<Block>(py_list);
    }

    // 必须实现的子类接口
    BlockList getBlockList() override {
        py::gil_scoped_acquire gil;
        py::function py_func = get_py_override("_getBlockList");
        if (!py_func) {
            pybind11::pybind11_fail(
              "Tried to call pure virtual function \"BlockInfoDriver::_getBlockList\"");
        }

        auto py_list = py_func(py::none());
        return python_list_to_vector<Block>(py_list);
    }

    void save(const Block& block) override {
        PYBIND11_OVERLOAD_PURE(void, BlockInfoDriver, save, block);
    }

    void remove(const string& category, const string& name) override {
        PYBIND11_OVERLOAD_PURE(void, BlockInfoDriver, remove, category, name);
    }
};