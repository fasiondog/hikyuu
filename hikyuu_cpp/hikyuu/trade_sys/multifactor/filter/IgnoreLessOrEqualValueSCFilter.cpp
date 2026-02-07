/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-14
 *      Author: fasiondog
 */

#include "IgnoreLessOrEqualValueSCFilter.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IgnoreLessOrEqualValueSCFilter)
#endif

namespace hku {

IgnoreLessOrEqualValueSCFilter::IgnoreLessOrEqualValueSCFilter()
: ScoresFilterBase("SCFilter_LessOrEqualValue") {
    setParam<double>("value", 0.0);
}

void IgnoreLessOrEqualValueSCFilter::_checkParam(const string& name) const {
    if (name == "value") {
        HKU_ASSERT(!std::isnan(getParam<double>("value")));
    }
}

ScoreRecordList IgnoreLessOrEqualValueSCFilter::_filter(const ScoreRecordList& scores,
                                                        const Datetime& date, const KQuery& query) {
    ScoreRecordList ret;
    ret.reserve(scores.size());

    double value = getParam<double>("value");
    for (auto& item : scores) {
        if (item.value > value) {
            ret.push_back(item);
        }
    }
    return ret;
}

ScoresFilterPtr HKU_API SCFilter_LessOrEqualValue(double value) {
    auto p = std::make_shared<IgnoreLessOrEqualValueSCFilter>();
    p->setParam<double>("value", value);
    return p;
}

}  // namespace hku