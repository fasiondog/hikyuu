/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-02
 *      Author: fasiondog
 */

#include "IMddCurrent.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMddCurrent)
#endif

namespace hku {

IMddCurrent::IMddCurrent() : IndicatorImp("MDD_CURRENT", 1) {}

IMddCurrent::~IMddCurrent() {}

void IMddCurrent::_calculate(const Indicator& data) {
    m_discard = data.discard();
    size_t total = data.size();
    HKU_IF_RETURN(m_discard >= total, void());

    _increment_calculate(data, m_discard);
}

void IMddCurrent::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    auto const* src = ind.data();
    auto* dst = this->data();

    value_t run_max = std::numeric_limits<value_t>::lowest();
    for (size_t i = m_discard; i < start_pos; ++i) {
        if (src[i] > run_max) {
            run_max = src[i];
        }
    }

    for (size_t i = start_pos; i < total; ++i) {
        value_t current = src[i];
        if (std::isnan(current)) {
            dst[i] = 0.0;
            continue;
        }
        if (current > run_max) {
            run_max = current;
        }
        dst[i] = (run_max - current) / run_max * 100.0;
    }
}

Indicator HKU_API MDD_CURRENT() {
    return Indicator(make_shared<IMddCurrent>());
}

}  // namespace hku