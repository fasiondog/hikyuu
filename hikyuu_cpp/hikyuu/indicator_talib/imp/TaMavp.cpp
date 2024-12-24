/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "ta_func.h"
#include "hikyuu/indicator/crt/ALIGN.h"
#include "TaMavp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMavp)
#endif

namespace hku {

TaMavp::TaMavp() : IndicatorImp("TA_MAVP") {
    setParam<int>("min_n", 2);
    setParam<int>("max_n", 30);
    setParam<int>("matype", 0);
}

TaMavp::TaMavp(int min_n, int max_n, int matype) : IndicatorImp("TA_MAVP") {
    setParam<int>("min_n", min_n);
    setParam<int>("max_n", max_n);
    setParam<int>("matype", matype);
}

TaMavp::TaMavp(const Indicator& ref_ind, int min_n, int max_n, int matype)
: IndicatorImp("TA_MAVP"), m_ref_ind(ref_ind) {
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

IndicatorImpPtr TaMavp::_clone() {
    auto p = make_shared<TaMavp>();
    p->m_ref_ind = m_ref_ind.clone();
    return p;
}

void TaMavp::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    auto k = getContext();
    m_ref_ind.setContext(k);
    Indicator ref = m_ref_ind;
    if (m_ref_ind.size() != ind.size()) {
        ref = ALIGN(m_ref_ind, ind);
    }

    _readyBuffer(total, 1);

    int min_n = getParam<int>("min_n");
    int max_n = getParam<int>("max_n");
    TA_MAType matype = (TA_MAType)getParam<int>("matype");
    int lookback = TA_MAVP_Lookback(min_n, max_n, matype);
    HKU_IF_RETURN(lookback < 0, void());

    m_discard = lookback + std::max(ind.discard(), ref.discard());

    const auto* src0 = ind.data();
    const auto* src1 = ref.data();
    auto* dst = this->data();
    int outBegIdx;
    int outNbElement;
    TA_MAVP(m_discard, total, src0, src1, min_n, max_n, matype, &outBegIdx, &outNbElement,
            dst + m_discard);
    if (outBegIdx > m_discard) {
        memmove(dst + outBegIdx, dst + m_discard, outNbElement * sizeof(double));
        double null_double = Null<double>();
        for (size_t i = m_discard; i < outBegIdx; ++i) {
            _set(null_double, i);
        }
        m_discard = outBegIdx;
    }
}

Indicator HKU_API TA_MAVP(int min_n, int max_n, int matype) {
    return Indicator(make_shared<TaMavp>(min_n, max_n, matype));
}

Indicator HKU_API TA_MAVP(const Indicator& ind1, const Indicator& ind2, int min_n, int max_n,
                          int matype) {
    auto p = make_shared<TaMavp>(ind2, min_n, max_n, matype);
    Indicator result(p);
    return result(ind1);
}

}  // namespace hku