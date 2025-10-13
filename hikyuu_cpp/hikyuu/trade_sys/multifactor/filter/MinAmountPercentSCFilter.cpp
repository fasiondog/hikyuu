/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#include "MinAmountPercentSCFilter.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MinAmountPercentSCFilter)
#endif

namespace hku {

MinAmountPercentSCFilter::MinAmountPercentSCFilter() : ScoresFilterBase("SCFilter_AmountLimit") {
    setParam<double>("min_amount_percent_limit", 0.1);
}

void MinAmountPercentSCFilter::_checkParam(const string& name) const {
    if (name == "min_amount_percent_limit") {
        double min_amount_percent_limit = getParam<double>("min_amount_percent_limit");
        HKU_CHECK(min_amount_percent_limit >= 0.0 && min_amount_percent_limit <= 1.0,
                  "min_amount_percent_limit must in [0.0, 1.0]!");
    }
}

ScoreRecordList MinAmountPercentSCFilter::_filter(const ScoreRecordList& scores,
                                                  const Datetime& date, const KQuery& query) {
    ScoreRecordList ret;
    const auto& ktype = query.kType();

    std::vector<std::pair<size_t, price_t>> amount_list;
    for (size_t i = 0; i < scores.size(); ++i) {
        if (scores[i].stock.isNull()) {
            continue;
        }

        auto kr = scores[i].stock.getKRecord(date, ktype);
        if (!kr.isValid()) {
            continue;
        }

        amount_list.emplace_back(i, kr.transAmount);
    }

    std::sort(amount_list.begin(), amount_list.end(),
              [](const std::pair<size_t, price_t>& a, const std::pair<size_t, price_t>& b) {
                  return a.second > b.second;
              });

    // 计算需要保留的数量(过滤掉后10%)
    size_t total = amount_list.size();
    if (total == 0) {
        return ret;
    }

    double min_amount_percent_limit = getParam<double>("min_amount_percent_limit");
    size_t keep_count = total * (1 - min_amount_percent_limit);  // 保留前90%
    if (keep_count == 0) {
        keep_count = 1;  // 至少保留一个
    }

    // 将前keep_count个记录加入结果
    for (size_t i = 0; i < keep_count && i < amount_list.size(); ++i) {
        size_t index = amount_list[i].first;
        ret.emplace_back(scores[index]);
    }

    return ret;
}

ScoresFilterPtr HKU_API SCFilter_AmountLimit(double min_amount_percent_limit) {
    auto p = std::make_shared<MinAmountPercentSCFilter>();
    p->setParam<double>("min_amount_percent_limit", min_amount_percent_limit);
    return p;
}

}  // namespace hku