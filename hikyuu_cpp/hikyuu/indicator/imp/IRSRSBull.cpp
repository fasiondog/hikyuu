/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#include "IRSRSBull.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRSRSBull)
#endif

namespace hku {

IRSRSBull::IRSRSBull() : IndicatorImp("RSRS_BULL", 4) {
    m_need_context = true;
    setParam<int>("n", 20);  // 回归窗口
    setParam<int>("m", 60);  // Z-score窗口
}

IRSRSBull::~IRSRSBull() {}

void IRSRSBull::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 2);
    } else if ("m" == name) {
        HKU_ASSERT(getParam<int>("m") >= 2);
    }
}

void IRSRSBull::_calculate(const Indicator&) {
    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    int m = getParam<int>("m");

    _readyBuffer(total, 4);

    size_t start_idx = n - 1;
    size_t z_start = start_idx + m - 1;

    if (total <= z_start) {
        m_discard = total;
        return;
    }

    auto* bull = this->data(0);  // 层级4修正值（右偏修正）
    auto* beta = this->data(1);  // β 值
    auto* r2 = this->data(2);    // R² 值
    auto* z = this->data(3);     // Z-score 值

    // 第一步：计算每个点的 β 和 R²（从 start_idx 开始）
    value_t sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0, sum_y2 = 0.0;
    for (size_t i = 0; i < static_cast<size_t>(n); i++) {
        value_t x = k[i].lowPrice;
        value_t y = k[i].highPrice;
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
        sum_y2 += y * y;
    }

    // 计算第 n-1 个点的 β 和 R²
    value_t denom_beta = n * sum_x2 - sum_x * sum_x;
    value_t denom_r2 = n * sum_y2 - sum_y * sum_y;

    if (denom_beta != 0.0 && denom_r2 != 0.0) {
        beta[start_idx] = (n * sum_xy - sum_x * sum_y) / denom_beta;
        value_t ssr = beta[start_idx] * beta[start_idx] * denom_beta / n;
        value_t sst = denom_r2 / n;
        r2[start_idx] = ssr / sst;
    }

    // 滚动计算后续点的 β 和 R²
    for (size_t i = n; i < total; i++) {
        value_t old_x = k[i - n].lowPrice;
        value_t old_y = k[i - n].highPrice;
        value_t new_x = k[i].lowPrice;
        value_t new_y = k[i].highPrice;

        sum_x += new_x - old_x;
        sum_y += new_y - old_y;
        sum_xy += new_x * new_y - old_x * old_y;
        sum_x2 += new_x * new_x - old_x * old_x;
        sum_y2 += new_y * new_y - old_y * old_y;

        denom_beta = n * sum_x2 - sum_x * sum_x;
        denom_r2 = n * sum_y2 - sum_y * sum_y;

        if (denom_beta != 0.0 && denom_r2 != 0.0) {
            beta[i] = (n * sum_xy - sum_x * sum_y) / denom_beta;
            value_t ssr = beta[i] * beta[i] * denom_beta / n;
            value_t sst = denom_r2 / n;
            r2[i] = ssr / sst;
        }
    }

    // 第二步：计算滚动 M 日的 Z-score
    // 计算前 M 个 β 的均值和标准差（从 start_idx 到 z_start）
    value_t sum_beta = 0.0, sum_beta2 = 0.0;
    int valid_count = 0;
    for (size_t i = start_idx; i <= z_start; i++) {
        if (!std::isnan(beta[i])) {
            sum_beta += beta[i];
            sum_beta2 += beta[i] * beta[i];
            valid_count++;
        }
    }

    if (valid_count >= 2) {
        value_t mean_beta = sum_beta / valid_count;
        value_t var_beta = (sum_beta2 - sum_beta * sum_beta / valid_count) / (valid_count - 1);
        value_t std_beta = std::sqrt(var_beta);

        if (std_beta > 0.0 && !std::isnan(beta[z_start])) {
            z[z_start] = (beta[z_start] - mean_beta) / std_beta;
            bull[z_start] = z[z_start] * r2[z_start] * beta[z_start];
        }

        // 滚动计算后续的 Z-score 和层级4修正值
        for (size_t i = z_start + 1; i < total; i++) {
            // 移除最旧的 β
            size_t oldest_idx = i - m;
            if (oldest_idx >= start_idx && !std::isnan(beta[oldest_idx])) {
                sum_beta -= beta[oldest_idx];
                sum_beta2 -= beta[oldest_idx] * beta[oldest_idx];
                valid_count--;
            }

            // 添加新的 β
            if (!std::isnan(beta[i])) {
                sum_beta += beta[i];
                sum_beta2 += beta[i] * beta[i];
                valid_count++;
            }

            if (valid_count >= 2) {
                mean_beta = sum_beta / valid_count;
                var_beta = (sum_beta2 - sum_beta * sum_beta / valid_count) / (valid_count - 1);
                std_beta = std::sqrt(var_beta);

                if (std_beta > 0.0 && !std::isnan(beta[i])) {
                    z[i] = (beta[i] - mean_beta) / std_beta;
                    bull[i] = z[i] * r2[i] * beta[i];
                }
            }
        }
    }

    // 设置 discard，并将 beta 和 r2 前面的位置置为 Null（参考 ADX 处理方式）
    m_discard = z_start;
    for (size_t i = 0; i < z_start; i++) {
        beta[i] = Null<value_t>();
        r2[i] = Null<value_t>();
    }
}

Indicator HKU_API RSRS_BULL(int n, int m) {
    auto p = make_shared<IRSRSBull>();
    p->setParam<int>("n", n);
    p->setParam<int>("m", m);
    return Indicator(p);
}

Indicator HKU_API RSRS_BULL(const KData& kdata, int n, int m) {
    auto p = make_shared<IRSRSBull>();
    p->setParam<int>("n", n);
    p->setParam<int>("m", m);
    p->setContext(kdata);
    return Indicator(p);
}

}  // namespace hku