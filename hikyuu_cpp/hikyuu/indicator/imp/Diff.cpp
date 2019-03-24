/*
 * Diff.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "Diff.h"

namespace hku {

Diff::Diff(): IndicatorImp("DIFF", 1) {

}

Diff::~Diff() {

}

bool Diff::check() {
    return true;
}

void Diff::_calculate(const Indicator& data) {
    size_t total = data.size();

    m_discard = data.discard() + 1;
    if (total <= m_discard) {
        m_discard = total;
        return;
    }

    for (size_t i = discard(); i < total; ++i) {
        _set(data[i] - data[i-1], i);
    }
}


Indicator HKU_API DIFF() {
    return Indicator(make_shared<Diff>());
}

Indicator HKU_API DIFF(const Indicator& data) {
    return DIFF()(data);
}

} /* namespace hku */
