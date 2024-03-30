/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "hikyuu/trade_sys/multifactor/build_in.h"
#include "MultiFactorSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MultiFactorSelector)
#endif

namespace hku {

MultiFactorSelector::MultiFactorSelector() : SelectorBase("SE_Multi_factor") {
    setParam<int>("topn", 10);
}

MultiFactorSelector::MultiFactorSelector(const MFPtr& mf, int topn)
: SelectorBase("SE_Multi_factor"), m_mf(mf) {
    HKU_CHECK(mf, "mf is null!");
    setParam<int>("topn", topn);
    checkParam("topn");
}

MultiFactorSelector::~MultiFactorSelector() {}

void MultiFactorSelector::_checkParam(const string& name) const {
    if ("topn" == name) {
        int topn = getParam<int>("topn");
        HKU_ASSERT(topn > 0);
    }
}

void MultiFactorSelector::_reset() {
    if (m_mf) {
        m_mf->reset();
    }
    m_stk_sys_dict.clear();
}

SelectorPtr MultiFactorSelector::_clone() {
    MultiFactorSelector* p = new MultiFactorSelector();
    p->m_mf = m_mf->clone();
    p->m_stk_sys_dict = m_stk_sys_dict;
    return SelectorPtr(p);
}

bool MultiFactorSelector::isMatchAF(const AFPtr& af) {
    return true;
}

SystemWeightList MultiFactorSelector::getSelected(Datetime date) {
    SystemWeightList ret;
    auto scores = m_mf->getScores(date, 0, getParam<int>("topn"),
                                  [](const ScoreRecord& sc) { return !std::isnan(sc.value); });
    for (const auto& sc : scores) {
        ret.emplace_back(m_stk_sys_dict[sc.stock], sc.value);
    }
    return ret;
}

void MultiFactorSelector::_calculate() {
    for (const auto& sys : m_real_sys_list) {
        m_stk_sys_dict[sys->getStock()] = sys;
    }
}

SelectorPtr HKU_API SE_Multi_factor(const MFPtr& mf, int topn) {
    return make_shared<MultiFactorSelector>(mf, topn);
}

SelectorPtr HKU_API SE_Multi_factor(const IndicatorList& src_inds, const StockList& stks,
                                    const KQuery& query, int topn, int ic_n, int ic_rolling_n,
                                    const Stock& ref_stk, const string& mode) {
    Stock n_ref_stk = ref_stk.isNull() ? getStock("sh000300") : ref_stk;
    MFPtr mf;
    if ("MF_ICIRWeight" == mode) {
        mf = MF_ICIRWeight(src_inds, stks, query, n_ref_stk, ic_n, ic_rolling_n);
    } else if ("MF_ICWeight" == mode) {
        mf = MF_ICWeight(src_inds, stks, query, n_ref_stk, ic_n, ic_rolling_n);
    } else if ("MF_EqualWeight" == mode) {
        mf = MF_EqualWeight(src_inds, stks, query, n_ref_stk, ic_n);
    } else {
        HKU_THROW("Invalid mode: {}", mode);
    }

    return make_shared<MultiFactorSelector>(mf, topn);
}

}  // namespace hku