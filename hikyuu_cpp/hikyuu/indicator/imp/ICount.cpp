/*
 * Icount.cpp
 *
 *  Created on: 2019年3月25日
 *      Author: fasiondog
 */

#include "ICount.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICount)
#endif

namespace hku {

ICount::ICount() : IndicatorImp("COUNT", 1) {
    setParam<int>("n", 20);
}

ICount::~ICount() {}

bool ICount::check() {
    return getParam<int>("n") >= 0;
}

void ICount::_calculate(const Indicator& data) {
    size_t total = data.size();
    if (0 == total) {
        m_discard = 0;
        return;
    }

    if (data.discard() >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");

    if (0 == n) {
        m_discard = data.discard();
        int count = 0;
        for (size_t i = m_discard; i < total; ++i) {
            if (data[i] != 0) {
                count++;
            }
            _set(count, i);
        }
        return;
    }

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t startPos = data.discard();
    int sum = 0;
    size_t first_end = startPos + n >= total ? total : startPos + n;
    for (size_t i = startPos; i < first_end; ++i) {
        if (data[i] != 0) {
            sum++;
        }
    }

    if (first_end >= 1) {
        _set(sum, first_end - 1);
    }

    for (size_t i = first_end; i < total; ++i) {
        if (data[i] != 0) {
            sum++;
        }
        if (data[i - n] != 0) {
            sum--;
        }
        _set(sum, i);
    }
}

void ICount::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = 0;
    if (0 == step) {
        start = ind.discard();
    } else if (curPos < ind.discard() + step - 1) {
        return;
    } else {
        start = curPos + 1 - step;
    }
    price_t count = 0;
    for (size_t i = start; i <= curPos; i++) {
        if (ind[i] != 0.0) {
            count++;
        }
    }
    _set(count, curPos);
}

void ICount::_after_dyn_calculate(const Indicator& ind) {
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

Indicator HKU_API COUNT(int n) {
    IndicatorImpPtr p = make_shared<ICount>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API COUNT(const IndParam& n) {
    IndicatorImpPtr p = make_shared<ICount>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
