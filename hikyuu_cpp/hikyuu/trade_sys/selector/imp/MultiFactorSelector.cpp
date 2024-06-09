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
    setParam<bool>("ignore_null", true);  // 是否忽略 MF 中 score 值为 nan 的证券
    setParam<int>("topn", 10);
    setParam<int>("ic_n", 5);
    setParam<int>("ic_rolling_n", 120);
    setParam<KQuery>("query", KQuery());
    setParam<Stock>("ref_stk", Stock());
    setParam<string>("mode", "MF_ICIRWeight");
}

MultiFactorSelector::MultiFactorSelector(const MFPtr& mf, int topn)
: SelectorBase("SE_MultiFactor"), m_mf(mf) {
    HKU_CHECK(mf, "mf is null!");
    setParam<bool>("only_should_buy", false);
    setParam<bool>("ignore_null", true);
    setParam<int>("topn", topn);

    setParam<int>("ic_n", mf->getParam<int>("ic_n"));
    setParam<KQuery>("query", mf->getQuery());
    setParam<Stock>("ref_stk", mf->getRefStock());
    if (mf->haveParam("ic_rolling_n")) {
        setParam<int>("ic_rolling_n", mf->getParam<int>("ic_rolling_n"));
    }
    setParam<string>("mode", mf->name());
}

MultiFactorSelector::~MultiFactorSelector() {}

void MultiFactorSelector::_checkParam(const string& name) const {
    if ("ic_n" == name) {
        HKU_ASSERT(getParam<int>("ic_n") >= 1);
    } else if ("ic_rolling_n" == name) {
        HKU_ASSERT(getParam<int>("ic_rolling_n") >= 1);
    } else if ("mode" == name) {
        auto mode = getParam<string>("mode");
        HKU_ASSERT("MF_ICIRWeight" == mode || "MF_ICWeight" == mode || "MF_EqualWeight" == mode);
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
    for (const auto& ind : m_inds) {
        p->m_inds.emplace_back(ind.clone());
    }
    return p;
}

bool MultiFactorSelector::isMatchAF(const AFPtr& af) {
    return true;
}

SystemWeightList MultiFactorSelector::getSelected(Datetime date) {
    SystemWeightList ret;
    int topn = getParam<int>("topn");
    if (topn <= 0) {
        topn = std::numeric_limits<int>::max();
    }

    ScoreRecordList scores;
    if (getParam<bool>("ignore_null")) {
        scores = m_mf->getScores(date, 0, getParam<int>("topn"),
                                 [](const ScoreRecord& sc) { return !std::isnan(sc.value); });
    } else {
        scores = m_mf->getScores(date, 0, getParam<int>("topn"));
    }

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
    Stock ref_stk = getParam<Stock>("ref_stk");
    if (ref_stk.isNull()) {
        ref_stk = getStock("sh000300");
    }

    StockList stks;
    for (const auto& sys : m_pro_sys_list) {
        stks.emplace_back(sys->getStock());
    }

    auto query = getParam<KQuery>("query");
    auto ic_n = getParam<int>("ic_n");
    auto ic_rolling_n = getParam<int>("ic_rolling_n");
    auto mode = getParam<string>("mode");

    if (!m_mf) {
        if ("MF_ICIRWeight" == mode) {
            m_mf = MF_ICIRWeight(m_inds, stks, query, ref_stk, ic_n, ic_rolling_n);
        } else if ("MF_ICWeight" == mode) {
            m_mf = MF_ICWeight(m_inds, stks, query, ref_stk, ic_n, ic_rolling_n);
        } else if ("MF_EqualWeight" == mode) {
            m_mf = MF_EqualWeight(m_inds, stks, query, ref_stk, ic_n);
        } else {
            HKU_THROW("Invalid mode: {}", mode);
        }
    } else {
        m_mf->setQuery(query);
        m_mf->setRefIndicators(m_inds);
        m_mf->setRefStock(ref_stk);
        m_mf->setStockList(stks);
        m_mf->setParam<int>("ic_n", ic_n);
        if (m_mf->haveParam("ic_rolling_n")) {
            m_mf->setParam<int>("ic_rolling_n", ic_rolling_n);
        }
    }

    for (const auto& sys : m_real_sys_list) {
        m_stk_sys_dict[sys->getStock()] = sys;
    }
}

SelectorPtr HKU_API SE_MultiFactor(const MFPtr& mf, int topn) {
    return make_shared<MultiFactorSelector>(mf, topn);
}

SelectorPtr HKU_API SE_MultiFactor(const IndicatorList& src_inds, int topn = 10, int ic_n = 5,
                                   int ic_rolling_n = 120, const Stock& ref_stk = Stock(),
                                   const string& mode = "MF_ICIRWeight") {
    auto p = make_shared<MultiFactorSelector>();
    p->setIndicators(src_inds);
    p->setParam<int>("topn", topn);
    p->setParam<int>("ic_n", ic_n);
    p->setParam<int>("ic_rolling_n", ic_rolling_n);
    p->setParam<Stock>("ref_stk", ref_stk);
    p->setParam<string>("mode", mode);
    return p;
}

}  // namespace hku