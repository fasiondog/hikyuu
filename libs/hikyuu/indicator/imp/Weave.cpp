/*
 * Weave.cpp
 *
 *  Created on: 2017年4月19日
 *      Author: Administrator
 */

#include <hikyuu/indicator/imp/Weave.h>

namespace hku {

Weave::Weave(): IndicatorImp("WEAVE") {

}

Weave::~Weave() {

}

bool Weave::check() {
    return true;
}

void Weave::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (m_result_num != 0 && total != size()) {
        HKU_ERROR("ind's size must be equal weave's size! [Weave::_calculate]");
        return;
    }

    if (total == 0) {
        return;
    }

    size_t old_m_result_num = m_result_num;
    m_result_num = ind.getResultNumber() + m_result_num;
    if (m_result_num > MAX_RESULT_NUM) {
        HKU_WARN("Weave only can contains " << MAX_RESULT_NUM <<
                 "reult_num! [Weave::_calculate]");
        m_result_num = MAX_RESULT_NUM;
    }

    if (m_discard < ind.discard()) {
        m_discard = ind.discard();
    }

    price_t null_price = Null<price_t>();
    for (size_t i = old_m_result_num; i < m_result_num; ++i) {
        m_pBuffer[i] = new PriceList(total, null_price);
        for (size_t j = m_discard; j < total; ++j) {
            (*m_pBuffer[i])[j] = ind.get(j, i-old_m_result_num);
        }
    }
}

IndicatorImpPtr Weave::operator()(const Indicator& ind) {
    Weave* p = new Weave();
    p->m_discard = m_discard;
    p->m_result_num = m_result_num;

    size_t self_size = size();
    if (m_result_num != 0) {
        p->_readyBuffer(self_size, m_result_num);
        if (self_size != 0) {
            for (size_t i = 0; i < m_result_num; ++i) {
                for (size_t j = m_discard; j < self_size; ++j) {
                    p->_set(get(j,i), j, i);
                }
            }
        }
    }

    p->_calculate(ind);
    return IndicatorImpPtr(p);
}

Indicator HKU_API WEAVE() {
    IndicatorImpPtr p = make_shared<Weave>();
    return Indicator(p);
}

Indicator HKU_API WEAVE(const Indicator& ind) {
    IndicatorImpPtr p = make_shared<Weave>();
    p->calculate(ind);
    return Indicator(p);
}

} /* namespace hku */
