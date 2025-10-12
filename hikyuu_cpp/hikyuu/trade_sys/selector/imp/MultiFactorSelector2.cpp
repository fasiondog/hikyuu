/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-xx-xx
 *      Author: fasiondog
 */

#include "hikyuu/trade_sys/multifactor/build_in.h"
#include "MultiFactorSelector2.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MultiFactorSelector2)
#endif

namespace hku {

MultiFactorSelector2::MultiFactorSelector2() : SelectorBase("SE_MultiFactor2") {
    setParam<bool>("only_should_buy", false);  // 只选择同时发出买入信号的系统
    setParam<bool>("ignore_null", true);       // 忽略 MF 中 score 值为 nan 的证券
    setParam<bool>("ignore_le_zero", false);   // 忽略 MF 中 score 值小于等于 0 的证券
    setParam<int>("group", 5);
    setParam<int>("group_index", 0);
    setParam<int>("ic_n", 5);
    setParam<int>("ic_rolling_n", 120);
    setParam<Stock>("ref_stk", Stock());
    setParam<bool>("use_spearman", true);
    setParam<string>("mode", "MF_ICIRWeight");
    setParam<double>("min_price", 0.0);       // 忽略价格小于等于 min_price 的证券
    setParam<double>("max_price", 100000.0);  // 忽略价格大于等于 max_price 的证券
}

MultiFactorSelector2::MultiFactorSelector2(const MFPtr& mf, int group, int group_index)
: SelectorBase("SE_MultiFactor2"), m_mf(mf) {
    HKU_CHECK(mf, "mf is null!");
    setParam<bool>("only_should_buy", false);
    setParam<bool>("ignore_null", true);
    setParam<bool>("ignore_le_zero", false);
    setParam<int>("group", group);
    setParam<int>("group_index", group_index);
    setParam<double>("min_price", 0.0);       // 忽略价格小于等于 min_price 的证券
    setParam<double>("max_price", 100000.0);  // 忽略价格大于等于 max_price 的证券

    setParam<int>("ic_n", mf->getParam<int>("ic_n"));
    setParam<Stock>("ref_stk", mf->getRefStock());
    if (mf->haveParam("ic_rolling_n")) {
        setParam<int>("ic_rolling_n", mf->getParam<int>("ic_rolling_n"));
    } else {
        setParam<int>("ic_rolling_n", 120);
    }
    setParam<bool>("use_spearman", mf->getParam<bool>("use_spearman"));
    setParam<string>("mode", "CUSTOM");
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
    } else if ("group" == name) {
        HKU_ASSERT(getParam<int>("group") >= 1);
    } else if ("group_index" == name) {
        HKU_ASSERT(getParam<int>("group_index") >= 0 &&
                   getParam<int>("group_index") < getParam<int>("group"));
    } else if ("min_price" == name) {
        double min_price = getParam<double>("min_price");
        HKU_ASSERT(!std::isnan(min_price));
    } else if ("max_price" == name) {
        double max_price = getParam<double>("max_price");
        HKU_ASSERT(!std::isnan(max_price) && max_price > getParam<double>("min_price"));
    }
}

void MultiFactorSelector2::_reset() {
    if (m_mf) {
        m_mf->reset();
    }
    m_stk_sys_dict.clear();
}

SelectorPtr MultiFactorSelector2::_clone() {
    auto p = make_shared<MultiFactorSelector2>();
    p->m_mf = m_mf->clone();
    p->m_stk_sys_dict = m_stk_sys_dict;
    for (const auto& ind : m_inds) {
        p->m_inds.emplace_back(ind.clone());
    }
    return p;
}

bool MultiFactorSelector2::isMatchAF(const AFPtr& af) {
    return true;
}

