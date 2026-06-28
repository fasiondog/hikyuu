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

IFitR2::IFitR2() : IndicatorImp("FITR2", 2) {
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
    auto* r2_dst = this->data();
    auto* slope_dst = this->data(1);

    price_t xsum = 0.0, ysum = 0.0, xysum = 0.0, x2sum = 0.0, y2sum = 0.0;
    size_t first_end = m_discard + 1;
    for (size_t i = m_discard - n + 1; i < first_end; i++) {
        xsum += i;
        ysum += src[i];
        xysum += i * src[i];
        x2sum += std::pow(i, 2);
        y2sum += std::pow(src[i], 2);
    }

    price_t xy_diff = n * xysum - xsum * ysum;
    price_t x_diff = n * x2sum - xsum * xsum;
    price_t y_diff = n * y2sum - ysum * ysum;
    price_t numerator_r2 = std::pow(xy_diff, 2);
    price_t denominator_r2 = x_diff * y_diff;
    r2_dst[m_discard] = denominator_r2 == 0 ? 0.0 : numerator_r2 / denominator_r2;
    slope_dst[m_discard] = x_diff == 0 ? 0.0 : xy_diff / x_diff;

    for (size_t i = first_end; i < total; i++) {
        size_t remove_pos = i - n;
        xsum += n;
        ysum += src[i] - src[remove_pos];
        xysum += src[i] * i - src[remove_pos] * remove_pos;
        x2sum += (2 * i - n) * n;
        y2sum += std::pow(src[i], 2) - std::pow(src[remove_pos], 2);

        xy_diff = n * xysum - xsum * ysum;
        x_diff = n * x2sum - xsum * xsum;
        y_diff = n * y2sum - ysum * ysum;
        numerator_r2 = std::pow(xy_diff, 2);
        denominator_r2 = x_diff * y_diff;
        r2_dst[i] = denominator_r2 == 0 ? 0.0 : numerator_r2 / denominator_r2;
        slope_dst[i] = x_diff == 0 ? 0.0 : xy_diff / x_diff;
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
    auto* r2_dst = this->data();
    auto* slope_dst = this->data(1);

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

        price_t xy_diff = n * xysum - xsum * ysum;
        price_t x_diff = n * x2sum - xsum * xsum;
        price_t y_diff = n * y2sum - ysum * ysum;
        price_t numerator_r2 = std::pow(xy_diff, 2);
        price_t denominator_r2 = x_diff * y_diff;
        r2_dst[i] = denominator_r2 == 0 ? 0.0 : numerator_r2 / denominator_r2;
        slope_dst[i] = x_diff == 0 ? 0.0 : xy_diff / x_diff;
    }
}

Indicator HKU_API FITR2(int n) {
    IndicatorImpPtr p = make_shared<IFitR2>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku