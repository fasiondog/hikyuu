/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#include "IFitR2.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IFitR2)
#endif

namespace hku {

IFitR2::IFitR2() : IndicatorImp("FITR2", 1) {
    setParam<int>("n", 22);
}

IFitR2::~IFitR2() {}

void IFitR2::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 2);
    }
}

void IFitR2::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    int n = getParam<int>("n");
    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    price_t xsum = 0.0, ysum = 0.0, xysum = 0.0, x2sum = 0.0, y2sum = 0.0;
    size_t first_end = m_discard + 1;
    for (size_t i = m_discard - n + 1; i < first_end; i++) {
        xsum += i;
        ysum += src[i];
        xysum += i * src[i];
        x2sum += std::pow(i, 2);
        y2sum += std::pow(src[i], 2);
    }

    price_t numerator = std::pow(n * xysum - xsum * ysum, 2);
    price_t denominator = (n * x2sum - xsum * xsum) * (n * y2sum - ysum * ysum);
    dst[m_discard] = denominator == 0 ? 0.0 : numerator / denominator;

    for (size_t i = first_end; i < total; i++) {
        size_t remove_pos = i - n;
        xsum += n;
        ysum += src[i] - src[remove_pos];
        xysum += src[i] * i - src[remove_pos] * remove_pos;
        x2sum += (2 * i - n) * n;
        y2sum += std::pow(src[i], 2) - std::pow(src[remove_pos], 2);

        numerator = std::pow(n * xysum - xsum * ysum, 2);
        denominator = (n * x2sum - xsum * xsum) * (n * y2sum - ysum * ysum);
        dst[i] = denominator == 0 ? 0.0 : numerator / denominator;
    }
}

bool IFitR2::supportIncrementCalculate() const {
    return getParam<int>("n") >= 2;
}

size_t IFitR2::min_increment_start() const {
    return getParam<int>("n");
}

void IFitR2::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    auto const* src = ind.data();
    auto* dst = this->data();

    int n = getParam<int>("n");

    price_t xsum = 0.0, ysum = 0.0, xysum = 0.0, x2sum = 0.0, y2sum = 0.0;
    for (size_t i = start_pos - n; i < start_pos; i++) {
        xsum += i;
        ysum += src[i];
        xysum += i * src[i];
        x2sum += std::pow(i, 2);
        y2sum += std::pow(src[i], 2);
    }

    for (size_t i = start_pos; i < total; i++) {
        size_t remove_pos = i - n;
        xsum += n;
        ysum += src[i] - src[remove_pos];
        xysum += src[i] * i - src[remove_pos] * remove_pos;
        x2sum += (2 * i - n) * n;
        y2sum += std::pow(src[i], 2) - std::pow(src[remove_pos], 2);

        price_t numerator = std::pow(n * xysum - xsum * ysum, 2);
        price_t denominator = (n * x2sum - xsum * xsum) * (n * y2sum - ysum * ysum);
        dst[i] = denominator == 0 ? 0.0 : numerator / denominator;
    }
}

Indicator HKU_API FITR2(int n) {
    IndicatorImpPtr p = make_shared<IFitR2>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku