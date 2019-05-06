/*
 * ISumBars.cpp
 * 
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-5
 *      Author: fasiondog
 */

#include "ISumBars.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISumBars)
#endif


namespace hku {

ISumBars::ISumBars() : IndicatorImp("SUMBARS", 1) {
    setParam<double>("a", 0);
}

ISumBars::~ISumBars() {

}

bool ISumBars::check() {
    return true;
}

void ISumBars::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    double a = getParam<double>("a");
    if (total == m_discard + 1) {
        if (ind[m_discard] >= a) {
            _set(0, m_discard);
        } else {
            m_discard = total;
        }
        return;
    }

    if (total == m_discard + 2) {
        double sum = ind[m_discard] + ind[m_discard+1];
        if (sum >= a) {
            _set(ind[m_discard+1] >= a ? 0 : 1, m_discard+1);
            if (ind[m_discard] >= a) {
                _set(0, m_discard);
            } else {
                m_discard += 1;
            }
        } else if (ind[m_discard] >= a) {
            _set(0, m_discard);
        } else {
            m_discard = total;
        }
        return;
    }

    size_t pos = total - 1;
    size_t last_pos = pos;
    double sum = ind[total-1];
    for (size_t i = total - 2; i >= m_discard; i--) {
        sum = sum - ind[i+1];
        if (sum < a) {
            if (pos >= 1) {
                for (size_t j = pos - 1; j >= m_discard; j--) {
                    sum += ind[j];
                    if (sum >= a) {
                        pos = j;
                        last_pos = j;
                        break;
                    }

                    if (j == m_discard) {
                        pos = Null<size_t>();
                        break;
                    }
                }
            }
        }

        if (pos == last_pos) {
            _set(i - pos, i);
        } else {
            break;
        }

        if (i == m_discard) {
            break;
        }
    }

    if (pos != last_pos) {
        m_discard = last_pos;
    }
}


Indicator HKU_API SUMBARS(double a) {
    IndicatorImpPtr p = make_shared<ISumBars>();
    p->setParam<double>("a", a);
    return Indicator(p);
}


} /* namespace hku */
