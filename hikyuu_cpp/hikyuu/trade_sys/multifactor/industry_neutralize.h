/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  行业中性化的纯函数实现，提取为内部 inline header 以支持白盒单元测试。
 *  MultiFactorBase.cpp 与 test_MF_IndustryNeutralize.cpp 共同包含此头文件。
 */

#pragma once

#include <cmath>
#include <vector>
#include "hikyuu/DataType.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/Null.h"

namespace hku {

// 计算行业中性化后的因子：按行业分组去组内均值，残差 = y_i - mean(y_{group(i)})。
//
// 数学等价性：当行业归属互斥（一只股票只属一个行业）且仅对这一个分类变量做中性化时，
// "去全局截距的 one-hot 多元回归取残差"严格等价于"组内去均值"：
//   min_{β} Σ_i (y_i - Σ_k X_{i,k} β_k)^2  因 X 为互斥 one-hot，可拆成各行业独立求和，
//   解析解 β_k = mean(y_{group(k)})，残差 e_i = y_i - mean(y_{group(i)})。
// 故无需构造设计矩阵与 QR 分解，两趟扫描 O(n) 即可，且天然兼容每日动态 NaN 掩码
// （算组内均值时跳过无效值），避免原整数编码 + 单变量回归的模型误设（结果依赖
// 板块列表排列顺序）与多线程下 Eigen 矩阵堆分配争用。
//
// labels[i] = 股票 i 的行业下标（0..blk_count-1），无归属则为 blk_count（跳过，残差置 NaN）。
inline PriceList calculate_industry_residuals(const PriceList& y, const PriceList& labels,
                                              size_t blk_count) {
    HKU_ASSERT(y.size() == labels.size());

    const size_t n = y.size();
    PriceList residuals(n, Null<price_t>());

    // 全市场同行业（k=1）或无行业分类时，去均值退化为去全局均值；无有效数据则全 NaN。
    if (blk_count == 0) {
        return residuals;
    }

    std::vector<double> sums(blk_count, 0.0);
    std::vector<size_t> counts(blk_count, 0);

    // Pass 1: 累计各组有效 y 的和与计数（跳过 NaN/Inf 与无归属股票）
    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(y[i]) || std::isinf(y[i])) {
            continue;
        }
        double label = labels[i];
        // 守卫 NaN/Inf/负数/超大值：static_cast<size_t>(负数) 是 UB，
        // 可能溢出为 SIZE_MAX 导致 sums[g] 越界段错误，必须在 cast 前用 double 比较拦截。
        if (std::isnan(label) || std::isinf(label) || label < 0.0 ||
            label >= static_cast<double>(blk_count)) {
            continue;  // 无归属或非法标签不参与组内均值
        }
        size_t g = static_cast<size_t>(label);
        sums[g] += y[i];
        counts[g]++;
    }

    // Pass 2: 残差 = y_i - mean(y_{group(i)})
    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(y[i]) || std::isinf(y[i])) {
            continue;  // 保留 NaN（与原 calculate_residuals 语义一致）
        }
        double label = labels[i];
        if (std::isnan(label) || std::isinf(label) || label < 0.0 ||
            label >= static_cast<double>(blk_count)) {
            continue;  // 无归属或非法标签：残差置 NaN
        }
        size_t g = static_cast<size_t>(label);
        if (counts[g] == 0) {
            continue;  // 该组无有效样本：残差置 NaN
        }
        residuals[i] = y[i] - sums[g] / static_cast<double>(counts[g]);
    }

    return residuals;
}

}  // namespace hku
