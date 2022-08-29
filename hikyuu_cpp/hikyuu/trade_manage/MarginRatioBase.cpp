/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-08-30
 *      Author: fasiondog
 */

#include "MarginRatioBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const MarginRatioBase& tc) {
    os << "MarginRatioFun(" << tc.name() << ", " << tc.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const MarginRatioPtr& tc) {
    if (tc) {
        os << *tc;
    } else {
        os << "MarginRatioFun(NULL)";
    }
    return os;
}

MarginRatioPtr MarginRatioBase::clone() {
    MarginRatioPtr result = _clone();
    MarginRatioBase* p = result.get();
    p->m_params = m_params;
    p->m_name = m_name;
    return result;
}

}  // namespace hku