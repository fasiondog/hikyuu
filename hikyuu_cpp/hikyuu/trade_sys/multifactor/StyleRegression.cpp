/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-27
 *      Author: woleigegg
 *
 *  The residual regression of the style factor neutralization, a serial kernel extracted from
 *  MultiFactorBase.
 *
 *  Design points:
 *    - The Eigen ColPivHouseholderQR solver is kept, which is not in the Eigen parallel
 *      algorithm set itself;
 *    - The fitted values use an explicit row-by-row dot product, avoiding the parallel path of
 *      the general matrix-matrix multiplication;
 *    - The process level Eigen::setNbThreads is not called, avoiding the concurrent MFs
 *      polluting the global config mutually;
 *    - The valid sample / rank deficiency / NaN semantics of the original calculate_residuals
 *      are kept.
 */

#include <cmath>
#include <Eigen/Dense>
#include "StyleRegression.h"
#include "hikyuu/utilities/Null.h"

namespace hku {

PriceList calculate_style_residuals(const PriceList& y, const vector<PriceList>& x) {
    HKU_ASSERT(!x.empty());
    size_t n = y.size();
    for (const auto& xi : x) {
        HKU_ASSERT(xi.size() == n);
    }

    PriceList residuals(n, Null<price_t>());
    size_t k = x.size();  // The number of the explanatory variables

    // Build the design matrix and the dependent variable vector
    Eigen::MatrixXd Xmat(n, k + 1);
    Eigen::VectorXd Yvec(n);

    // The first column is the constant term (all 1)
    Xmat.col(0).setConstant(1.0);

    // Mark the valid data points
    std::vector<bool> valid(n, true);

    for (size_t i = 0; i < n; ++i) {
        Yvec(i) = y[i];

        // Check whether the dependent variable is valid
        if (std::isnan(y[i]) || std::isinf(y[i])) {
            valid[i] = false;
            continue;
        }

        // Fill the independent variables and check their validity
        for (size_t j = 0; j < k; ++j) {
            Xmat(i, j + 1) = x[j][i];
            if (std::isnan(x[j][i]) || std::isinf(x[j][i])) {
                valid[i] = false;
                break;
            }
        }
    }

    // Count the valid data points
    size_t valid_count = std::count(valid.begin(), valid.end(), true);

    // There are not enough data points
    if (valid_count <= k + 1) {
        return residuals;
    }

    // Create the submatrix of the valid data
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

    // Solve the linear regression with the QR decomposition: beta = (X'X)^(-1)X'Y
    Eigen::VectorXd beta = X_valid.colPivHouseholderQr().solve(Y_valid);

    // Check whether the solution is valid
    if (beta.hasNaN()) {
        return residuals;
    }

    // Calculate the residuals: the fitted value is a row-by-row dot product, avoiding the general
    // matrix-matrix parallel path
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