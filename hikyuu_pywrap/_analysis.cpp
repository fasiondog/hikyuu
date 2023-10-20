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

static py::dict combinate_ind_analysis_with_block(const Block& blk, const KQuery& query,
                                                  TradeManagerPtr tm, SystemPtr sys,
                                                  object buy_inds, object sell_inds, int n) {
    std::vector<Indicator> c_buy_inds;
    for (size_t i = 0, total = len(buy_inds); i < total; i++) {
        c_buy_inds.emplace_back(py::extract<Indicator>(buy_inds[i])());
    }

    std::vector<Indicator> c_sell_inds;
    for (size_t i = 0, total = len(sell_inds); i < total; i++) {
        c_sell_inds.emplace_back(py::extract<Indicator>(sell_inds[i])());
    }

    auto records =
      combinateIndicatorAnalysisWithBlock(blk, query, tm, sys, c_buy_inds, c_sell_inds, n);

    std::vector<py::list> tmp;

    StringList names{"组合名称", "证券代码", "证券名称"};
    Performance per;
    auto keys = per.names();
    for (const auto& key : keys) {
        names.emplace_back(key);
    }

    for (const auto& name : names) {
        tmp.emplace_back(py::list());
    }

    for (size_t i = 0, total = records.size(); i < total; i++) {
        CombinateAnalysisOutput& record = records[i];
        tmp[0].append(record.combinateName);
        tmp[1].append(record.code);
        tmp[2].append(record.name);
        HKU_WARN_IF(names.size() != record.values.size() + 3, "lenght invalid: {} {}", names.size(),
                    record.values.size());
        for (size_t j = 3, len = names.size(); j < len; j++) {
            tmp[j].append(record.values[j - 3]);
        }
    }

    py::dict result;
    for (size_t i = 0, total = names.size(); i < total; i++) {
        result[names[i]] = tmp[i];
    }

    return result;
}

void export_analysis() {
    def("combinate_index", combinate_index, R"(combinate_index(seq)

    获取序列组合的下标索引, 输入序列的长度最大不超过15，否则抛出异常

    :param inds: list 或 tuple 等可使用索引的可迭代对象
    :return: 返回组合的索引，可用于获取输入中相应索引位置的值
    :rtype: list)");

    def("combinate_ind", combinate_indicator, (arg("inds"), arg("n") = 7),
        R"(combinate_ind(inds[, n=7])

    对输入的指标序列进行组合, 如输入为 [ind1, ind2], 输出为 [EXIST(ind1,n), EXIST(ind2,n), EXIST(ind1,n)&EXIST(ind2,n)]

    :param list|tuple|seq inds: 待组合的指标列表
    :param int n: 指标在 n 周期内存在
    :return: 组合后的指标列表
    :rtype: list)");

    def("_combinate_ind_analysis", combinate_ind_analysis);
    def("_combinate_ind_analysis_with_block", combinate_ind_analysis_with_block);
}