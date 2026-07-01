/*
 * IStd.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "IStdev.h"
#include "../crt/MA.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IStdev)
#endif

namespace hku {

IStdev::IStdev() : IndicatorImp("STDEV", 1) {
    setParam<int>("n", 10);
}

IStdev::~IStdev() {}

void IStdev::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>("n");
        HKU_ASSERT(n == 0 || n >= 2);
    }
}

void IStdev::_calculate(const Indicator& data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    auto const* src = data.data();
    auto* dst = this->data();

    // n == 0：全量累计标准差（expand-all 语义，每个位置输出到当前位置的累计 std）
    if (0 == n) {
        m_discard = data.discard();
        if (m_discard >= total) {
            m_discard = total;
            return;
        }
        size_t valid_count = 0;
        price_t mean = 0.0;
        price_t M2 = 0.0;
        for (size_t i = m_discard; i < total; ++i) {
            if (!std::isnan(src[i])) {
                valid_count++;
                if (valid_count == 1) {
                    mean = src[i];
                    M2 = 0.0;
                } else {
                    price_t delta = src[i] - mean;
                    mean += delta / valid_count;
                    M2 += delta * (src[i] - mean);
                }
            }
            if (valid_count > 1) {
                dst[i] = std::sqrt(std::max(0.0, M2 / (valid_count - 1)));
            }
        }
        return;
    }

    // n > 0：滚动窗口 Welford 方差，状态 (valid_count, mean, M2)，O(1) 出入队。
    // 与 IMa 共用相同的 valid_count/mean 更新逻辑，保证 MA/STDEV 基于一致样本集。
    // 离群值离开窗口时 M2 可能因灾难性相消变为负值或丢失低位精度，
    // 此时触发 O(k) 单趟重算（k=窗口长度 n）重建精确状态。
    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t startPos = data.discard();
    size_t valid_count = 0;
    price_t mean = 0.0;
    price_t M2 = 0.0;
    for (size_t i = startPos; i < total; ++i) {
        // 移除 leaving
        if (i >= static_cast<size_t>(startPos) + static_cast<size_t>(n)) {
            price_t leaving = src[i - n];
            if (!std::isnan(leaving)) {
                if (valid_count > 1) {
                    price_t old_M2 = M2;
                    price_t delta = leaving - mean;
                    mean -= delta / (valid_count - 1);
                    M2 -= delta * (leaving - mean);
                    valid_count--;
                    // 灾难性相消检测：M2 变负或较 remove 前缩减 10^9 倍（float64 有效位坍塌）时重算
                    if (M2 < 0.0 || M2 < 1e-9 * old_M2) {
                        // O(n) 单趟 Welford 重算 [i-n+1, i-1]（已移除 leaving，未加入 entering）
                        size_t vc_r = 0;
                        price_t mean_r = 0.0, M2_r = 0.0;
                        size_t lo = i - static_cast<size_t>(n) + 1;
                        for (size_t j = lo; j < i; ++j) {
                            if (!std::isnan(src[j])) {
                                vc_r++;
                                if (vc_r == 1) {
                                    mean_r = src[j];
                                } else {
                                    price_t d = src[j] - mean_r;
                                    mean_r += d / vc_r;
                                    M2_r += d * (src[j] - mean_r);
                                }
                            }
                        }
                        valid_count = vc_r;
                        mean = mean_r;
                        M2 = M2_r;
                    }
                } else {
                    // valid_count == 1，移除后窗口归零
                    mean = 0.0;
                    M2 = 0.0;
                    valid_count = 0;
                }
            }
        }
        // 加入 entering
        price_t entering = src[i];
        if (!std::isnan(entering)) {
            valid_count++;
            if (valid_count == 1) {
                mean = entering;
                M2 = 0.0;
            } else {
                price_t delta = entering - mean;
                mean += delta / valid_count;
                M2 += delta * (entering - mean);
            }
        }
        // 窗口未满或有效值不足时不写输出（缓冲区已是 NaN）
        if (i >= m_discard && valid_count > 1) {
            dst[i] = std::sqrt(std::max(0.0, M2 / (valid_count - 1)));
        }
    }
}

bool IStdev::supportIncrementCalculate() const {
    return getParam<int>("n") != 0;
}

size_t IStdev::min_increment_start() const {
    return getParam<int>("n");
}

void IStdev::_increment_calculate(const Indicator& data, size_t start_pos) {
    size_t total = data.size();
    int n = getParam<int>("n");
    auto const* src = data.data();
    auto* dst = this->data();
    HKU_ASSERT(start_pos + 1 >= (size_t)n);

    // 从窗口起点 start_pos+1-n 单趟重建 Welford 状态，再滚动至 total
    size_t start = start_pos + 1 - n;
    size_t valid_count = 0;
    price_t mean = 0.0;
    price_t M2 = 0.0;
    for (size_t i = start; i < total; ++i) {
        if (i > start_pos) {
            price_t leaving = src[i - n];
            if (!std::isnan(leaving)) {
                if (valid_count > 1) {
                    price_t old_M2 = M2;
                    price_t delta = leaving - mean;
                    mean -= delta / (valid_count - 1);
                    M2 -= delta * (leaving - mean);
                    valid_count--;
                    if (M2 < 0.0 || M2 < 1e-9 * old_M2) {
                        size_t vc_r = 0;
                        price_t mean_r = 0.0, M2_r = 0.0;
                        size_t lo = i - static_cast<size_t>(n) + 1;
                        for (size_t j = lo; j < i; ++j) {
                            if (!std::isnan(src[j])) {
                                vc_r++;
                                if (vc_r == 1) {
                                    mean_r = src[j];
                                } else {
                                    price_t d = src[j] - mean_r;
                                    mean_r += d / vc_r;
                                    M2_r += d * (src[j] - mean_r);
                                }
                            }
                        }
                        valid_count = vc_r;
                        mean = mean_r;
                        M2 = M2_r;
                    }
                } else {
                    mean = 0.0;
                    M2 = 0.0;
                    valid_count = 0;
                }
            }
        }
        price_t entering = src[i];
        if (!std::isnan(entering)) {
            valid_count++;
            if (valid_count == 1) {
                mean = entering;
                M2 = 0.0;
            } else {
                price_t delta = entering - mean;
                mean += delta / valid_count;
                M2 += delta * (entering - mean);
            }
        }
        if (i >= start_pos && valid_count > 1) {
            dst[i] = std::sqrt(std::max(0.0, M2 / (valid_count - 1)));
        }
    }
}

void IStdev::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    if (curPos + 1 < ind.discard() + step) {
        return;
    }
    size_t start = _get_step_start(curPos, step, ind.discard());
    // 单趟 Welford（动态窗口左边界跳变，不用 remove，无重算需求）
    size_t valid_count = 0;
    price_t mean = 0.0;
    price_t M2 = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        if (!std::isnan(ind[i])) {
            valid_count++;
            if (valid_count == 1) {
                mean = ind[i];
                M2 = 0.0;
            } else {
                price_t delta = ind[i] - mean;
                mean += delta / valid_count;
                M2 += delta * (ind[i] - mean);
            }
        }
    }
    if (valid_count > 1) {
        _set(std::sqrt(std::max(0.0, M2 / (valid_count - 1))), curPos);
    }
}

Indicator HKU_API STDEV(int n) {
    IndicatorImpPtr p = make_shared<IStdev>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API STDEV(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IStdev>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
