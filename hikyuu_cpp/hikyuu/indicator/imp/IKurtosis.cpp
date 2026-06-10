/*
 * IKurtosis.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "IKurtosis.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IKurtosis)
#endif

namespace hku {

IKurtosis::IKurtosis() : IndicatorImp("KURT", 1) {
    setParam<int>("n", 10);
}

IKurtosis::~IKurtosis() {}

void IKurtosis::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>(name);
        HKU_ASSERT(n >= 4 || n == 0);
    }
}

void IKurtosis::_calculate(const Indicator& data) {
    size_t total = data.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    if (n == 0) {
        n = total;
    }

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = data.data();
    auto* dst = this->data();

    vector<price_t> pow2_buf(data.size());
    vector<price_t> pow3_buf(data.size());
    vector<price_t> pow4_buf(data.size());

    size_t start_pos = data.discard();
    size_t first_end = start_pos + n >= total ? total : start_pos + n;
    value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0, ex4 = 0.0;

    value_t k = src[start_pos];
    for (size_t i = start_pos; i < first_end; i++) {
        value_t d = src[i] - k;
        ex += d;
        value_t d2 = d * d;
        pow2_buf[i] = d2;
        ex2 += d2;
        value_t d3 = d2 * d;
        pow3_buf[i] = d3;
        ex3 += d3;
        value_t d4 = d3 * d;
        pow4_buf[i] = d4;
        ex4 += d4;
    }

    value_t var = ex2 / n - ex * ex / (n * n);
    value_t std_dev = sqrt(var);
    value_t std_dev_pow4 = var * var;

    if (std_dev == 0) {
        dst[first_end - 1] = -3.0;
    } else {
        value_t ex_over_n = ex / n;
        value_t ex2_over_n = ex2 / n;
        value_t fourth_moment = ex4 / n - 4 * (ex3 / n) * ex_over_n +
                                6 * ex2_over_n * ex_over_n * ex_over_n -
                                3 * ex_over_n * ex_over_n * ex_over_n * ex_over_n;
        value_t kurtosis = fourth_moment / std_dev_pow4;
        dst[first_end - 1] = kurtosis - 3.0;
    }

    for (size_t i = first_end, pre_ix = first_end - n; i < total; i++, pre_ix++) {
        ex -= src[pre_ix] - k;
        ex2 -= pow2_buf[pre_ix];
        ex3 -= pow3_buf[pre_ix];
        ex4 -= pow4_buf[pre_ix];

        value_t d = src[i] - k;
        ex += d;
        value_t d2 = d * d;
        pow2_buf[i] = d2;
        ex2 += d2;
        value_t d3 = d2 * d;
        pow3_buf[i] = d3;
        ex3 += d3;
        value_t d4 = d3 * d;
        pow4_buf[i] = d4;
        ex4 += d4;

        var = ex2 / n - ex * ex / (n * n);
        std_dev = sqrt(var);
        std_dev_pow4 = var * var;

        if (std_dev == 0) {
            dst[i] = -3.0;
        } else {
            value_t ex_over_n = ex / n;
            value_t ex2_over_n = ex2 / n;
            value_t fourth_moment = ex4 / n - 4 * (ex3 / n) * ex_over_n +
                                    6 * ex2_over_n * ex_over_n * ex_over_n -
                                    3 * ex_over_n * ex_over_n * ex_over_n * ex_over_n;
            value_t kurtosis = fourth_moment / std_dev_pow4;
            dst[i] = kurtosis - 3.0;
        }
    }
}

bool IKurtosis::supportIncrementCalculate() const {
    return getParam<int>("n") != 0;
}

size_t IKurtosis::min_increment_start() const {
    return getParam<int>("n");
}

void IKurtosis::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    int n = getParam<int>("n");
    auto const* src = data.data();
    auto* dst = this->data();

    vector<price_t> pow2_buf(data.size());
    vector<price_t> pow3_buf(data.size());
    vector<price_t> pow4_buf(data.size());

    value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0, ex4 = 0.0;
    value_t k = src[start_pos];
    for (size_t i = start_pos - n; i < start_pos; i++) {
        value_t d = src[i] - k;
        ex += d;
        value_t d2 = d * d;
        pow2_buf[i] = d2;
        ex2 += d2;
        value_t d3 = d2 * d;
        pow3_buf[i] = d3;
        ex3 += d3;
        value_t d4 = d3 * d;
        pow4_buf[i] = d4;
        ex4 += d4;
    }

    for (size_t i = start_pos, pre_ix = start_pos - n; i < total; i++, pre_ix++) {
        ex -= src[pre_ix] - k;
        ex2 -= pow2_buf[pre_ix];
        ex3 -= pow3_buf[pre_ix];
        ex4 -= pow4_buf[pre_ix];

        value_t d = src[i] - k;
        ex += d;
        value_t d2 = d * d;
        pow2_buf[i] = d2;
        ex2 += d2;
        value_t d3 = d2 * d;
        pow3_buf[i] = d3;
        ex3 += d3;
        value_t d4 = d3 * d;
        pow4_buf[i] = d4;
        ex4 += d4;

        value_t var = ex2 / n - ex * ex / (n * n);
        value_t std_dev = sqrt(var);
        value_t std_dev_pow4 = var * var;

        if (std_dev == 0) {
            dst[i] = -3.0;
        } else {
            value_t ex_over_n = ex / n;
            value_t ex2_over_n = ex2 / n;
            value_t fourth_moment = ex4 / n - 4 * (ex3 / n) * ex_over_n +
                                    6 * ex2_over_n * ex_over_n * ex_over_n -
                                    3 * ex_over_n * ex_over_n * ex_over_n * ex_over_n;
            value_t kurtosis = fourth_moment / std_dev_pow4;
            dst[i] = kurtosis - 3.0;
        }
    }
}

void IKurtosis::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    HKU_IF_RETURN(step < 4, void());

    size_t start = _get_step_start(curPos, step, ind.discard());
    HKU_IF_RETURN(start != curPos + 1 - step, void());

    value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0, ex4 = 0.0;
    value_t k = ind[start];
    for (size_t i = start; i <= curPos; i++) {
        value_t d = ind[i] - k;
        ex += d;
        value_t d2 = d * d;
        ex2 += d2;
        ex3 += d2 * d;
        ex4 += d2 * d2;
    }

    value_t var = ex2 / step - ex * ex / (step * step);
    value_t std_dev = sqrt(var);
    value_t std_dev_pow4 = var * var;

    if (std_dev == 0) {
        _set(-3.0, curPos);
    } else {
        value_t ex_over_step = ex / step;
        value_t ex2_over_step = ex2 / step;
        value_t fourth_moment = ex4 / step - 4 * (ex3 / step) * ex_over_step +
                                6 * ex2_over_step * ex_over_step * ex_over_step -
                                3 * ex_over_step * ex_over_step * ex_over_step * ex_over_step;
        value_t kurtosis = fourth_moment / std_dev_pow4;
        _set(kurtosis - 3.0, curPos);
    }
}

Indicator HKU_API KURT(int n) {
    IndicatorImpPtr p = make_shared<IKurtosis>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API KURT(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IKurtosis>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */