/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/CVAL.h"
#include "hikyuu/indicator/crt/SLICE.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/imp/IContext.h"
#include "hikyuu/indicator/crt/CONTEXT.h"
#include "TaMavp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMavp)
#endif

namespace hku {

TaMavp::TaMavp() : IndicatorImp("TA_MAVP", 1) {
    setParam<int>("min_n", 2);
    setParam<int>("max_n", 30);
    setParam<int>("matype", 0);
    setParam<bool>("fill_null", true);
}

TaMavp::TaMavp(const Indicator& ref_ind, int min_n, int max_n, int matype, bool fill_null)
: IndicatorImp("TA_MAVP", 1), m_ref_ind(ref_ind) {
    setParam<int>("min_n", min_n);
    setParam<int>("max_n", max_n);
    setParam<int>("matype", matype);
    setParam<bool>("fill_null", fill_null);
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

IndicatorImpPtr TaMavp::_clone() {
    auto p = make_shared<TaMavp>();
    p->m_ref_ind = m_ref_ind.clone();
    return p;
}

void TaMavp::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 2);

    auto k = getContext();
    m_ref_ind.setContext(k);
    Indicator ref = m_ref_ind;
    auto dates = ref.getDatetimeList();
    if (dates.empty()) {
        if (ref.size() > ind.size()) {
            ref = SLICE(ref, ref.size() - ind.size(), ref.size());
        } else if (ref.size() < ind.size()) {
            ref = CVAL(ind, 0.) + ref;
        }
    } else if (m_ref_ind.size() != ind.size()) {
        ref = ALIGN(m_ref_ind, ind, getParam<bool>("fill_null"));
    }

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