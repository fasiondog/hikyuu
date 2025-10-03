/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "NormalizeBase.h"

namespace hku {

void NormalizeBase::paramChanged() {}

void NormalizeBase::baseCheckParam(const string& name) const {}

NormalizePtr NormalizeBase::clone() {
    NormalizePtr p;
    p = _clone();
    HKU_ERROR_IF(!p, "Failed clone! {}", m_name);

    p->m_name = m_name;
    p->m_params = m_params;
    return p;
}

}  // namespace hku