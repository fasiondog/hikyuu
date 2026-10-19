/*
 * IRocr100.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#include <cmath>
#include <climits>
#include "IRocr100.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRocr100)
#endif

namespace hku {

IRocr100::IRocr100() : IndicatorImp("ROCR100", 1) {
    setParam<int>("n", 10);
}

IRocr100::~IRocr100() {}

void IRocr100::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0 && getParam<int>("n") < INT_MAX);
    }
}

void IRocr100::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    int n = getParam<int>("n");

    m_discard = ind.discard() + n;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    if (0 == n) {
        size_t base_pos = m_discard;
        while (base_pos < total && (!std::isfinite(src[base_pos]) || src[base_pos] == 0.0)) {
            base_pos++;
        }
        if (base_pos < total) {
            price_t base_price = src[base_pos];
            dst[base_pos] = 100.0;
            for (size_t i = base_pos + 1; i < total; i++) {
                dst[i] = src[i] / base_price * 100.0;
            }
        }
        updateDiscard();
        return;
    }

    for (size_t i = m_discard; i < total; i++) {
        price_t pre_price = src[i - n];
        bool valid = std::isfinite(pre_price) && pre_price != 0.0;
        dst[i] = valid ? (src[i] / pre_price) * 100.0 : Null<price_t>();
    }
    // n alone cannot tell which referenced prices are unusable, scan the output to declare it
    updateDiscard();
}

bool IRocr100::supportIncrementCalculate() const {
    return getParam<int>("n") > 0;
}

void IRocr100::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    int n = getParam<int>("n");

    auto const* src = ind.data();
    auto* dst = this->data();

    for (size_t i = start_pos; i < total; i++) {
        price_t pre_price = src[i - n];
        bool valid = std::isfinite(pre_price) && pre_price != 0.0;
        dst[i] = valid ? (src[i] / pre_price) * 100.0 : Null<price_t>();
    }
}

void IRocr100::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    if (curPos < ind.discard() + step) {
        return;
    }
    size_t start = curPos - step;

    price_t pre_price = ind[start];
    bool valid = std::isfinite(pre_price) && pre_price != 0.0;
    _set(valid ? ind[curPos] / pre_price * 100.0 : Null<price_t>(), curPos);
}

Indicator HKU_API ROCR100(int n) {
    IndicatorImpPtr p = make_shared<IRocr100>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API ROCR100(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IRocr100>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
