/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/KData.h"
#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/utilities/Parameter.h"

namespace hku {

class MultiFactor {
    PARAMETER_SUPPORT

public:
    typedef Indicator::value_t value_t;

public:
    MultiFactor() = delete;
    MultiFactor(const vector<Indicator>& inds, const StockList& stks, const KQuery& query,
                const Stock& ref_stk);

    void calculate();

    /** 获取指定证券合成因子 */
    const Indicator& get(const Stock&) const;

    /** 获取指定日期截面的所有因子值 */
    const vector<std::pair<Stock, value_t>>& get(const Datetime&) const;

    /** 获取合成因子的IC */
    Indicator getIC(int ndays) const;

    /** 获取合成因子的 ICIR */
    Indicator getICIR(int ic_n, int ir_n) const;

private:
    vector<vector<Indicator>> _alignAllInds();
    vector<Indicator> _getAllReturns(int ndays) const;
    void _calculateAllFactorsByEqualWeight(const vector<vector<Indicator>>&);
    void _calculateAllFactorsByIC(const vector<vector<Indicator>>&);
    void _calculateAllFactorsByICIR(const vector<vector<Indicator>>&);

private:
    std::mutex m_mutex;
    vector<Indicator> m_inds;
    StockList m_stks;
    Stock m_ref_stk;
    KQuery m_query;
    unordered_map<Stock, size_t> m_stk_map;
    vector<Indicator> m_all_factors;

    unordered_map<Datetime, size_t> m_date_index;
    vector<vector<std::pair<Stock, value_t>>> m_stk_factor_by_date;

    DatetimeList m_ref_dates;
};

}  // namespace hku