/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#include "PriceSCFilter.h"

namespace hku {

PriceSCFilter::PriceSCFilter() : ScoresFilterBase("SCFilter_Price") {
    setParam<double>("min_price", 5.0);       // 最低价格
    setParam<double>("max_price", 100000.0);  // 最高价格
}

void PriceSCFilter::_checkParam(const string& name) const {
    if (name == "min_price" || name == "max_price") {
        HKU_ASSERT(!std::isnan(getParam<double>(name)));
    }
}

ScoreRecordList PriceSCFilter::filter(const ScoreRecordList& scores, const Datetime& date,
                                      const KQuery& query) {
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

ScoresFilterPtr HKU_API SCFilter_Price(double min_price, double max_price) {
    HKU_CHECK(max_price > min_price, "max_price must > min_price!");
    auto p = std::make_shared<PriceSCFilter>();
    p->setParam<double>("min_price", min_price);
    p->setParam<double>("max_price", max_price);
    return p;
}

}  // namespace hku