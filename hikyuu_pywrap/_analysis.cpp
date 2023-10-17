/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-10-10
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/analysis/combinate.h>
#include "pybind_utils.h"

using namespace boost::python;
using namespace hku;

namespace py = boost::python;

static py::list combinate_index(object seq) {
    size_t total = len(seq);
    std::vector<size_t> index_list(total);
    for (size_t i = 0; i < total; ++i) {
        index_list[i] = i;
    }

    py::list result;
    std::vector<std::vector<size_t>> comb = combinateIndex(index_list);
    for (size_t i = 0, count = comb.size(); i < count; i++) {
        py::list tmp = vector_to_py_list<std::vector<size_t>>(comb[i]);
        result.append(tmp);
    }
    return result;
}

static py::list combinate_indicator(object seq, int n) {
    size_t total = len(seq);
    std::vector<Indicator> inds(total);
    for (size_t i = 0; i < total; ++i) {
        inds[i] = py::extract<Indicator>(seq[i])();
    }

    auto comb = combinateIndicator(inds, n);
    return vector_to_py_list(comb);
}

static py::dict combinate_ind_analysis(const Stock& stk, const KQuery& query, TradeManagerPtr tm,
                                       SystemPtr sys, object buy_inds, object sell_inds, int n) {
    std::vector<Indicator> c_buy_inds;
    for (size_t i = 0, total = len(buy_inds); i < total; i++) {
        c_buy_inds.emplace_back(py::extract<Indicator>(buy_inds[i])());
    }

    std::vector<Indicator> c_sell_inds;
    for (size_t i = 0, total = len(sell_inds); i < total; i++) {
        c_sell_inds.emplace_back(py::extract<Indicator>(sell_inds[i])());
    }

    py::dict result;
    auto pers = combinateIndicatorAnalysis(stk, query, tm, sys, c_buy_inds, c_sell_inds, n);
    for (auto iter = pers.begin(); iter != pers.end(); ++iter) {
        result[iter->first] = std::move(iter->second);
    }
    return result;
}

void export_analysis() {
    def("combinate_index", combinate_index, R"(combinate_index(seq)

    获取序列组合的下标索引, 输入序列的长度最大不超过15，否则抛出异常

    :param seq: 可迭代的 python 对象
    :rtype: list

    )");

    def("combinate_ind", combinate_indicator);
    def("combinate_ind_analysis", combinate_ind_analysis);
}