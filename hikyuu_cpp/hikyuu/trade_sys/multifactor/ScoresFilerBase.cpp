/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#include "ScoresFilterBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& out, const ScoresFilterBase& scfilter) {
    out << "SCFilter{";
    out << scfilter.name() << "(params:" << scfilter.getParameter() << ")";
    auto child = scfilter.m_child;
    while (child) {
        out << " -> " << child->name() << "(params:" << child->getParameter() << ")";
        child = child->m_child;
    }
    out << "}";
    return out;
}

HKU_API std::ostream& operator<<(std::ostream& out, const ScoresFilterPtr& scfilter) {
    if (scfilter) {
        out << *scfilter;
    } else {
        out << "SCFilter(NULL)";
    }
    return out;
}

void ScoresFilterBase::baseCheckParam(const string& name) const {}

void ScoresFilterBase::paramChanged() {}

ScoresFilterPtr ScoresFilterBase::clone() {
    auto p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_is_python_object = m_is_python_object;

    if (m_child) {
        p->m_child = m_child->clone();
    }
    return p;
}

ScoreRecordList ScoresFilterBase::filter(const ScoreRecordList& scores, const Datetime& date,
                                         const KQuery& query) {
    auto ret = _filter(scores, date, query);
    if (m_child) {
        ret = m_child->filter(ret, date, query);
    }
    return ret;
}

HKU_API ScoresFilterPtr operator|(const ScoresFilterPtr& a, const ScoresFilterPtr& b) {
    ScoresFilterPtr ret;
    if (a && b) {
        auto node = a;
        while (node->m_child) {
            node = node->m_child;
        }
        node->m_child = b;
        ret = a;
    } else if (a) {
        HKU_WARN("filter b is null, will be ignored.");
        ret = a;
    } else if (b) {
        HKU_WARN("filter a is null, will be ignored.");
        ret = b;
    } else {
        HKU_WARN("filter a and b are all null, will be returned null.");
    }
    return ret;
}

}  // namespace hku