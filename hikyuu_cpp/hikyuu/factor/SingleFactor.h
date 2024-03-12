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

class SingleFactor {
    PARAMETER_SUPPORT

public:
    typedef Indicator::value_t value_t;

public:
    SingleFactor();
    SingleFactor(const Indicator& ind, const StockList& stks, const KQuery& query,
                 const Stock& ref_stk);

    void calculate();

    const Indicator& get(const Stock&) const;

    const vector<std::pair<Stock, value_t>>& get(const Datetime&) const;

    Indicator getIC(int ndays) const;
    Indicator getICIR(int ic_n, int ir_n) const;

private:
    Indicator m_ind;
    StockList m_stks;
    Stock m_ref_stk;
    KQuery m_query;
    unordered_map<Stock, size_t> m_stk_map;
    vector<Indicator> m_all_inds;

    unordered_map<Datetime, size_t> m_date_index;
    vector<vector<std::pair<Stock, value_t>>> m_stk_factor_by_date;
};

}  // namespace hku