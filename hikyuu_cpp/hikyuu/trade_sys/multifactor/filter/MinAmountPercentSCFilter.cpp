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

    // The market code breaks the ties, so equal amounts keep a deterministic order
    std::sort(amount_list.begin(), amount_list.end(),
              [&scores](const std::pair<size_t, price_t>& a, const std::pair<size_t, price_t>& b) {
                  if (a.second != b.second) {
                      return a.second > b.second;
                  }
                  return scores[a.first].stock.market_code() <
                         scores[b.first].stock.market_code();
              });

    // Calculate the number to keep (the last 10% is filtered out)
    size_t total = amount_list.size();
    if (total == 0) {
        return ret;
    }

    double min_amount_percent_limit = getParam<double>("min_amount_percent_limit");
    size_t keep_count = std::max<size_t>(
      1, static_cast<size_t>(std::llround(total * (1.0 - min_amount_percent_limit))));

    // Emit in the score order of the input: TopN / Group slice by position, so emitting in the
    // amount order would make them select by the amount instead of by the score
    std::vector<bool> keep(scores.size(), false);
    for (size_t i = 0; i < keep_count; ++i) {
        keep[amount_list[i].first] = true;
    }

    ret.reserve(keep_count);
    for (size_t i = 0; i < scores.size(); ++i) {
        if (keep[i]) {
            ret.emplace_back(scores[i]);
        }
    }

    return ret;
}

ScoresFilterPtr HKU_API SCFilter_AmountLimit(double min_amount_percent_limit) {
    auto p = std::make_shared<MinAmountPercentSCFilter>();
    p->setParam<double>("min_amount_percent_limit", min_amount_percent_limit);
    return p;
}

}  // namespace hku