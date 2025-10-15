/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include <boost/math/distributions/normal.hpp>
#include "quantile_trunc.h"
#include "NormQuantile.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NormQuantile)
#endif

namespace hku {

NormQuantile::NormQuantile() : NormalizeBase("NORM_Quantile") {
    setParam<double>("quantile_min", 0.01);
    setParam<double>("quantile_max", 0.99);
}

NormQuantile::NormQuantile(double quantile_min, double quantile_max)
: NormalizeBase("NORM_Quantile") {
    setParam<double>("quantile_min", quantile_min);
    setParam<double>("quantile_max", quantile_max);
}

NormQuantile::~NormQuantile() {}

void NormQuantile::_checkParam(const string &name) const {
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

PriceList NormQuantile::normalize(const PriceList &src) {
    PriceList result(src.size());
    HKU_IF_RETURN(src.empty(), result);

    auto data =
      quantile_trunc(src, getParam<double>("quantile_min"), getParam<double>("quantile_max"));

    size_t total = data.size();
    std::vector<std::pair<double, int64_t>> valueIndices;
    valueIndices.reserve(total);
    for (size_t i = 0; i < total; ++i) {
        if (!std::isnan(data[i])) {
            valueIndices.emplace_back(data[i], int64_t(i));
        }
    }

    // 按值排序（升序）
    std::sort(valueIndices.begin(), valueIndices.end());

    boost::math::normal_distribution<> stdNormal(0.0, 1.0);

    // 计算平均排名（处理重复值）
    int64_t i = 0;
    int64_t n = static_cast<int64_t>(valueIndices.size());
    if (n >= 50) {
        // 大样本使用 (k - 0.5) / n
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

            double z = boost::math::quantile(stdNormal, quantile);

            // 为所有相同值的原始位置赋值
            for (int64_t j = start; j <= end; ++j) {
                int originalIndex = valueIndices[j].second;
                result[originalIndex] = z;
            }
        }
    } else {
        // 小样本使用 (k) / (n+1)
        double denominator = n + 1.0;
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

            // 计算分位数百分比：(平均排名) / (n+1)
            double quantile = avgRank / denominator;

            double z = boost::math::quantile(stdNormal, quantile);

            // 为所有相同值的原始位置赋值
            for (int64_t j = start; j <= end; ++j) {
                int originalIndex = valueIndices[j].second;
                result[originalIndex] = z;
            }
        }
    }
    return result;
}

NormPtr HKU_API NORM_Quantile(double quantile_min, double quantile_max) {
    return std::make_shared<NormQuantile>(quantile_min, quantile_max);
}

}  // namespace hku