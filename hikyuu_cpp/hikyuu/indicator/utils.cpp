/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-28
 *      Author: fasiondog
 */

#include <Eigen/Dense>
#include "hikyuu/utilities/thread/algorithm.h"
#include "utils.h"

namespace hku {

/**
 * 多元线性回归实现
 *
 * @param stk 股票对象
 * @param query K线查询条件
 * @param inds 指标列表（自变量）
 * @return 回归系数向量 [alpha, beta1, beta2, ...]
 */
std::vector<double> multi_regression(const Stock& stk, const KQuery& query,
                                     const IndicatorList& inds) {
    KData kdata = stk.getKData(query);
    HKU_IF_RETURN(kdata.empty(), {});

    size_t factor_count = inds.size();
    HKU_IF_RETURN(factor_count == 0, {});

    IndicatorList all_inds(factor_count);
    global_parallel_for_index_void(0, factor_count, [&](size_t i) {
        all_inds[i] = ALIGN(inds[i], kdata, false)(kdata).getResult(0);
    });

    Indicator returns = ROCP(kdata.close(), 1);
    size_t n = returns.size();
    HKU_IF_RETURN(n <= factor_count, {});

    std::vector<size_t> valid_indices;
    valid_indices.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(returns[i]))
            continue;
        bool valid = true;
        for (const auto& ind : all_inds) {
            if (std::isnan(ind[i])) {
                valid = false;
                break;
            }
        }
        if (valid) {
            valid_indices.push_back(i);
        }
    }

    size_t valid_count = valid_indices.size();
    HKU_IF_RETURN(valid_count <= factor_count, {});

    Eigen::MatrixXd X(valid_count, factor_count + 1);
    Eigen::VectorXd Y(valid_count);

    X.col(0).setOnes();
    for (size_t j = 0; j < factor_count; ++j) {
        for (size_t k = 0; k < valid_count; ++k) {
            X(k, j + 1) = all_inds[j][valid_indices[k]];
        }
    }
    for (size_t k = 0; k < valid_count; ++k) {
        Y(k) = returns[valid_indices[k]];
    }

    Eigen::VectorXd beta = X.colPivHouseholderQr().solve(Y);
    HKU_IF_RETURN(beta.hasNaN(), {});

    std::vector<double> result(beta.size());
    for (size_t i = 0; i < beta.size(); ++i) {
        result[i] = beta(i);
    }

    return result;
}

/**
 * 多元线性回归完整版本实现
 *
 * @param stk 股票对象
 * @param query K线查询条件
 * @param inds 指标列表（自变量）
 * @return 回归结果向量 [alpha, beta1, ..., betan, e1, ..., en, RSS, R²]
 */
std::vector<double> multi_regression_full(const Stock& stk, const KQuery& query,
                                          const IndicatorList& inds) {
    KData kdata = stk.getKData(query);
    HKU_IF_RETURN(kdata.empty(), {});

    size_t factor_count = inds.size();
    HKU_IF_RETURN(factor_count == 0, {});

    IndicatorList all_inds;
    all_inds.reserve(factor_count);
    for (auto& ind : inds) {
        all_inds.emplace_back(ALIGN(ind, kdata, false)(kdata).getResult(0));
    }

    Indicator returns = ROCP(kdata.close(), 1);
    size_t n = returns.size();
    HKU_IF_RETURN(n <= factor_count, {});

    std::vector<size_t> valid_indices;
    valid_indices.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(returns[i]))
            continue;
        bool valid = true;
        for (const auto& ind : all_inds) {
            if (std::isnan(ind[i])) {
                valid = false;
                break;
            }
        }
        if (valid) {
            valid_indices.push_back(i);
        }
    }

    size_t valid_count = valid_indices.size();
    HKU_IF_RETURN(valid_count <= factor_count, {});

    Eigen::MatrixXd X(valid_count, factor_count + 1);
    Eigen::VectorXd Y(valid_count);

    X.col(0).setOnes();
    for (size_t j = 0; j < factor_count; ++j) {
        for (size_t k = 0; k < valid_count; ++k) {
            X(k, j + 1) = all_inds[j][valid_indices[k]];
        }
    }
    for (size_t k = 0; k < valid_count; ++k) {
        Y(k) = returns[valid_indices[k]];
    }

    Eigen::VectorXd beta = X.colPivHouseholderQr().solve(Y);
    HKU_IF_RETURN(beta.hasNaN(), {});

    Eigen::VectorXd Y_hat = X * beta;
    Eigen::VectorXd residuals = Y - Y_hat;

    double RSS = residuals.squaredNorm();
    double Y_mean = Y.mean();
    double TSS = (Y.array() - Y_mean).square().sum();
    double R_squared = TSS > 1e-10 ? (1.0 - RSS / TSS) : 0.0;

    size_t result_size = beta.size() + valid_count + 2;
    std::vector<double> result(result_size);

    for (size_t i = 0; i < beta.size(); ++i) {
        result[i] = beta(i);
    }

    for (size_t i = 0; i < valid_count; ++i) {
        result[beta.size() + i] = residuals(i);
    }

    result[beta.size() + valid_count] = RSS;
    result[beta.size() + valid_count + 1] = R_squared;

    return result;
}

}  // namespace hku