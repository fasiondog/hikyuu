/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-28
 *      Author: fasiondog
 */

#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <vector>
#include "hikyuu/utilities/thread/algorithm.h"
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/ROCP.h"
#include "utils.h"

namespace hku {

namespace {

// Assemble the design matrix (a leading ones column plus one column per factor) and the target
// return vector, dropping every time point where the return or any factor is non-finite. Returns
// false when the sample is empty, has no factor, or has no more valid rows than coefficients.
bool build_regression_dataset(const Stock& stk, const KQuery& query, const IndicatorList& inds,
                              Eigen::MatrixXd& X, Eigen::VectorXd& Y) {
    KData kdata = stk.getKData(query);
    HKU_IF_RETURN(kdata.empty(), false);

    size_t factor_count = inds.size();
    HKU_IF_RETURN(factor_count == 0, false);

    IndicatorList all_inds(factor_count);
    global_parallel_for_index_void(0, factor_count, [&](size_t i) {
        all_inds[i] = ALIGN(inds[i], kdata, false)(kdata).getResult(0);
    });

    Indicator returns = ROCP(kdata.close(), 1);
    size_t n = returns.size();
    HKU_IF_RETURN(n <= factor_count, false);

    std::vector<size_t> valid_indices;
    valid_indices.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        if (!std::isfinite(returns[i]))
            continue;
        bool valid = true;
        for (const auto& ind : all_inds) {
            if (!std::isfinite(ind[i])) {
                valid = false;
                break;
            }
        }
        if (valid) {
            valid_indices.push_back(i);
        }
    }

    size_t valid_count = valid_indices.size();
    HKU_IF_RETURN(valid_count <= factor_count, false);

    X.resize(valid_count, factor_count + 1);
    Y.resize(valid_count);
    X.col(0).setOnes();
    for (size_t j = 0; j < factor_count; ++j) {
        for (size_t k = 0; k < valid_count; ++k) {
            X(k, j + 1) = all_inds[j][valid_indices[k]];
        }
    }
    for (size_t k = 0; k < valid_count; ++k) {
        Y(k) = returns[valid_indices[k]];
    }
    return true;
}

// Solve the least-squares coefficients with one shared column-pivoted Householder QR. Returns false
// on a rank-deficient design matrix (exact collinearity leaves no unique solution, and the QR basic
// solution would be an arbitrary one that hasNaN() cannot catch) or a non-finite result.
bool solve_regression(const Eigen::MatrixXd& X, const Eigen::VectorXd& Y, Eigen::VectorXd& beta) {
    Eigen::ColPivHouseholderQR<Eigen::MatrixXd> qr(X);
    HKU_IF_RETURN(qr.rank() < X.cols(), false);
    beta = qr.solve(Y);
    return !beta.hasNaN();
}

}  // namespace

/**
 * The multiple linear regression implementation
 *
 * @param stk the stock object
 * @param query the K-line query condition
 * @param inds the indicator list (the independent variables)
 * @return the regression coefficient vector [alpha, beta1, beta2, ...]
 */
std::vector<double> multi_regression(const Stock& stk, const KQuery& query,
                                     const IndicatorList& inds) {
    Eigen::MatrixXd X;
    Eigen::VectorXd Y;
    HKU_IF_RETURN(!build_regression_dataset(stk, query, inds, X, Y), {});

    Eigen::VectorXd beta;
    HKU_IF_RETURN(!solve_regression(X, Y, beta), {});

    std::vector<double> result(beta.size());
    for (size_t i = 0; i < beta.size(); ++i) {
        result[i] = beta(i);
    }
    return result;
}

/**
 * The complete version of the multiple linear regression implementation
 *
 * @param stk the stock object
 * @param query the K-line query condition
 * @param inds the indicator list (the independent variables)
 * @return the regression result vector [alpha, beta1, ..., betan, e1, ..., en, RSS, R2]
 */
