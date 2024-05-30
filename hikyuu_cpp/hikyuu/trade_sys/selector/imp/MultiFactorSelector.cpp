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

MultiFactorSelector::MultiFactorSelector() : SelectorBase("SE_MultiFactor") {
    // 只选择发出买入信号的系统，此时选中的系统会变成资产平均分配，参考 AF 参数：ignore_zero_weight
    setParam<bool>("only_should_buy", false);
    setParam<int>("topn", 10);
}

MultiFactorSelector::MultiFactorSelector(const MFPtr& mf, int topn)
: SelectorBase("SE_MultiFactor"), m_mf(mf) {
    HKU_CHECK(mf, "mf is null!");
    setParam<bool>("only_should_buy", false);
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
    auto p = make_shared<MultiFactorSelector>();
    p->m_mf = m_mf->clone();
    p->m_stk_sys_dict = m_stk_sys_dict;
    return p;
}

bool MultiFactorSelector::isMatchAF(const AFPtr& af) {
    return true;
}

SystemWeightList MultiFactorSelector::getSelected(Datetime date) {
    SystemWeightList ret;
    auto scores = m_mf->getScores(date, 0, getParam<int>("topn"),
                                  [](const ScoreRecord& sc) { return !std::isnan(sc.value); });
    if (getParam<bool>("only_should_buy")) {
        for (const auto& sc : scores) {
            auto sys = m_stk_sys_dict[sc.stock];
            if (sys->getSG()->shouldBuy(date)) {
                ret.emplace_back(sys, sc.value);
            }
        }

    } else {
        for (const auto& sc : scores) {
            ret.emplace_back(m_stk_sys_dict[sc.stock], sc.value);
        }
    }
    return ret;
}

void MultiFactorSelector::_calculate() {
    for (const auto& sys : m_real_sys_list) {
        m_stk_sys_dict[sys->getStock()] = sys;
    }
}

SelectorPtr HKU_API SE_MultiFactor(const MFPtr& mf, int topn) {
    return make_shared<MultiFactorSelector>(mf, topn);
}

SelectorPtr HKU_API SE_MultiFactor(const IndicatorList& src_inds, const StockList& stks,
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