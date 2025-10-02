/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "IQuantileNormUniform.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IQuantileNormUniform)
#endif

namespace hku {

IQuantileNormUniform::IQuantileNormUniform() : IndicatorImp("QUANTILE_NORM_UNIFORM", 1) {
    setParam<double>("quantile_min", 0.01);
    setParam<double>("quantile_max", 0.99);
}

IQuantileNormUniform::IQuantileNormUniform(double quantile_min, double quantile_max)
: IndicatorImp("QUANTILE_NORM_UNIFORM", 1) {
    setParam<double>("quantile_min", quantile_min);
    setParam<double>("quantile_max", quantile_max);
}

IQuantileNormUniform::~IQuantileNormUniform() {}

void IQuantileNormUniform::_checkParam(const string &name) const {
    if ("quantile_min" == name) {
        double quantile_min = getParam<double>("quantile_min");
        HKU_ASSERT(quantile_min > 0.0 && quantile_min < 1.0);
        if (haveParam("quantile_max")) {
            double quantile_max = getParam<double>("quantile_max");
            HKU_ASSERT(quantile_min < quantile_max);
        }
    } else if ("quantile_max" == name) {
        double quantile_max = getParam<double>("quantile_max");
        HKU_ASSERT(quantile_max > 0.0 && quantile_max < 1.0);
        if (haveParam("quantile_min")) {
            double quantile_min = getParam<double>("quantile_min");
            HKU_ASSERT(quantile_min < quantile_max);
        }
    }
}

// 判断 double 是否为整数（考虑精度误差）
static bool isInteger(double num) {
    // 处理特殊值：NaN（非数值）或无穷大
    if (std::isnan(num) || std::isinf(num)) {
        return false;
    }

    // 计算小数部分（对 1.0 取模）
    double fractionalPart = num - std::floor(num);  // 等价于 num % 1.0，但处理负数更稳妥

    // 允许微小误差（因浮点数精度问题，如 5.0000000001 应视为 5）
    const double epsilon = 1e-9;
    return std::fabs(fractionalPart) < epsilon || std::fabs(fractionalPart - 1.0) < epsilon;
}

// 计算已排序vector中第 quantile 位的数值
static Indicator::value_t get_quantile(const std::vector<IndicatorImp::value_t> &vec,
                                       double quantile) {
    HKU_ASSERT(!vec.empty());
    if (quantile == 0.0) {
        return vec.front();
    } else if (quantile == 1.0) {
        return vec.back();
    }

    Indicator::value_t ret;
    double qpos = vec.size() * quantile;
    if (qpos <= 1.0) {
        ret = vec[0];
    } else if (isInteger(qpos)) {
        size_t pos = static_cast<size_t>(qpos);
        ret = (vec[pos - 1] + vec[pos]) / 2;
    } else {
        size_t pos = static_cast<size_t>(qpos);
        Indicator::value_t x = qpos - pos;
        ret = vec[pos - 1] + x * (vec[pos] - vec[pos - 1]);
    }
    return ret;
}

// 替换掉分位数范围外数值
static vector<Indicator::value_t> quantile_trunc(Indicator::value_t const *src, size_t total,
                                                 double quantile_min, double quantile_max) {
    vector<Indicator::value_t> ret(total);
    if (quantile_min == 0.0 && quantile_max == 1.0) {
        std::copy(src, src + total, ret.begin());
        return ret;
    }

    std::vector<IndicatorImp::value_t> tmp;
    tmp.reserve(total);
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(src[i])) {
            tmp.push_back(src[i]);
        }
    }
    std::sort(tmp.begin(), tmp.end());
    if (tmp.empty()) {
        for (size_t i = 0; i < total; i++) {
            ret[i] = Null<Indicator::value_t>();
        }
        return ret;
    }

    auto down_limit = get_quantile(tmp, quantile_min);
    auto up_limit = get_quantile(tmp, quantile_max);
    HKU_INFO("quantile_min: {}, quantile_max: {}", down_limit, up_limit);
    for (size_t i = 0; i < total; i++) {
        if (src[i] > up_limit) {
            ret[i] = up_limit;
        } else if (src[i] < down_limit) {
            ret[i] = down_limit;
        } else {
            ret[i] = src[i];
        }
    }

    return ret;
}

static void quantile_normalize(IndicatorImp::value_t *dst, Indicator::value_t const *src,
                               size_t total, double quantile_min, double quantile_max) {
    auto data = quantile_trunc(src, total, quantile_min, quantile_max);
    HKU_IF_RETURN(data.empty(), void());

    std::vector<std::pair<double, int64_t>> valueIndices;
    valueIndices.reserve(total);
    for (size_t i = 0; i < total; ++i) {
        if (!std::isnan(data[i])) {
            valueIndices.emplace_back(data[i], int64_t(i));
        }
    }

    // 按值排序（升序）
    std::sort(valueIndices.begin(), valueIndices.end());

    // 计算平均排名（处理重复值）
    int64_t i = 0;
    int64_t n = static_cast<int64_t>(valueIndices.size());
    while (i < n) {
        double currentValue = valueIndices[i].first;
        size_t start = i;

        // 找到所有相同值的范围
        while (i < n && valueIndices[i].first == currentValue) {
            ++i;
        }
        int end = i - 1;  // 相同值的最后一个索引

        // 计算平均排名：(start+1 + end+1) / 2 （排名从1开始）
        double avgRank = (start + 1 + end + 1) / 2.0;

        // 计算分位数百分比：(平均排名 - 0.5) / n
        double quantile = (avgRank - 0.5) / n;

        // 为所有相同值的原始位置赋值
        for (int64_t j = start; j <= end; ++j) {
            int originalIndex = valueIndices[j].second;
            dst[originalIndex] = quantile;
        }
    }
}

void IQuantileNormUniform::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard + 1 >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data() + m_discard;
    auto *dst = this->data() + m_discard;
    quantile_normalize(dst, src, total - m_discard, getParam<double>("quantile_min"),
                       getParam<double>("quantile_max"));

    _update_discard();
}

Indicator HKU_API QUANTILE_NORM_UNIFORM(double quantile_min, double quantile_max) {
    return Indicator(make_shared<IQuantileNormUniform>(quantile_min, quantile_max));
}

}  // namespace hku