/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#include "ScoresFilterBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& out, const ScoresFilterBase& scfilter) {
    out << "SCFilter{" << "\n  name: " << scfilter.name()
        << "\n  params: " << scfilter.getParameter();
    out << "\n}";
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
    return p;
}

}  // namespace hku