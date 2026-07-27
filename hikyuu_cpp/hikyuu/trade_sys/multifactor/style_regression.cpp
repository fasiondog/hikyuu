/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  风格因子中性化残差回归，从 MultiFactorBase 中提取的串行内核。
 *
 *  设计要点：
 *    - 保留 Eigen ColPivHouseholderQR 求解，其本身不在 Eigen 并行算法集合内；
 *    - 拟合值改用显式逐行点积，避免触发 general matrix-matrix 乘法的并行路径；
 *    - 不调用进程级 Eigen::setNbThreads，避免并发 MF 互相污染全局配置；
 *    - 保留原 calculate_residuals 的有效样本/秩不足/NaN 语义。
 */

#include <cmath>
#include <Eigen/Dense>
#include "style_regression.h"
#include "hikyuu/utilities/Null.h"

namespace hku {

PriceList calculate_style_residuals(const PriceList& y, const vector<PriceList>& x) {
    HKU_ASSERT(!x.empty());
    size_t n = y.size();
    for (const auto& xi : x) {
        HKU_ASSERT(xi.size() == n);
    }

    PriceList residuals(n, Null<price_t>());
    size_t k = x.size();  // 解释变量个数

    // 构建设计矩阵和因变量向量
    Eigen::MatrixXd Xmat(n, k + 1);
    Eigen::VectorXd Yvec(n);

    // 第一列为常数项（全1）
    Xmat.col(0).setConstant(1.0);

    // 标记有效数据点
    std::vector<bool> valid(n, true);

    for (size_t i = 0; i < n; ++i) {
        Yvec(i) = y[i];

        // 检查因变量是否有效
        if (std::isnan(y[i]) || std::isinf(y[i])) {
            valid[i] = false;
            continue;
        }

        // 填充自变量并检查有效性
        for (size_t j = 0; j < k; ++j) {
            Xmat(i, j + 1) = x[j][i];
            if (std::isnan(x[j][i]) || std::isinf(x[j][i])) {
                valid[i] = false;
                break;
            }
        }
    }

    // 计算有效数据点数量
    size_t valid_count = std::count(valid.begin(), valid.end(), true);

    // 数据点不足
    if (valid_count <= k + 1) {
        return residuals;
    }

    // 创建有效数据的子矩阵
    Eigen::MatrixXd X_valid(valid_count, k + 1);
    Eigen::VectorXd Y_valid(valid_count);

    size_t valid_idx = 0;
    for (size_t i = 0; i < n; ++i) {
        if (valid[i]) {
            X_valid.row(valid_idx) = Xmat.row(i);
            Y_valid(valid_idx) = Yvec(i);
            valid_idx++;
        }
    }

    // 使用 QR 分解求解线性回归 β = (X'X)^(-1)X'Y
    Eigen::VectorXd beta = X_valid.colPivHouseholderQr().solve(Y_valid);

    // 检查解是否有效
    if (beta.hasNaN()) {
        return residuals;
    }

    // 计算残差：逐行点积拟合值，避免 general matrix-matrix 并行路径
    for (size_t i = 0; i < n; ++i) {
        if (!valid[i]) {
            continue;
        }
        double fitted = beta(0);
        for (size_t j = 0; j < k; ++j) {
            fitted += beta(j + 1) * x[j][i];
        }
        residuals[i] = y[i] - fitted;
    }

    return residuals;
}

}  // namespace hku