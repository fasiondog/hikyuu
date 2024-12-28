/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "IndicatorIn2Imp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IndicatorIn2Imp)
#endif

namespace hku {

IndicatorIn2Imp::IndicatorIn2Imp() : IndicatorImp("IndicatorIn2Imp") {}

IndicatorIn2Imp::IndicatorIn2Imp(int n) : IndicatorImp("IndicatorIn2Imp") {}

IndicatorIn2Imp::IndicatorIn2Imp(const Indicator& ref_ind, int n)
: IndicatorImp("IndicatorIn2Imp"), m_ref_ind(ref_ind) {}

IndicatorIn2Imp::~IndicatorIn2Imp() {}

void IndicatorIn2Imp::_checkParam(const string& name) const {}

IndicatorImpPtr IndicatorIn2Imp::_clone() {
    auto p = make_shared<IndicatorIn2Imp>();
    p->m_ref_ind = m_ref_ind.clone();
    return p;
}

void IndicatorIn2Imp::_calculate(const Indicator& ind) {}

}  // namespace hku