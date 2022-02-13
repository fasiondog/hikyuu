/*
 * IExist.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-19
 *      Author: fasiondog
 */

#include "IExist.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IExist)
#endif

namespace hku {

IExist::IExist() : IndicatorImp("EXIST", 1) {
    setParam<int>("n", 20);
}

IExist::~IExist() {}

bool IExist::check() {
    return getParam<int>("n") >= 0;
}

void IExist::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    if (n == 0) {
        n = total;
        m_discard = ind.discard();
        for (size_t i = m_discard; i < total; i++) {
            price_t exist = 0.0;
            for (size_t j = m_discard; j <= i; j++) {
                if (ind[j] != 0.0) {
                    exist = 1.0;
                    break;
                }
            }
            _set(exist, i);
        }
        return;
    }

    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    price_t exist = 0;
    size_t pre_pos = m_discard;
    for (size_t i = ind.discard(); i <= m_discard; i++) {
        if (ind[i] != 0) {
            pre_pos = i;
            exist = 1;
        }
    }

    _set(exist, m_discard);

    for (size_t i = m_discard + 1; i < total - 1; i++) {
        size_t j = i + 1 - n;
        if (pre_pos < j) {
            pre_pos = j;
            exist = ind[j] != 0 ? 1 : 0;
        }
        if (ind[i] != 0) {
            pre_pos = i;
            exist = 1;
        }
        _set(exist, i);
    }

    size_t start_pos = total - n;
    exist = 0;
    for (size_t i = start_pos; i < total; i++) {
        if (ind[i] != 0) {
            exist = 1;
            break;
        }
    }
    _set(exist, total - 1);
}

void IExist::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = 0;
    if (0 == step) {
        start = ind.discard();
    } else if (curPos < ind.discard() + step - 1) {
        return;
    } else {
        start = curPos + 1 - step;
    }

    price_t exist = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        if (ind[i] != 0.0) {
            exist = 1.0;
            break;
        }
    }
    _set(exist, curPos);
}

void IExist::_after_dyn_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(m_discard == total, void());

    size_t discard = m_discard;
    for (size_t i = total - 1; i > discard; i--) {
        if (std::isnan(get(i))) {
            m_discard = i + 1;
            break;
        }
    }
    if (m_discard == discard && std::isnan(get(discard))) {
        m_discard = discard + 1;
    }
}

Indicator HKU_API EXIST(int n) {
    IndicatorImpPtr p = make_shared<IExist>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API EXIST(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IExist>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
