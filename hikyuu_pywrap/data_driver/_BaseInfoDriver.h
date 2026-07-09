/*
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-6-18
 *      Author: fasiondog
 */

#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <hikyuu/data_driver/BaseInfoDriver.h>
#include <hikyuu/StockWeight.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

class PyBaseInfoDriver : public BaseInfoDriver {
public:
    PyBaseInfoDriver() : BaseInfoDriver("PyBaseInfoDriver") {}

    PyBaseInfoDriver(const string& name) : BaseInfoDriver(name) {}

    bool _init() override {
        PYBIND11_OVERLOAD_PURE(bool, BaseInfoDriver, _init, );
    }

    vector<StockInfo> getAllStockInfo() override {
        PYBIND11_OVERLOAD_PURE(vector<StockInfo>, BaseInfoDriver, getAllStockInfo, );
    }

    StockInfo getStockInfo(string market, const string& code) override {
        PYBIND11_OVERLOAD_PURE(StockInfo, BaseInfoDriver, getStockInfo, market, code);
    }

    StockWeightList getStockWeightList(const string& market, const string& code, Datetime start,
                                       Datetime end) override {
        PYBIND11_OVERLOAD(StockWeightList, BaseInfoDriver, getStockWeightList, market, code, start,
                          end);
    }

    unordered_map<string, StockWeightList> getAllStockWeightList() override {
        auto self = py::cast(this);
        py::dict py_dict = self.attr("getAllStockWeightList")();
        std::unordered_map<std::string, StockWeightList> result;
        for (auto item : py_dict) {
            std::string key = py::cast<std::string>(item.first);
            py::list py_list = item.second.cast<py::list>();
            StockWeightList value = python_list_to_vector<StockWeight>(py_list);
            result[key] = value;
        }
        return result;
    }

    vector<HistoryFinanceInfo> getHistoryFinance(const string& market, const string& code,
                                                 Datetime start, Datetime end) override {
        PYBIND11_OVERLOAD(vector<HistoryFinanceInfo>, BaseInfoDriver, getHistoryFinance, market,
                          code, start, end);
    }

    vector<std::pair<size_t, string>> getHistoryFinanceField() override {
        auto self = py::cast(this);
        py::list py_list = self.attr("getHistoryFinanceField")();
        return python_list_to_vector<std::pair<size_t, string>>(py_list);
    }

    Parameter getFinanceInfo(const string& market, const string& code) override {
        PYBIND11_OVERLOAD(Parameter, BaseInfoDriver, getFinanceInfo, market, code);
    }

    MarketInfo getMarketInfo(const string& market) override {
        PYBIND11_OVERLOAD_PURE(MarketInfo, BaseInfoDriver, getMarketInfo, market);
    }

    vector<MarketInfo> getAllMarketInfo() override {
        PYBIND11_OVERLOAD_PURE(vector<MarketInfo>, BaseInfoDriver, getAllMarketInfo, );
    }

    vector<StockTypeInfo> getAllStockTypeInfo() override {
        PYBIND11_OVERLOAD_PURE(vector<StockTypeInfo>, BaseInfoDriver, getAllStockTypeInfo, );
    }

    StockTypeInfo getStockTypeInfo(uint32_t type) override {
        PYBIND11_OVERLOAD_PURE(StockTypeInfo, BaseInfoDriver, getStockTypeInfo, type);
    }

    std::unordered_set<Datetime> getAllHolidays() override {
        PYBIND11_OVERLOAD_PURE(std::unordered_set<Datetime>, BaseInfoDriver, getAllHolidays, );
    }

    ZhBond10List getAllZhBond10() override {
        PYBIND11_OVERLOAD_PURE(ZhBond10List, BaseInfoDriver, getAllZhBond10, );
    }
};