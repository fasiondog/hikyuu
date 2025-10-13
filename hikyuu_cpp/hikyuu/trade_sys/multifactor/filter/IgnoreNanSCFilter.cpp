/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-14
 *      Author: fasiondog
 */

#include "IgnoreNanSCFilter.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IgnoreNanSCFilter)
#endif

namespace hku {

IgnoreNanSCFilter::IgnoreNanSCFilter() : ScoresFilterBase("SCFilter_IgnoreNan") {}

void IgnoreNanSCFilter::_checkParam(const string& name) const {}

ScoreRecordList IgnoreNanSCFilter::_filter(const ScoreRecordList& scores, const Datetime& date,
                                           const KQuery& query) {
    ScoreRecordList ret;
    ret.reserve(scores.size());
    for (auto& item : scores) {
        if (!std::isnan(item.value)) {
            ret.push_back(item);
        }
    }
    return ret;
}

ScoresFilterPtr HKU_API SCFilter_IgnoreNan() {
    return std::make_shared<IgnoreNanSCFilter>();
}

}  // namespace hku