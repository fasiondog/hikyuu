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
    MultiFactor();
    MultiFactor(const vector<Indicator>& inds, const StockList& stks, const KQuery& query,
                const Stock& ref_stk);

    void calculate();

    /** 获取指定证券合成因子 */
    const Indicator& get(const Stock&) const;

    /** 获取指定日期截面的所有因子值 */
    const vector<std::pair<Stock, value_t>>& get(const Datetime&) const;

    Indicator getIC(int ndays) const;
    Indicator getICIR(int ic_n, int ir_n) const;

private:
    void _alignAllInds();
    vector<Indicator> _getAllReturns(int ndays) const;
    void _calculateAllFactorsByEqualWeight();
    void _calculateAllFactorsByIC();
    void _calculateAllFactorsByICIR();

private:
    vector<Indicator> m_inds;
    StockList m_stks;
    Stock m_ref_stk;
    KQuery m_query;
    unordered_map<Stock, size_t> m_stk_map;
    vector<Indicator> m_all_factors;

    unordered_map<Datetime, size_t> m_date_index;
    vector<vector<std::pair<Stock, value_t>>> m_stk_factor_by_date;

    vector<vector<Indicator>> m_all_stk_inds;  // stock * inds
    DatetimeList m_ref_dates;
};

}  // namespace hku