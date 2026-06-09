/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-06-09
 *      Author: fasiondog
 */

#include "ISignedPower.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISignedPower)
#endif

namespace hku {

ISignedPower::ISignedPower() : IndicatorImp("SIGNED_POWER", 1) {
    setParam<int>("n", 3);
}

ISignedPower::~ISignedPower() {}

void ISignedPower::_checkParam(const string& name) const {}

void ISignedPower::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(data, m_discard);
}

void ISignedPower::_increment_calculate(const Indicator& data, size_t start_pos) {
    int n = getParam<int>("n");
    auto const* src = data.data();
    auto* dst = this->data();
    for (size_t i = start_pos; i < data.size(); ++i) {
        dst[i] = (std::signbit(src[i]) ? -1.0 : 1.0) * std::pow(std::abs(src[i]), n);
    }
}

void ISignedPower::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    _set((std::signbit(ind[curPos]) ? -1.0 : 1.0) * std::pow(std::abs(ind[curPos]), step), curPos);
}

Indicator HKU_API SIGNED_POWER(int n) {
    IndicatorImpPtr p = make_shared<ISignedPower>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API SIGNED_POWER(const IndParam& n) {
    IndicatorImpPtr p = make_shared<ISignedPower>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
