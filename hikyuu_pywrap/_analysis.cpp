/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-10-10
 *      Author: fasiondog
 */

#include <hikyuu/analysis/combinate.h>
#include <hikyuu/analysis/analysis_sys.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

static py::list combinate_index(py::object seq) {
    size_t total = len(seq);
    std::vector<size_t> index_list(total);
    for (size_t i = 0; i < total; ++i) {
        index_list[i] = i;
    }

    py::list result;
    std::vector<std::vector<size_t>> comb = combinateIndex(index_list);
    for (size_t i = 0, count = comb.size(); i < count; i++) {
        py::list tmp = vector_to_python_list<size_t>(comb[i]);
        result.append(tmp);
    }
    return result;
}

static py::list combinate_indicator(const py::sequence& seq, int n) {
    size_t total = len(seq);
    std::vector<Indicator> inds(total);
    for (size_t i = 0; i < total; ++i) {
        inds[i] = seq[i].cast<Indicator>();
    }

    auto comb = combinateIndicator(inds, n);
    return vector_to_python_list(comb);
}

static py::dict combinate_ind_analysis(const Stock& stk, const KQuery& query, TradeManagerPtr tm,
                                       SystemPtr sys, py::sequence buy_inds, py::sequence sell_inds,
                                       int n) {
    std::vector<Indicator> c_buy_inds;
    for (size_t i = 0, total = len(buy_inds); i < total; i++) {
        c_buy_inds.emplace_back(buy_inds[i].cast<Indicator>());
    }

    std::vector<Indicator> c_sell_inds;
    for (size_t i = 0, total = len(sell_inds); i < total; i++) {
        c_sell_inds.emplace_back(sell_inds[i].cast<Indicator>());
    }

    std::map<std::string, Performance> pers;
    {
        OStreamToPython guard(false);
        py::gil_scoped_release release;
        pers = combinateIndicatorAnalysis(stk, query, tm, sys, c_buy_inds, c_sell_inds, n);
    }

    py::dict result;
    for (auto iter = pers.begin(); iter != pers.end(); ++iter) {
        result[iter->first.c_str()] = std::move(iter->second);
    }
    return result;
}

static py::dict combinate_ind_analysis_with_block(const Block& blk, const KQuery& query,
                                                  TradeManagerPtr tm, SystemPtr sys,
                                                  const py::sequence& buy_inds,
                                                  const py::sequence& sell_inds, int n) {
    std::vector<Indicator> c_buy_inds;
    for (size_t i = 0, total = len(buy_inds); i < total; i++) {
        c_buy_inds.emplace_back(buy_inds[i].cast<Indicator>());
    }

    std::vector<Indicator> c_sell_inds;
    for (size_t i = 0, total = len(sell_inds); i < total; i++) {
        c_sell_inds.emplace_back(sell_inds[i].cast<Indicator>());
    }

    vector<CombinateAnalysisOutput> records;
    {
        OStreamToPython guard(false);
        py::gil_scoped_release release;
        records =
          combinateIndicatorAnalysisWithBlock(blk, query, tm, sys, c_buy_inds, c_sell_inds, n);
    }

    std::vector<py::list> tmp;

    StringList names{"组合名称", "证券代码", "证券名称"};
    Performance per;
    auto keys = per.names();
    for (const auto& key : keys) {
        names.emplace_back(key);
    }

    for (size_t i = 0, len = names.size(); i < len; i++) {
        tmp.emplace_back(py::list());
    }

    for (size_t i = 0, total = records.size(); i < total; i++) {
        CombinateAnalysisOutput& record = records[i];
        tmp[0].append(record.combinateName);
        tmp[1].append(record.market_code);
        tmp[2].append(record.name);
        HKU_WARN_IF(names.size() != record.values.size() + 3, "lenght invalid: {} {}", names.size(),
                    record.values.size());
        for (size_t j = 3, len = names.size(); j < len; j++) {
            tmp[j].append(record.values[j - 3]);
        }
    }

    py::dict result;
    for (size_t i = 0, total = names.size(); i < total; i++) {
        result[names[i].c_str()] = tmp[i];
    }

    return result;
}

static py::dict analysis_sys_list(const py::object& pystk_list, const KQuery& query,
                                  SystemPtr sys_proto) {
    HKU_CHECK(sys_proto, "sys_proto is null!");

    sys_proto->forceResetAll();
    SystemList sys_list;
    StockList stk_list;

    if (py::isinstance<Block>(pystk_list)) {
        const auto& blk = pystk_list.cast<Block&>();
        for (const auto& stk : blk) {
            sys_list.emplace_back(std::move(sys_proto->clone()));
            stk_list.emplace_back(stk);
        }
    } else if (py::isinstance<StockManager>(pystk_list)) {
        const auto& blk = pystk_list.cast<StockManager&>();
        for (const auto& stk : blk) {
            sys_list.emplace_back(std::move(sys_proto->clone()));
            stk_list.emplace_back(stk);
        }
    } else if (py::isinstance<py::sequence>(pystk_list)) {
        auto pyseq = pystk_list.cast<py::sequence>();
        for (const auto& obj : pyseq) {
            sys_list.emplace_back(std::move(sys_proto->clone()));
            stk_list.emplace_back(obj.cast<Stock&>());
        }
    }

    vector<AnalysisSystemWithBlockOut> records;
    {
        OStreamToPython guard(false);
        py::gil_scoped_release release;
        records = analysisSystemList(sys_list, stk_list, query);
        // records = analysisSystemList(stk_list, query, sys_proto);
    }

    Performance per;
    auto keys = per.names();
    std::vector<py::list> tmp(keys.size() + 2);
    for (size_t i = 0, total = records.size(); i < total; i++) {
        const auto& record = records[i];
        if (record.values.size() != keys.size()) {
            continue;
        }
        tmp[0].append(record.market_code);
        tmp[1].append(record.name);
        for (size_t j = 0, len = keys.size(); j < len; j++) {
            tmp[j + 2].append(record.values[j]);
        }
    }

    py::dict result;
    result["证券代码"] = tmp[0];
    result["证券名称"] = tmp[1];
    for (size_t i = 0, total = keys.size(); i < total; i++) {
        if (!tmp[i + 2].empty()) {
            result[keys[i].c_str()] = tmp[i + 2];
        }
    }
    return result;
}

void export_analysis(py::module& m) {
    m.def("combinate_index", combinate_index, R"(combinate_index(seq)

    获取序列组合的下标索引, 输入序列的长度最大不超过15，否则抛出异常

    :param inds: list 或 tuple 等可使用索引的可迭代对象
    :return: 返回组合的索引，可用于获取输入中相应索引位置的值
    :rtype: list)");

    m.def("combinate_ind", combinate_indicator, py::arg("inds"), py::arg("n") = 7,
          R"(combinate_ind(inds[, n=7])

    对输入的指标序列进行组合, 如输入为 [ind1, ind2], 输出为 [EXIST(ind1,n), EXIST(ind2,n),
    EXIST(ind1,n)&EXIST(ind2,n)]

    :param list|tuple|seq inds: 待组合的指标列表
    :param int n: 指标在 n 周期内存在
    :return: 组合后的指标列表
    :rtype: list)");

    m.def("inner_combinate_ind_analysis", combinate_ind_analysis);
    m.def("inner_combinate_ind_analysis_with_block", combinate_ind_analysis_with_block);

    m.def("inner_analysis_sys_list", analysis_sys_list);
}