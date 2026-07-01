/*
 * IMa.cpp
 *
 *  Created on: 2013-2-10
 *      Author: fasiondog
 */

#include "IMa.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMa)
#endif

namespace hku {

IMa::IMa() : IndicatorImp("MA", 1) {
    setParam<int>("n", 22);
}

IMa::~IMa() {}

void IMa::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void IMa::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    auto const* src = indicator.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    if (n <= 0) {
        m_discard = indicator.discard();
        if (m_discard >= total) {
            m_discard = total;
            return;
        }

        price_t sum = 0.0;
        for (size_t i = m_discard; i < total; i++) {
            if (!std::isnan(src[i])) {
                sum += src[i];
                dst[i] = sum / (i - m_discard + 1);
            }
        }
        return;
    }

    m_discard = indicator.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    if (n == 1) {
        memcpy(dst + m_discard, src + m_discard, (total - m_discard) * sizeof(value_t));
        return;
    }

    // Welford 滚动均值：状态 (valid_count, mean)，O(1) 出入队。
    // 与 IStdev 共用相同的 valid_count 增减逻辑，保证 MA/STDEV 基于完全一致的样本集。
    size_t startPos = indicator.discard();
    size_t valid_count = 0;
    price_t mean = 0.0;
    for (size_t i = startPos; i < total; ++i) {
        // 移除 leaving（窗口已满时）
        if (i >= static_cast<size_t>(startPos) + static_cast<size_t>(n)) {
            price_t leaving = src[i - n];
            if (!std::isnan(leaving)) {
                if (valid_count > 1) {
                    price_t delta = leaving - mean;
                    mean -= delta / (valid_count - 1);
                } else {
                    // valid_count == 1，移除后窗口归零
                    mean = 0.0;
                }
                valid_count--;
            }
        }
        // 加入 entering
        price_t entering = src[i];
        if (!std::isnan(entering)) {
            valid_count++;
            if (valid_count == 1) {
                mean = entering;
            } else {
                price_t delta = entering - mean;
                mean += delta / valid_count;
            }
        }
        // 窗口未满或无有效值时不写输出（缓冲区已是 NaN）
        if (i >= m_discard && valid_count > 0) {
            dst[i] = mean;
        }
    }
}

bool IMa::supportIncrementCalculate() const {
    int n = getParam<int>("n");
    return n > 1;
}

size_t IMa::min_increment_start() const {
    int n = getParam<int>("n");
    return n;
}

void IMa::_increment_calculate(const Indicator& indicator, size_t startPos) {
    size_t total = indicator.size();
    auto const* src = indicator.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    HKU_ASSERT(startPos + 1 >= (size_t)n);
    // 从窗口起点 startPos+1-n 单趟重建 Welford 均值状态，再滚动至 total
    size_t start = startPos + 1 - n;
    size_t valid_count = 0;
    price_t mean = 0.0;
    for (size_t i = start; i < total; ++i) {
        if (i > startPos) {
            price_t leaving = src[i - n];
            if (!std::isnan(leaving)) {
                if (valid_count > 1) {
                    price_t delta = leaving - mean;
                    mean -= delta / (valid_count - 1);
                } else {
                    mean = 0.0;
                }
                valid_count--;
            }
        }
        price_t entering = src[i];
        if (!std::isnan(entering)) {
            valid_count++;
            if (valid_count == 1) {
                mean = entering;
            } else {
                price_t delta = entering - mean;
                mean += delta / valid_count;
            }
        }
        if (i >= startPos && valid_count > 0) {
            dst[i] = mean;
        }
    }
}

void IMa::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    if (curPos + 1 < ind.discard() + step) {
        return;
    }
    size_t start = _get_step_start(curPos, step, ind.discard());
    // 单趟 Welford 均值（动态窗口左边界跳变，不用 remove）
    size_t valid_count = 0;
    price_t mean = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        if (!std::isnan(ind[i])) {
            valid_count++;
            if (valid_count == 1) {
                mean = ind[i];
            } else {
                price_t delta = ind[i] - mean;
                mean += delta / valid_count;
            }
        }
    }
    if (valid_count > 0) {
        _set(mean, curPos);
    }
}

Indicator HKU_API MA(int n) {
    IndicatorImpPtr p = make_shared<IMa>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API MA(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IMa>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