std::vector<double> multi_regression_full(const Stock& stk, const KQuery& query,
                                          const IndicatorList& inds) {
    Eigen::MatrixXd X;
    Eigen::VectorXd Y;
    HKU_IF_RETURN(!build_regression_dataset(stk, query, inds, X, Y), {});

    Eigen::VectorXd beta;
    HKU_IF_RETURN(!solve_regression(X, Y, beta), {});

    Eigen::VectorXd residuals = Y - X * beta;
    double RSS = residuals.squaredNorm();
    double Y_mean = Y.mean();
    double TSS = (Y.array() - Y_mean).square().sum();
    double R_squared = TSS > 1e-10 ? (1.0 - RSS / TSS) : 0.0;

    size_t valid_count = static_cast<size_t>(Y.size());
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

namespace detail {

Indicator::value_t cross_section_spearman(const Indicator::value_t* a, const Indicator::value_t* b,
                                          size_t n) {
    if (n < 2)
        return Null<Indicator::value_t>();
    static thread_local std::vector<Indicator::value_t> tmpa, tmpb, levela, levelb;
    static thread_local std::vector<std::pair<Indicator::value_t, size_t>> idx_a, idx_b;
    tmpa.clear();
    tmpb.clear();
    if (tmpa.capacity() < n) {
        tmpa.reserve(n);
        tmpb.reserve(n);
    }
    for (size_t j = 0; j < n; j++) {
        if (std::isfinite(a[j]) && std::isfinite(b[j])) {
            tmpa.push_back(a[j]);
            tmpb.push_back(b[j]);
        }
    }
    size_t act = tmpa.size();
    if (act < 2)
        return Null<Indicator::value_t>();
    if (levela.size() < act) {
        levela.resize(act);
        levelb.resize(act);
    }
    auto rank_level = [](const Indicator::value_t* data, Indicator::value_t* level, size_t total,
                         std::vector<std::pair<Indicator::value_t, size_t>>& idx) {
        if (idx.size() != total)
            idx.resize(total);
        for (size_t i = 0; i < total; i++)
            idx[i] = {data[i], i};
        std::sort(idx.begin(), idx.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        size_t i = 0;
        while (i < total) {
            size_t j = i + 1;
            while (j < total && idx[i].first == idx[j].first)
                j++;
            Indicator::value_t score = ((Indicator::value_t)(i + j + 1)) / 2.0;
            for (size_t k = i; k < j; k++)
                level[idx[k].second] = score;
            i = j;
        }
    };
    rank_level(tmpa.data(), levela.data(), act, idx_a);
    rank_level(tmpb.data(), levelb.data(), act, idx_b);
    double mean_rank = (act + 1) / 2.0;
    double var_r = 0.0, var_s = 0.0, cov = 0.0;
    for (size_t j = 0; j < act; j++) {
        double dr = levela[j] - mean_rank;
        double ds = levelb[j] - mean_rank;
        var_r += dr * dr;
        var_s += ds * ds;
        cov += dr * ds;
    }
    if (var_r == 0.0 || var_s == 0.0)
        return 0.0;
    double rho = cov / std::sqrt(var_r * var_s);
    return std::max(-1.0, std::min(1.0, rho));
}

Indicator::value_t cross_section_pearson(const Indicator::value_t* a, const Indicator::value_t* b,
                                         size_t n) {
    if (n < 2)
        return Null<Indicator::value_t>();
    Indicator::value_t ex = 0.0, ey = 0.0, ex2 = 0.0, ey2 = 0.0, exy = 0.0;
    for (size_t j = 0; j < n; j++) {
        Indicator::value_t x = a[j], y = b[j];
        ex += x;
        ey += y;
        ex2 += x * x;
        ey2 += y * y;
        exy += x * y;
    }
    Indicator::value_t nf = (Indicator::value_t)n;
    Indicator::value_t varx = ex2 - ex * ex / nf;
    Indicator::value_t vary = ey2 - ey * ey / nf;
    Indicator::value_t cov = exy - ex * ey / nf;
    if (!(varx > 0.0 && vary > 0.0 && std::isfinite(varx) && std::isfinite(vary) &&
          std::isfinite(cov)))
        return Null<Indicator::value_t>();
    return cov / std::sqrt(varx * vary);
}

}  // namespace detail

}  // namespace hku