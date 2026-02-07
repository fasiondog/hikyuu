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

    // 如果没有数据或分组数为0，直接返回空列表
    int group = getParam<int>("group");
    if (scores.empty() || group == 0) {
        return ret;
    }

    // 计算每组应包含的股票数
    size_t total_count = scores.size();
    size_t stocks_per_group = (total_count + group - 1) / group;  // 向上取整

    // 检查索引是否有效
    int group_index = getParam<int>("group_index");
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