/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#include "GroupSCFilter.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::GroupSCFilter)
#endif

namespace hku {

GroupSCFilter::GroupSCFilter() : ScoresFilterBase("SCFilter_Group") {
    setParam<int>("group", 10);
    setParam<int>("group_index", 0);
}

void GroupSCFilter::_checkParam(const string& name) const {
    if (name == "group") {
        HKU_ASSERT(getParam<int>("group") > 0);
    } else if (name == "group_index") {
        int group_index = getParam<int>(name);
        HKU_ASSERT(group_index >= 0 && group_index < getParam<int>("group"));
    }
}

ScoreRecordList GroupSCFilter::_filter(const ScoreRecordList& scores, const Datetime& date,
                                       const KQuery& query) {
    ScoreRecordList ret;

    // Return an empty list directly when there is no data or the group count is 0
    int group = getParam<int>("group");
    if (scores.empty() || group == 0) {
        return ret;
    }

    // Calculate the number of the stocks every group should contain
    size_t total_count = scores.size();
    size_t stocks_per_group = (total_count + group - 1) / group;  // Round up

    // Check whether the index is valid
    int group_index = getParam<int>("group_index");
    if (group_index >= group) {
        return ret;
    }

    // Calculate the start and the end position of the current group
    size_t start = group_index * stocks_per_group;
    size_t end = std::min(start + stocks_per_group, total_count);

    // Return an empty list when the start position is out of range
    if (start >= total_count) {
        return ret;
    }

    // Extract the data of the current group from the original data
    ret.reserve(end - start);
    for (size_t i = start; i < end; i++) {
        ret.emplace_back(scores[i]);
    }

    return ret;
}

ScoresFilterPtr HKU_API SCFilter_Group(int group, int group_index) {
    HKU_CHECK(group > 0, "group must > 0!");
    HKU_CHECK(group_index >= 0 && group_index < group, "group_index must >= 0 and < group!");
    auto p = std::make_shared<GroupSCFilter>();
    p->setParam<int>("group", group);
    p->setParam<int>("group_index", group_index);
    return p;
}

}  // namespace hku