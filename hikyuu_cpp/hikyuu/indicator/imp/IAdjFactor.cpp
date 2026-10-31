/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-17
 *      Author: fasiondog
 */

#include "IAdjFactor.h"
#include <vector>

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAdjFactor)
#endif

namespace hku {

IAdjFactor::IAdjFactor() : IndicatorImp("ADJ_FACTOR", 1) {
    m_need_context = true;
}

IAdjFactor::~IAdjFactor() {}

void IAdjFactor::_calculate(const Indicator& ind) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
    m_discard = 0;

    // Reuse the incremental calculation method; start_pos = 0 means calculating from the beginning
    _increment_calculate(ind, 0);
}

bool IAdjFactor::supportIncrementCalculate() const {
    return true;
}

void IAdjFactor::_increment_calculate(const Indicator& ind, size_t start_pos) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0 || start_pos >= total, void());

    auto* dst = data();
    auto* kdata = k.data();

    // The multipliers of the effective ex-rights/ex-dividend records are cached per stock and
    // anchored at the fixed baseline (the data start), so a date always gets the same factor
    // whichever range is queried
    vector<EqualRecoverFactor> factors = k.getStock().getEqualRecoverFactors();

    // 1. The base factor before start_pos: the factor of the previous bar in the incremental case,
    //    or the product of the ex-rights before the first bar of the range
    price_t base_factor = 1.0;
    size_t f_idx = 0;
    if (start_pos > 0) {
        base_factor = dst[start_pos - 1];
        const Datetime prev_day = kdata[start_pos - 1].datetime.startOfDay();
        while (f_idx < factors.size() && factors[f_idx].date <= prev_day) {
            f_idx++;
        }
    } else {
        const Datetime start_day = kdata[0].datetime.startOfDay();
        while (f_idx < factors.size() && factors[f_idx].date < start_day) {
            base_factor *= factors[f_idx].price_k;
            f_idx++;
        }
    }

    for (size_t i = start_pos; i < total; ++i) {
        dst[i] = base_factor;
    }

    // 2. Apply the factors inside the range from the ex-rights/ex-dividend day on
    size_t pos = start_pos;
    for (; f_idx < factors.size(); ++f_idx) {
        while (pos < total && kdata[pos].datetime < factors[f_idx].date) {
            pos++;
        }
        if (pos >= total) {
            break;
        }
        for (size_t i = pos; i < total; ++i) {
            dst[i] *= factors[f_idx].price_k;
        }
    }
}

Indicator HKU_API ADJ_FACTOR() {
    return Indicator(make_shared<IAdjFactor>());
}

Indicator HKU_API ADJ_FACTOR(const KData& k) {
    auto p = make_shared<IAdjFactor>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hku */
