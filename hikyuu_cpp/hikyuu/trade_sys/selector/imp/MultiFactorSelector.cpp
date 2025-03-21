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
    setParam<bool>("only_should_buy", false);  // 只选择同时发出买入信号的系统
    setParam<bool>("ignore_null", true);       // 忽略 MF 中 score 值为 nan 的证券
    setParam<bool>("ignore_le_zero", false);   // 忽略 MF 中 score 值小于等于 0 的证券
    setParam<int>("topn", 10);
    setParam<bool>("reverse", false);  // 逆序，此时 topn 代表最末尾的几个，相当于按最低值排序
    setParam<int>("ic_n", 5);
    setParam<int>("ic_rolling_n", 120);
    setParam<Stock>("ref_stk", Stock());
    setParam<bool>("use_spearman", true);
    setParam<string>("mode", "MF_ICIRWeight");
}

MultiFactorSelector::MultiFactorSelector(const MFPtr& mf, int topn)
: SelectorBase("SE_MultiFactor"), m_mf(mf) {
    HKU_CHECK(mf, "mf is null!");
    setParam<bool>("only_should_buy", false);
    setParam<bool>("ignore_null", true);
    setParam<bool>("ignore_le_zero", false);
    setParam<int>("topn", topn);
    setParam<bool>("reverse", false);

    setParam<int>("ic_n", mf->getParam<int>("ic_n"));
    setParam<Stock>("ref_stk", mf->getRefStock());
    if (mf->haveParam("ic_rolling_n")) {
        setParam<int>("ic_rolling_n", mf->getParam<int>("ic_rolling_n"));
    } else {
        setParam<int>("ic_rolling_n", 120);
    }
    setParam<bool>("use_spearman", mf->getParam<bool>("use_spearman"));
    setParam<string>("mode", mf->name());
    setIndicators(mf->getRefIndicators());
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

ScoreRecordList MultiFactorSelector::filterOnlyShouldBuy(Datetime date,
                                                         const ScoreRecordList& scores,
                                                         size_t topn) {
    ScoreRecordList ret;
    size_t i = 0, cnt = 0, total = scores.size();
    while (i < total && cnt < topn) {
        auto const& sc = scores[i];
        auto sys = m_stk_sys_dict[sc.stock];
        if (sys->getSG()->shouldBuy(date)) {
            ret.emplace_back(sc);
            cnt++;
        }
        i++;
    }
    return ret;
}

ScoreRecordList MultiFactorSelector::filterTopNReverse(Datetime date,
                                                       const ScoreRecordList& raw_scores,
                                                       size_t topn, bool only_should_buy,
                                                       bool ignore_null) {
    ScoreRecordList scores;
    auto iter = raw_scores.rbegin();
    for (size_t count = 0; count < topn && iter != raw_scores.rend(); ++iter) {
        if (!std::isnan(iter->value)) {
            scores.emplace_back(*iter);
            count++;
        }
    }
    if (scores.size() < topn && !ignore_null) {
        size_t lack = topn - scores.size();
        iter = raw_scores.rbegin();
        for (size_t count = 0; count < lack && iter != raw_scores.rend(); ++iter) {
            if (std::isnan(iter->value)) {
                scores.emplace_back(*iter);
                count++;
            } else {
                break;
            }
        }
    }
    if (only_should_buy) {
        scores = filterOnlyShouldBuy(date, scores, topn);
    }

    return scores;
}

ScoreRecordList MultiFactorSelector::filterTopN(Datetime date, const ScoreRecordList& raw_scores,
                                                size_t topn, bool only_should_buy) {
    ScoreRecordList scores;
    if (only_should_buy) {
        scores = filterOnlyShouldBuy(date, raw_scores, topn);
    } else {
        scores.assign(raw_scores.begin(), raw_scores.begin() + std::min(topn, raw_scores.size()));
    }
    return scores;
}

SystemWeightList MultiFactorSelector::getSelected(Datetime date) {
    bool ignore_null = getParam<bool>("ignore_null");
    bool ignore_le_zero = getParam<bool>("ignore_le_zero");
    bool only_should_buy = getParam<bool>("only_should_buy");
    bool reverse = getParam<bool>("reverse");

    ScoreRecordList raw_scores = m_mf->getScores(date);
    raw_scores.erase(std::remove_if(raw_scores.begin(), raw_scores.end(),
                                    [ignore_null, ignore_le_zero](const ScoreRecord& sc) {
                                        return (ignore_null && std::isnan(sc.value)) ||
                                               (ignore_le_zero && sc.value <= 0.0);
                                    }),
                     raw_scores.end());

    int param_topn = getParam<int>("topn");
    size_t topn = param_topn > 0 ? static_cast<size_t>(param_topn) : raw_scores.size();
    if (topn > raw_scores.size()) {
        topn = raw_scores.size();
    }

    ScoreRecordList scores;
    if (!reverse) {
        // 正序排列
        scores = filterTopN(date, raw_scores, topn, only_should_buy);

    } else {
        // 倒序排列
        scores = filterTopNReverse(date, raw_scores, topn, only_should_buy, ignore_null);
    }

    SystemWeightList ret;
    for (const auto& sc : scores) {
        ret.emplace_back(m_stk_sys_dict[sc.stock], sc.value);
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

    const auto& query = m_query;
    auto ic_n = getParam<int>("ic_n");
    auto ic_rolling_n = getParam<int>("ic_rolling_n");
    bool spearman = getParam<bool>("use_spearman");
    auto mode = getParam<string>("mode");

    if (!m_mf) {
        if ("MF_ICIRWeight" == mode) {
            m_mf = MF_ICIRWeight(m_inds, stks, query, ref_stk, ic_n, ic_rolling_n, spearman);
        } else if ("MF_ICWeight" == mode) {
            m_mf = MF_ICWeight(m_inds, stks, query, ref_stk, ic_n, ic_rolling_n, spearman);
        } else if ("MF_EqualWeight" == mode) {
            m_mf = MF_EqualWeight(m_inds, stks, query, ref_stk, ic_n, spearman);
        } else {
            HKU_THROW("Invalid mode: {}", mode);
        }
    } else {
        m_mf->setQuery(query);
        m_mf->setRefIndicators(m_inds);
        m_mf->setRefStock(ref_stk);
        m_mf->setStockList(stks);
        m_mf->setParam<int>("ic_n", ic_n);
        m_mf->setParam<bool>("use_spearman", spearman);
        if (m_mf->haveParam("ic_rolling_n")) {
            m_mf->setParam<int>("ic_rolling_n", ic_rolling_n);
        }
    }

    for (const auto& sys : m_real_sys_list) {
        m_stk_sys_dict.insert({sys->getStock(), sys});
    }
}

SelectorPtr HKU_API SE_MultiFactor(const MFPtr& mf, int topn) {
    return make_shared<MultiFactorSelector>(mf, topn);
}

SelectorPtr HKU_API SE_MultiFactor(const IndicatorList& src_inds, int topn, int ic_n,
                                   int ic_rolling_n, const Stock& ref_stk, bool spearman,
                                   const string& mode) {
    auto p = make_shared<MultiFactorSelector>();
    p->setIndicators(src_inds);
    p->setParam<int>("topn", topn);
    p->setParam<int>("ic_n", ic_n);
    p->setParam<int>("ic_rolling_n", ic_rolling_n);
    p->setParam<Stock>("ref_stk", ref_stk);
    p->setParam<bool>("use_spearman", spearman);
    p->setParam<string>("mode", mode);
    return p;
}

}  // namespace hku