/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "SubValueSignal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::SubValueSignal)
#endif

namespace hku {

SubValueSignal::SubValueSignal(double value, const SignalPtr& sg)
: OperatorValueSignal("SG_SubVlaue", sg, value, 1) {}

void SubValueSignal::_calculate(const KData& kdata) {
    HKU_IF_RETURN(!m_sg || std::isnan(m_value), void());

    auto const* ks = kdata.data();
    size_t total = kdata.size();

    m_sg->_calculate(kdata);
    if (m_value == 0.0) {
        if (m_mode == 0) {
            for (size_t i = 0; i < total; ++i) {
                _addSignal(ks[i].datetime, m_sg->getValue(ks[i].datetime));
            }
        } else {
            for (size_t i = 0; i < total; ++i) {
                _addSignal(ks[i].datetime, 0.0 - (m_sg->getValue(ks[i].datetime)));
            }
        }

    } else {
        if (m_mode == 0) {
            for (size_t i = 0; i < total; ++i) {
                double buy_value = m_sg->getBuyValue(ks[i].datetime);
                if (buy_value > 0.0) {
                    buy_value -= m_value;
                }
                double sell_value = m_sg->getSellValue(ks[i].datetime);
                if (sell_value < 0.0) {
                    sell_value -= m_value;
                }
                _addSignal(ks[i].datetime, buy_value + sell_value);
            }
        } else {
            for (size_t i = 0; i < total; ++i) {
                double buy_value = m_sg->getBuyValue(ks[i].datetime);
                if (buy_value > 0.0) {
                    buy_value = m_value - buy_value;
                }
                double sell_value = m_sg->getSellValue(ks[i].datetime);
                if (sell_value < 0.0) {
                    sell_value = m_value - sell_value;
                }
                _addSignal(ks[i].datetime, buy_value + sell_value);
            }
        }
    }
}

HKU_API SignalPtr operator-(const SignalPtr& sg, double value) {
    return make_shared<SubValueSignal>(sg, value);
}

HKU_API SignalPtr operator-(double value, const SignalPtr& sg) {
    return make_shared<SubValueSignal>(value, sg);
}

} /* namespace hku */
