/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-03
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/CVAL.h"
#include "hikyuu/indicator/crt/SLICE.h"
#include "hikyuu/indicator/crt/CONTEXT.h"
#include "Indicator2InImp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::Indicator2InImp)
#endif

namespace hku {

Indicator2InImp::Indicator2InImp() : IndicatorImp("Indicator2InImp") {
    setParam<bool>("fill_null", true);
}

Indicator2InImp::Indicator2InImp(const string& name, size_t result_num)
: IndicatorImp(name, result_num) {
    setParam<bool>("fill_null", true);
}

Indicator2InImp::Indicator2InImp(const string& name, const Indicator& ref_ind, bool fill_null,
                                 size_t result_num)
: IndicatorImp(name, result_num), m_ref_ind(ref_ind) {
    setParam<bool>("fill_null", fill_null);
}

Indicator2InImp::~Indicator2InImp() {}

IndicatorImpPtr Indicator2InImp::_clone() {
    auto p = make_shared<Indicator2InImp>();
    p->m_ref_ind = m_ref_ind.clone();
    return p;
}

Indicator Indicator2InImp::prepare(const Indicator& ind) {
    auto k = getContext();
    m_ref_ind.setContext(k);

    Indicator ref = m_ref_ind;
    auto dates = ref.getDatetimeList();
    if (dates.empty()) {
        // 如果不是时间序列，则以 ind 为基准，按右端对齐，不足用 nan 填充, 超长则截断左端
        if (ref.size() > ind.size()) {
            ref = SLICE(ref, ref.size() - ind.size(), ref.size());
        } else if (ref.size() < ind.size()) {
            ref = CVAL(ind, 0.) + ref;
        }
    } else if (k != ind.getContext()) {
        // 如果是时间序列，当两者的上下文不同，则按日期对齐
        ref = ALIGN(m_ref_ind, ind, getParam<bool>("fill_null"));
    }

    return ref;
}

}  // namespace hku