ScoreRecordList MultiFactorSelector2::filterByGroup(Datetime date,
                                                    const ScoreRecordList& raw_scores, size_t group,
                                                    size_t group_index, bool only_should_buy) {
    ScoreRecordList ret;

    // 如果没有数据或分组数为0，直接返回空列表
    if (raw_scores.empty() || group == 0) {
        return ret;
    }

    // 计算每组应包含的股票数
    size_t total_count = raw_scores.size();
    size_t stocks_per_group = (total_count + group - 1) / group;  // 向上取整

    // 检查索引是否有效
    if (group_index >= group) {
        return ret;
    }

    // 计算当前组的起始和结束位置
    size_t start = group_index * stocks_per_group;
    size_t end = std::min(start + stocks_per_group, total_count);

    // 如果起始位置超出范围，返回空列表
    if (start >= total_count) {
        return ret;
    }

    // 从原始数据中提取当前组的数据
    ScoreRecordList group_scores;
    for (size_t i = start; i < end; i++) {
        group_scores.emplace_back(raw_scores[i]);
    }

    // 如果只需要买入信号的股票，则进行过滤
    if (only_should_buy) {
        for (const auto& sc : group_scores) {
            auto sys = m_stk_sys_dict[sc.stock];
            if (sys->getSG()->shouldBuy(date)) {
                ret.emplace_back(sc);
            }
        }
    } else {
        ret = std::move(group_scores);
    }

    return ret;
}

SystemWeightList MultiFactorSelector2::_getSelected(Datetime date) {
    bool ignore_null = getParam<bool>("ignore_null");
    bool ignore_le_zero = getParam<bool>("ignore_le_zero");
    bool only_should_buy = getParam<bool>("only_should_buy");

    ScoreRecordList scores;
    double min_price = getParam<double>("min_price");
    double max_price = getParam<double>("max_price");
    if (min_price > 0.0 || max_price < 100000.0) {
        auto ktype = m_mf->getQuery().kType();
        scores = m_mf->getScores(
          date, 0, Null<size_t>(),
          [ktype, ignore_null, ignore_le_zero, min_price, max_price](const Datetime& date,
                                                                     const ScoreRecord& sc) {
              HKU_IF_RETURN(sc.stock.isNull() || (ignore_null && std::isnan(sc.value)) ||
                              (ignore_le_zero && sc.value <= 0.0),
                            false);
              auto kr = sc.stock.getKRecord(date, ktype);
              return kr.isValid() && kr.closePrice >= min_price && kr.closePrice <= max_price;
          });
    } else {
        scores = m_mf->getScores(
          date, 0, Null<size_t>(), [ignore_null, ignore_le_zero](const ScoreRecord& sc) {
              return !(ignore_null && std::isnan(sc.value)) && !(ignore_le_zero && sc.value <= 0.0);
          });
    }

    // 按照评分排序
    std::sort(scores.begin(), scores.end(), [](const ScoreRecord& a, const ScoreRecord& b) {
        return a.value > b.value;  // 降序排列
    });

    int group = getParam<int>("group");
    int group_index = getParam<int>("group_index");

    // 分组并选择指定组
    scores = filterByGroup(date, scores, group, group_index, only_should_buy);

    SystemWeightList ret;
    for (const auto& sc : scores) {
        ret.emplace_back(m_stk_sys_dict[sc.stock], sc.value);
    }
    return ret;
}

void MultiFactorSelector2::_calculate() {
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

SelectorPtr HKU_API SE_MultiFactor2(const MFPtr& mf, int group, int group_index) {
    return make_shared<MultiFactorSelector2>(mf, group, group_index);
}

SelectorPtr HKU_API SE_MultiFactor2(const IndicatorList& src_inds, int group, int group_index,
                                    int ic_n, int ic_rolling_n, const Stock& ref_stk, bool spearman,
                                    const string& mode) {
    auto p = make_shared<MultiFactorSelector2>();
    p->setIndicators(src_inds);
    p->setParam<int>("group", group);
    p->setParam<int>("group_index", group_index);
    p->setParam<int>("ic_n", ic_n);
    p->setParam<int>("ic_rolling_n", ic_rolling_n);
    p->setParam<Stock>("ref_stk", ref_stk);
    p->setParam<bool>("use_spearman", spearman);
    p->setParam<string>("mode", mode);
    return p;
}

}  // namespace hku