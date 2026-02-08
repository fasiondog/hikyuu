/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-xx-xx
 *      Author: fasiondog
 */

#include "hikyuu/trade_sys/multifactor/build_in.h"
#include "hikyuu/trade_sys/portfolio/Portfolio.h"
#include "MultiFactorSelector2.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MultiFactorSelector2)
#endif

namespace hku {

MultiFactorSelector2::MultiFactorSelector2() : SelectorBase("SE_MultiFactor2") {
    setParam<int>("ic_n", 5);
    setParam<int>("ic_rolling_n", 120);
    setParam<Stock>("ref_stk", Stock());
    setParam<bool>("use_spearman", true);
    setParam<string>("mode", "MF_ICIRWeight");
    setParam<int>("mf_recover_type", KQuery::INVALID_RECOVER_TYPE);  // 指定MF的计算时的复权方式
}

MultiFactorSelector2::MultiFactorSelector2(const MFPtr& mf) : SelectorBase("SE_MultiFactor2") {
    HKU_CHECK(mf, "mf is null!");
    m_mf = mf;
    setParam<int>("ic_n", mf->getParam<int>("ic_n"));
    setParam<Stock>("ref_stk", mf->getRefStock());
    if (mf->haveParam("ic_rolling_n")) {
        setParam<int>("ic_rolling_n", mf->getParam<int>("ic_rolling_n"));
    } else {
        setParam<int>("ic_rolling_n", 120);
    }
    setParam<bool>("use_spearman", mf->getParam<bool>("use_spearman"));
    setParam<string>("mode", "CUSTOM");
    setParam<int>("mf_recover_type", KQuery::INVALID_RECOVER_TYPE);  // 指定MF的计算时的复权方式
    setIndicators(mf->getRefIndicators());
}

MultiFactorSelector2::~MultiFactorSelector2() {}

void MultiFactorSelector2::_checkParam(const string& name) const {
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

void MultiFactorSelector2::_reset() {
    if (m_mf) {
        m_mf->reset();
    }
    m_sys_weight_dict.clear();
}

SelectorPtr MultiFactorSelector2::_clone() {
    auto p = make_shared<MultiFactorSelector2>();
    p->m_mf = m_mf->clone();
    p->m_sys_weight_dict = m_sys_weight_dict;
    for (const auto& ind : m_inds) {
        p->m_inds.emplace_back(ind.clone());
    }
    return p;
}

bool MultiFactorSelector2::isMatchAF(const AFPtr& af) {
    return true;
}

SystemWeightList MultiFactorSelector2::_getSelected(Datetime date) {
    SystemWeightList ret;
    auto iter = m_sys_weight_dict.find(date);
    if (iter != m_sys_weight_dict.end()) {
        ret = iter->second;
    }
    return ret;
}

void MultiFactorSelector2::_calculate() {
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

    m_mf->calculate();

    unordered_map<Stock, SYSPtr> stk_sys_dict;
    for (const auto& sys : m_real_sys_list) {
        stk_sys_dict.insert({sys->getStock(), sys});
    }

    auto pf = getPF();
    DatetimeList adjust_dates =
      pf ? pf->getAdjustDates() : StockManager::instance().getTradingCalendar(m_query);
    for (const auto& date : adjust_dates) {
        m_sys_weight_dict[date];
    }

    global_parallel_for_index_void(
      0, adjust_dates.size(), [&, mf = m_mf.get(), filter = m_sc_filter->clone(), this](size_t i) {
          const auto& date = adjust_dates[i];
          ScoreRecordList scores = mf->getScores(date, 0, Null<size_t>(), filter);
          SystemWeightList weights;
          for (const auto& sc : scores) {
              weights.emplace_back(stk_sys_dict[sc.stock], sc.value);
          }
          auto iter = m_sys_weight_dict.find(date);
          if (iter != m_sys_weight_dict.end()) {
              iter->second = std::move(weights);
          }
      });

    m_mf->reset();
}

SelectorPtr HKU_API SE_MultiFactor2(const MFPtr& mf, const ScoresFilterPtr& filter) {
    auto p = make_shared<MultiFactorSelector2>(mf);
    p->setScoresFilter(filter);
    return p;
}

SelectorPtr HKU_API SE_MultiFactor2(const IndicatorList& src_inds, int ic_n, int ic_rolling_n,
                                    const Stock& ref_stk, bool spearman, const string& mode,
                                    const ScoresFilterPtr& filter) {
    auto p = make_shared<MultiFactorSelector2>();
    p->setIndicators(src_inds);
    p->setParam<int>("ic_n", ic_n);
    p->setParam<int>("ic_rolling_n", ic_rolling_n);
    p->setParam<Stock>("ref_stk", ref_stk);
    p->setParam<bool>("use_spearman", spearman);
    p->setParam<string>("mode", mode);
    p->setScoresFilter(filter);
    return p;
}

}  // namespace hku