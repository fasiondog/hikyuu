/*
 * IExp.cpp
 *
 *  Created on: 2019年4月3日
 *      Author: fasiondog
 */

#include "IExp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IExp)
#endif


namespace hku {

IExp::IExp() : IndicatorImp("EXP", 1) {

}

IExp::~IExp() {

}

bool IExp::check() {
    return true;
}

void IExp::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    price_t null_price = Null<price_t>();
    for (size_t i = m_discard; i < total; ++i) {
        _set(std::exp(data[i]), i);
    }
}


Indicator HKU_API EXP() {
    return Indicator(make_shared<IExp>());
}


} /* namespace hku */
