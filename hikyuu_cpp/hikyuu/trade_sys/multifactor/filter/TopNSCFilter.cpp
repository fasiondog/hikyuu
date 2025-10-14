/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-14
 *      Author: fasiondog
 */

#include "TopNSCFilter.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TopNSCFilter)
#endif

namespace hku {

TopNSCFilter::TopNSCFilter() : ScoresFilterBase("SCFilter_TopN") {
    setParam<int>("topn", 10);
}

void TopNSCFilter::_checkParam(const string& name) const {
    if (name == "topn") {
        HKU_ASSERT(getParam<int>("topn") > 0);
    }
}

ScoreRecordList TopNSCFilter::_filter(const ScoreRecordList& scores, const Datetime& date,
                                      const KQuery& query) {
    ScoreRecordList ret;
    int topn = getParam<int>("topn");
    ret.reserve(topn);
    for (size_t i = 0; i < topn && i < scores.size(); i++) {
        ret.push_back(scores[i]);
    }
    return ret;
}

ScoresFilterPtr HKU_API SCFilter_TopN(int topn) {
    auto p = std::make_shared<TopNSCFilter>();
    p->setParam<int>("topn", topn);
    return p;
}

}  // namespace hku