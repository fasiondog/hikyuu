/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "hikyuu/trade_sys/multifactor/build_in.h"
#include "hikyuu/trade_sys/multifactor/buildin_scfilter.h"
#include "MultiFactorSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MultiFactorSelector)
#endif

namespace hku {

MultiFactorSelector::MultiFactorSelector() : SelectorBase("SE_MultiFactor") {
    setParam<bool>("only_should_buy", false);  // Select the systems that also issue a buy signal
    setParam<bool>("ignore_null", true);       // Ignore the securities whose score is nan in the MF
    setParam<bool>("ignore_le_zero", false);  // Ignore the securities whose score is <= 0 in the MF
    setParam<int>("topn", 10);
    setParam<bool>("reverse", false);  // Reverse order, topn means the last few (lowest values)
    setParam<int>("ic_n", 5);
    setParam<int>("ic_rolling_n", 120);
    setParam<Stock>("ref_stk", Stock());
    setParam<bool>("use_spearman", true);
    setParam<string>("mode", "MF_ICIRWeight");
    setParam<int>("mf_recover_type",
                  KQuery::INVALID_RECOVER_TYPE);    // The MF calculation adjustment type
    setParam<bool>("keep_mf_recover_type", false);  // Keep the existing recover type of the MF
}

MultiFactorSelector::MultiFactorSelector(const MFPtr& mf, int topn)
: SelectorBase("SE_MultiFactor") {
    HKU_CHECK(mf, "mf is null!");
    m_mf = mf;
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
    setParam<string>("mode", "CUSTOM");
    setParam<int>("mf_recover_type",
                  KQuery::INVALID_RECOVER_TYPE);    // The MF calculation adjustment type
    setParam<bool>("keep_mf_recover_type", false);  // Keep the existing recover type of the MF
    setFactorSet(mf->getRefFactorSet());
}

MultiFactorSelector::~MultiFactorSelector() {}

void MultiFactorSelector::_checkParam(const string& name) const {
    if ("ic_n" == name) {
        HKU_ASSERT(getParam<int>("ic_n") >= 1);
    } else if ("ic_rolling_n" == name) {
        HKU_ASSERT(getParam<int>("ic_rolling_n") >= 1);
    } else if ("mode" == name) {
        auto mode = getParam<string>("mode");
        HKU_ASSERT("MF_ICIRWeight" == mode || "MF_ICWeight" == mode || "MF_EqualWeight" == mode ||
                   "CUSTOM" == mode);
    } else if ("mf_recover_type" == name) {
        int recover_type = getParam<int>("mf_recover_type");
        HKU_ASSERT(recover_type >= KQuery::NO_RECOVER &&
                   recover_type <= KQuery::INVALID_RECOVER_TYPE);
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
    if (m_mf) {
        p->m_mf = m_mf->clone();
    }
    // m_stk_sys_dict references the original system instances, do not carry it into the clone
    p->m_factorset = m_factorset;
    return p;
}

ScoreRecordList MultiFactorSelector::filterOnlyShouldBuy(Datetime date,
                                                         const ScoreRecordList& scores,
                                                         size_t topn) {
    ScoreRecordList ret;
    size_t i = 0, cnt = 0, total = scores.size();
    while (i < total && cnt < topn) {
        auto const& sc = scores[i];
        auto iter = m_stk_sys_dict.find(sc.stock);
        if (iter != m_stk_sys_dict.end() && iter->second &&
            iter->second->getSG()->shouldBuy(date)) {
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
    // Filter before the truncation, so that the filtered-out ones do not consume the topn quota
    ScoreRecordList candidates;
    if (only_should_buy) {
        candidates.reserve(raw_scores.size());
        for (const auto& sc : raw_scores) {
            auto iter = m_stk_sys_dict.find(sc.stock);
            if (iter != m_stk_sys_dict.end() && iter->second &&
                iter->second->getSG()->shouldBuy(date)) {
                candidates.emplace_back(sc);
            }
        }
    } else {
        candidates = raw_scores;
    }

    ScoreRecordList scores;
    auto iter = candidates.rbegin();
    for (size_t count = 0; count < topn && iter != candidates.rend(); ++iter) {
        if (!std::isnan(iter->value)) {
            scores.emplace_back(*iter);
            count++;
        }
    }
    if (scores.size() < topn && !ignore_null) {
        size_t lack = topn - scores.size();
        iter = candidates.rbegin();
        for (size_t count = 0; count < lack && iter != candidates.rend(); ++iter) {
            if (std::isnan(iter->value)) {
                scores.emplace_back(*iter);
                count++;
            } else {
                break;
            }
        }
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

SystemWeightList MultiFactorSelector::_getSelected(Datetime date) {
    HKU_CHECK(m_mf, "MF is not set or the selector has not been calculated!");

    bool ignore_null = getParam<bool>("ignore_null");
    bool ignore_le_zero = getParam<bool>("ignore_le_zero");
    bool only_should_buy = getParam<bool>("only_should_buy");
    bool reverse = getParam<bool>("reverse");

    // ignore_null / ignore_le_zero are expressed as score filters, so the MF drops the filtered
    // records while copying instead of copying the whole cross-section first
    ScoreRecordList scores;
    if (ignore_null) {
        ScoresFilterPtr flag_filter = SCFilter_IgnoreNan();
        if (ignore_le_zero) {
            flag_filter = flag_filter | SCFilter_LessOrEqualValue(0.0);
        }
        scores = m_mf->getScores(date, 0, Null<size_t>(), flag_filter);
    } else if (ignore_le_zero) {
        // Keep the predicate: SCFilter_LessOrEqualValue also drops NaN, which must survive when
        // ignore_null is false (the reverse path re-adds it on purpose)
        scores = m_mf->getScores(date, 0, Null<size_t>(),
                                 [](const ScoreRecord& sc) { return !(sc.value <= 0.0); });
    } else {
        scores = m_mf->getScores(date, 0, Null<size_t>());
    }

    // MultiFactorSelector2 with filters is recommended; this is kept for the old interface
    // compatibility only Apply the user-defined score filters (set_scores_filter /
    // add_scores_filter are supported)
    if (m_sc_filter) {
        scores = m_sc_filter->filter(scores, date, m_query);
    }

    int param_topn = getParam<int>("topn");
    size_t topn = param_topn > 0 ? static_cast<size_t>(param_topn) : scores.size();
    if (topn > scores.size()) {
        topn = scores.size();
    }

    if (!reverse) {
        // Sort in the ascending order
        scores = filterTopN(date, scores, topn, only_should_buy);

    } else {
        // Sort in the descending order
        scores = filterTopNReverse(date, scores, topn, only_should_buy, ignore_null);
    }

    SystemWeightList ret;
    for (const auto& sc : scores) {
        auto iter = m_stk_sys_dict.find(sc.stock);
        if (iter == m_stk_sys_dict.end() || !iter->second) {
            continue;
        }
        ret.emplace_back(iter->second, sc.value);
    }
    return ret;
}

void MultiFactorSelector::_calculate() {
    // Drop the previous mapping first: a second calculate (a new query, a new stock pool, a
    // portfolio re-run) must not keep the systems of the earlier run
    m_stk_sys_dict.clear();

    Stock ref_stk = getParam<Stock>("ref_stk");

    StockList stks;
    for (const auto& sys : m_pro_sys_list) {
        stks.emplace_back(sys->getStock());
    }

    KQuery query = m_query;
    if (getParam<int>("mf_recover_type") != KQuery::INVALID_RECOVER_TYPE) {
        query.recoverType(static_cast<KQuery::RecoverType>(getParam<int>("mf_recover_type")));
    }

    auto ic_n = getParam<int>("ic_n");
    auto ic_rolling_n = getParam<int>("ic_rolling_n");
    bool spearman = getParam<bool>("use_spearman");
    auto mode = getParam<string>("mode");

    if (!m_mf) {
        if ("MF_ICIRWeight" == mode) {
            m_mf = MF_ICIRWeight(m_factorset, stks, query, ref_stk, ic_n, ic_rolling_n, spearman);
        } else if ("MF_ICWeight" == mode) {
            m_mf = MF_ICWeight(m_factorset, stks, query, ref_stk, ic_n, ic_rolling_n, spearman);
        } else if ("MF_EqualWeight" == mode) {
            m_mf = MF_EqualWeight(m_factorset, stks, query, ref_stk, ic_n, spearman);
        } else {
            HKU_THROW("Invalid mode: {}", mode);
        }
    } else {
        if (getParam<bool>("keep_mf_recover_type")) {
            auto mf_query = m_mf->getQuery();
            auto new_query = query;
            new_query.recoverType(mf_query.recoverType());
            m_mf->setQuery(new_query);
        } else {
            m_mf->setQuery(query);
        }
        m_mf->setRefFactorSet(m_factorset);
        m_mf->setRefStock(ref_stk);
        m_mf->setStockList(stks);
        m_mf->setParam<int>("ic_n", ic_n);
        m_mf->setParam<bool>("use_spearman", spearman);
        if (m_mf->haveParam("ic_rolling_n")) {
            m_mf->setParam<int>("ic_rolling_n", ic_rolling_n);
        }
    }

    m_mf->calculate();

    for (const auto& sys : m_real_sys_list) {
        m_stk_sys_dict.insert_or_assign(sys->getStock(), sys);
    }
}

SelectorPtr HKU_API SE_MultiFactor() {
    return make_shared<MultiFactorSelector>();
}

SelectorPtr HKU_API SE_MultiFactor(const MFPtr& mf, int topn) {
    return make_shared<MultiFactorSelector>(mf, topn);
}

SelectorPtr HKU_API SE_MultiFactor(const FactorSet& factorset, int topn, int ic_n, int ic_rolling_n,
                                   const Stock& ref_stk, bool spearman, const string& mode) {
    auto p = make_shared<MultiFactorSelector>();
    p->setFactorSet(factorset);
    p->setParam<int>("topn", topn);
    p->setParam<int>("ic_n", ic_n);
    p->setParam<int>("ic_rolling_n", ic_rolling_n);
    p->setParam<Stock>("ref_stk", ref_stk);
    p->setParam<bool>("use_spearman", spearman);
    p->setParam<string>("mode", mode);
    return p;
}

}  // namespace hku