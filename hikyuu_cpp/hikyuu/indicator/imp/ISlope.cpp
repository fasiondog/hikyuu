/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#include "ISlope.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISlope)
#endif

namespace hku {

ISlope::ISlope() : IndicatorImp("SLOPE", 2) {
    setParam<int>("n", 22);
}

ISlope::~ISlope() {}

void ISlope::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void ISlope::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    m_discard = ind.discard() + 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst_slope = this->data(0);
    auto* dst_r2 = this->data(1);

    int n = getParam<int>("n");
    if (n <= 1) {
        for (size_t i = m_discard; i < total; i++) {
            dst_slope[i] = 0.0;
            dst_r2[i] = 0.0;
        }
        return;
    }

    size_t startPos = m_discard - 1;
    price_t xsum = 0.0, ysum = 0.0, xysum = 0.0, x2sum = 0.0, y2sum = 0.0;
    size_t first_end = startPos + n >= total ? total : startPos + n;
    for (size_t i = startPos; i < first_end; i++) {
        price_t x = i;
        price_t y = src[i];
        xsum += x;
        ysum += y;
        xysum += x * y;
        x2sum += x * x;
        y2sum += y * y;
        size_t cnt = i + 1;
        price_t denominator = cnt * x2sum - xsum * xsum;
        dst_slope[i] = (cnt * xysum - xsum * ysum) / denominator;
        price_t numerator = std::pow(cnt * xysum - xsum * ysum, 2);
        price_t denominator_r2 = denominator * (cnt * y2sum - ysum * ysum);
        dst_r2[i] = numerator / denominator_r2;
    }

    for (size_t i = first_end; i < total; i++) {
        xsum += n;
        ysum += src[i] - src[i - n];
        xysum += src[i] * i - src[i - n] * (i - n);
        x2sum += (2 * i - n) * n;
        y2sum += src[i] * src[i] - src[i - n] * src[i - n];
        price_t denominator = n * x2sum - xsum * xsum;
        dst_slope[i] = (n * xysum - xsum * ysum) / denominator;
        price_t numerator = std::pow(n * xysum - xsum * ysum, 2);
        price_t denominator_r2 = denominator * (n * y2sum - ysum * ysum);
        dst_r2[i] = numerator / denominator_r2;
    }
}

bool ISlope::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t ISlope::min_increment_start() const {
    return getParam<int>("n");
}

void ISlope::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    auto const* src = ind.data();
    auto* dst_slope = this->data(0);
    auto* dst_r2 = this->data(1);

    int n = getParam<int>("n");

    price_t xsum = 0.0, ysum = 0.0, xysum = 0.0, x2sum = 0.0, y2sum = 0.0;
    for (size_t i = start_pos - n; i < start_pos; i++) {
        price_t x = i;
        price_t y = src[i];
        xsum += x;
        ysum += y;
        xysum += x * y;
        x2sum += x * x;
        y2sum += y * y;
    }

    for (size_t i = start_pos; i < total; i++) {
        xsum += n;
        ysum += src[i] - src[i - n];
        xysum += src[i] * i - src[i - n] * (i - n);
        x2sum += (2 * i - n) * n;
        y2sum += src[i] * src[i] - src[i - n] * src[i - n];
        price_t denominator = n * x2sum - xsum * xsum;
        dst_slope[i] = (n * xysum - xsum * ysum) / denominator;
        price_t numerator = std::pow(n * xysum - xsum * ysum, 2);
        price_t denominator_r2 = denominator * (n * y2sum - ysum * ysum);
        dst_r2[i] = numerator / denominator_r2;
    }
}

void ISlope::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos <= ind.discard()) {
        _set(Null<price_t>(), curPos);
        _set(Null<price_t>(), curPos, 1);
        return;
    }

    if (step <= 1) {
        _set(0, curPos);
        _set(0, curPos, 1);
        return;
    }

    double n = curPos - start + 1;
    price_t xsum = 0.0, ysum = 0.0, xysum = 0.0, x2sum = 0.0, y2sum = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        price_t x = i;
        price_t y = ind[i];
        xsum += x;
        ysum += y;
        xysum += x * y;
        x2sum += x * x;
        y2sum += y * y;
    }

    price_t denominator = n * x2sum - xsum * xsum;
    price_t slope = (n * xysum - xsum * ysum) / denominator;
    price_t numerator = std::pow(n * xysum - xsum * ysum, 2);
    price_t denominator_r2 = denominator * (n * y2sum - ysum * ysum);
    price_t r2 = numerator / denominator_r2;
    _set(slope, curPos);
    _set(r2, curPos, 1);
}

Indicator HKU_API SLOPE(int n) {
    IndicatorImpPtr p = make_shared<ISlope>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API SLOPE(const IndParam& n) {
    IndicatorImpPtr p = make_shared<ISlope>();
    p->setIndParam("n", n);
    return Indicator(p);
}

}  // namespace hku