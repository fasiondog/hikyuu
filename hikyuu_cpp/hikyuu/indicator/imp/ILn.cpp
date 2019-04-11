/*
 * ILn.cpp
 *
 *  Created on: 2019年4月11日
 *      Author: fasiondog
 */

#include "ILn.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ILn)
#endif


namespace hku {

ILn::ILn() : IndicatorImp("LN", 1) {

}

ILn::~ILn() {

}

bool ILn::check() {
    return true;
}

void ILn::_calculate(const Indicator& data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    price_t null_price = Null<price_t>();
    for (size_t i = m_discard; i < total; ++i) {
        if (data[i] <= 0.0) {
            _set(null_price, i);
        } else {
            _set(std::log(data[i]), i);
        }
    }
}


Indicator HKU_API LN() {
    return Indicator(make_shared<ILn>());
}


} /* namespace hku */
