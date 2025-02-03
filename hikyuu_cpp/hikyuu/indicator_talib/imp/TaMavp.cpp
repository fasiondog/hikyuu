/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "TaMavp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMavp)
#endif

namespace hku {

TaMavp::TaMavp() : Indicator2InImp("TA_MAVP", 1) {
    setParam<int>("min_n", 2);
    setParam<int>("max_n", 30);
    setParam<int>("matype", 0);
}

TaMavp::TaMavp(const Indicator& ref_ind, int min_n, int max_n, int matype, bool fill_null)
: Indicator2InImp("TA_MAVP", ref_ind, fill_null, 1) {
    setParam<int>("min_n", min_n);
    setParam<int>("max_n", max_n);
    setParam<int>("matype", matype);
}

TaMavp::~TaMavp() {}

void TaMavp::_checkParam(const string& name) const {
    if ("min_n" == name || "max_n" == name) {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 2 && n <= 100000, "{} must in [2, 100000]", name);
    } else if (name == "matype") {
        int matype = getParam<int>(name);
        HKU_ASSERT(matype >= 0 && matype <= 8);
    }
}

void TaMavp::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    Indicator ref = prepare(ind);

    int min_n = getParam<int>("min_n");
    int max_n = getParam<int>("max_n");
    TA_MAType matype = (TA_MAType)getParam<int>("matype");
    int lookback = TA_MAVP_Lookback(min_n, max_n, matype);
    if (lookback < 0) {
        m_discard = total;
        return;
    }

    m_discard = lookback + std::max(ind.discard(), ref.discard());
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    const auto* src0 = ind.data();
    const auto* src1 = ref.data();
    auto* dst = this->data();
    int outBegIdx;
    int outNbElement;
    TA_MAVP(m_discard, total - 1, src0, src1, min_n, max_n, matype, &outBegIdx, &outNbElement,
            dst + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_MAVP(const Indicator& ref_ind, int min_n, int max_n, int matype,
                          bool fill_null) {
    return Indicator(make_shared<TaMavp>(ref_ind, min_n, max_n, matype, fill_null));
}

}  // namespace hku