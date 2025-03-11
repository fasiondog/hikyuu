/*
 * IBarsSince.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#include "IBarsSince.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IBarsSince)
#endif

namespace hku {

IBarsSince::IBarsSince() : IndicatorImp("BARSSINCE", 1) {
    setParam<int>("n", 0);
}

IBarsSince::~IBarsSince() {}

void IBarsSince::_checkParam(const string &name) const {
    if (name == "n") {
        HKU_CHECK(getParam<int>("n") >= 0, "n must >= 0!");
    }
}

void IBarsSince::_calculate(const Indicator &ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const *src = ind.data();
    auto *dst = this->data();

    int n = getParam<int>("n");
    if (0 == n) {
        bool found = false;
        size_t pos = m_discard;
        for (size_t i = m_discard; i < total; ++i) {
            if (found) {
                dst[i] = i - pos;
            } else {
                if (!std::isnan(src[i]) && src[i] != 0.0) {
                    found = true;
                    pos = i;
                    dst[i] = 0.0;
                }
            }
        }

        m_discard = pos;
        return;
    }

    if (1 == n) {
        for (size_t i = m_discard; i < total; ++i) {
            if (!std::isnan(src[i]) && src[i] != 0.0) {
                dst[i] = 0.0;
            } else {
                dst[i] = Null<value_t>();
            }
        }
        _update_discard();
        return;
    }

    size_t first = m_discard + n - 1;
    for (size_t i = first; i < total; ++i) {
        size_t pos = 0;
        bool found = false;
        for (size_t j = i + 1 - n; j < i; j++) {
            if (!std::isnan(src[j]) && src[j] != 0.0) {
                pos = i - j;
                found = true;
                break;
            }
        }
        dst[i] = found ? pos : Null<value_t>();
    }
    _update_discard();
}

Indicator HKU_API BARSSINCE() {
    return Indicator(make_shared<IBarsSince>());
}

Indicator HKU_API BARSSINCEN(int n) {
    auto p = make_shared<IBarsSince>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

} /* namespace hku */
