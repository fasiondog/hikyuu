/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-31
 *      Author: woleigegg
 *
 *  The white box test of the style factor neutralization residual regression
 * (calculate_style_residuals).
 *
 *   - golden values: checked against the analytical solution of the unary linear regression;
 *   - boundaries: all NaN with few samples, NaN rows with a NaN variable, no crash when collinear;
 *   - the global state: Eigen::nbThreads() is unchanged around the concurrent calls
 * (anti-regression: if a runtime Eigen::setNbThreads is reintroduced this test fails).
 */

#include "../../test_config.h"
#include <thread>
#include <Eigen/Core>
#include <hikyuu/trade_sys/multifactor/StyleRegression.h>

using namespace hku;

/**
 * @defgroup test_style_regression test_style_regression
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par golden: a perfect fit unary regression with all the residuals 0
 *  y = [1,2,3,4] and x = [1,2,3,4]: the analytical solution is slope=1, intercept=0, residuals 0
 */
TEST_CASE("test_style_regression_perfect_fit") {
    PriceList y{1.0, 2.0, 3.0, 4.0};
    vector<PriceList> x{{1.0, 2.0, 3.0, 4.0}};
    auto residuals = calculate_style_residuals(y, x);

    CHECK_EQ(residuals.size(), y.size());
    for (size_t i = 0; i < residuals.size(); i++) {
        CHECK_EQ(residuals[i], doctest::Approx(0.0).epsilon(1e-9));
    }
}

/** @par golden: a non-perfect fit unary regression checked against the analytical solution
 *  y = [2,1,4,3] and x = [0,1,2,3]:
 *    x_mean=1.5, y_mean=2.5
 *    slope = Σ(x-x̄)(y-ȳ)/Σ(x-x̄)² = 3/5 = 0.6
 *    intercept = ȳ - slope*x̄ = 2.5 - 0.6*1.5 = 1.6
 *    the residuals = y - (1.6 + 0.6x) = [0.4, -1.2, 1.2, -0.4]
 */
TEST_CASE("test_style_regression_golden_values") {
    PriceList y{2.0, 1.0, 4.0, 3.0};
    vector<PriceList> x{{0.0, 1.0, 2.0, 3.0}};
    auto residuals = calculate_style_residuals(y, x);

    PriceList expect{0.4, -1.2, 1.2, -0.4};
    CHECK_EQ(residuals.size(), expect.size());
    for (size_t i = 0; i < residuals.size(); i++) {
        CHECK_EQ(residuals[i], doctest::Approx(expect[i]).epsilon(1e-9));
    }
}

/** @par Boundary: with few samples (valid_count <= k+1) all NaN is returned without a crash */
TEST_CASE("test_style_regression_insufficient_samples") {
    PriceList y{1.0, 2.0};  // 2 samples
    vector<PriceList> x{{1.0, 2.0}};
    auto residuals = calculate_style_residuals(y, x);

    CHECK_EQ(residuals.size(), y.size());
    for (size_t i = 0; i < residuals.size(); i++) {
        CHECK_UNARY(std::isnan(residuals[i]));
    }
}

/** @par Boundary: the residual of a row with a NaN variable is NaN and the other rows are normal */
TEST_CASE("test_style_regression_nan_row") {
    // The perfect data of y = 1 + 2x, with the x of row 2 being NaN
    PriceList y{1.0, 3.0, 5.0, 7.0};
    vector<PriceList> x{{0.0, 1.0, Null<price_t>(), 3.0}};
    auto residuals = calculate_style_residuals(y, x);

    CHECK_EQ(residuals.size(), y.size());
    // The valid rows (0, 1, 3) have a perfect fit and the residuals are 0
    CHECK_EQ(residuals[0], doctest::Approx(0.0).epsilon(1e-9));
    CHECK_EQ(residuals[1], doctest::Approx(0.0).epsilon(1e-9));
    // The NaN row does not participate in the fit, its residual is NaN
    CHECK_UNARY(std::isnan(residuals[2]));
    CHECK_EQ(residuals[3], doctest::Approx(0.0).epsilon(1e-9));
}

/** @par Boundary: collinear variables (two fully collinear columns) neither crash nor give Inf */
TEST_CASE("test_style_regression_rank_deficient") {
    PriceList y{1.0, 2.0, 3.0, 4.0};
    // The two columns are fully collinear: x1 = 2*x0
    vector<PriceList> x{{1.0, 2.0, 3.0, 4.0}, {2.0, 4.0, 6.0, 8.0}};
    auto residuals = calculate_style_residuals(y, x);

    CHECK_EQ(residuals.size(), y.size());
    for (size_t i = 0; i < residuals.size(); i++) {
        CHECK_UNARY(!std::isinf(residuals[i]));
        CHECK_UNARY(!std::isnan(residuals[i]) || residuals[i] == residuals[i]);
    }
}

/** @par The global state: the Eigen thread configuration is unchanged around the concurrent calls
 *  Eigen::nbThreads() exists unconditionally in <Eigen/Core> (it returns 1 without OpenMP),
 *  so it is asserted unconditionally: whether it starts as 1 or N, it must stay afterwards.
 *  Anti-regression: if a runtime global Eigen::setNbThreads switch is reintroduced, this fails. */
*/ TEST_CASE("test_style_regression_eigen_threads_unchanged") {
    PriceList y{2.0, 1.0, 4.0, 3.0, 6.0, 5.0, 8.0, 7.0};
    vector<PriceList> x{{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0}};

    int threads_before = Eigen::nbThreads();
    std::vector<PriceList> results(8);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < 8; i++) {
        threads.emplace_back([&, i]() {
            // Every thread writes its own slot and the main thread verifies after the join
            for (size_t j = 0; j < 20; j++) {
                results[i] = calculate_style_residuals(y, x);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    // The Eigen global thread configuration must not change around the concurrent calls
    CHECK_EQ(Eigen::nbThreads(), threads_before);
    for (const auto& r : results) {
        CHECK_EQ(r.size(), y.size());
    }
}

/** @} */
