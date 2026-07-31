/*
 * test_style_regression.cpp
 *
 * 风格因子中性化残差回归（calculate_style_residuals）的白盒测试。
 *
 *   - golden 值：一元线性回归解析解核对；
 *   - 边界：样本不足全 NaN、自变量含 NaN 行置 NaN、秩亏共线不崩溃；
 *   - 全局状态：并发调用前后 Eigen::nbThreads() 不变（防回归：
 *     若重新引入运行时 Eigen::setNbThreads，本测试将失败）。
 */

#include "../../test_config.h"
#include <thread>
#include <Eigen/Core>
#include <hikyuu/trade_sys/multifactor/style_regression.h>

using namespace hku;

/**
 * @defgroup test_style_regression test_style_regression
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par golden：perfect fit 一元回归，残差全 0
 *  y = [1,2,3,4], x = [1,2,3,4]：解析解 slope=1, intercept=0，残差全 0
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

/** @par golden：非 perfect fit 一元回归，解析解核对
 *  y = [2,1,4,3], x = [0,1,2,3]：
 *    x_mean=1.5, y_mean=2.5
 *    slope = Σ(x-x̄)(y-ȳ)/Σ(x-x̄)² = 3/5 = 0.6
 *    intercept = ȳ - slope*x̄ = 2.5 - 0.6*1.5 = 1.6
 *    残差 = y - (1.6 + 0.6x) = [0.4, -1.2, 1.2, -0.4]
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

/** @par 边界：样本不足（valid_count <= k+1）返回全 NaN，不崩溃 */
TEST_CASE("test_style_regression_insufficient_samples") {
    PriceList y{1.0, 2.0};  // 2 个样本
    vector<PriceList> x{{1.0, 2.0}};
    auto residuals = calculate_style_residuals(y, x);

    CHECK_EQ(residuals.size(), y.size());
    for (size_t i = 0; i < residuals.size(); i++) {
        CHECK_UNARY(std::isnan(residuals[i]));
    }
}

/** @par 边界：自变量含 NaN 的行残差置 NaN，其他行正常 */
TEST_CASE("test_style_regression_nan_row") {
    // y = 1 + 2x 的完美数据，第 2 行 x 为 NaN
    PriceList y{1.0, 3.0, 5.0, 7.0};
    vector<PriceList> x{{0.0, 1.0, Null<price_t>(), 3.0}};
    auto residuals = calculate_style_residuals(y, x);

    CHECK_EQ(residuals.size(), y.size());
    // 有效行（0, 1, 3）perfect fit 残差 0
    CHECK_EQ(residuals[0], doctest::Approx(0.0).epsilon(1e-9));
    CHECK_EQ(residuals[1], doctest::Approx(0.0).epsilon(1e-9));
    // NaN 行不参与拟合，残差置 NaN
    CHECK_UNARY(std::isnan(residuals[2]));
    CHECK_EQ(residuals[3], doctest::Approx(0.0).epsilon(1e-9));
}

/** @par 边界：秩亏共线自变量（两列完全共线）不崩溃，不产生 Inf */
TEST_CASE("test_style_regression_rank_deficient") {
    PriceList y{1.0, 2.0, 3.0, 4.0};
    // 两列完全共线：x1 = 2*x0
    vector<PriceList> x{{1.0, 2.0, 3.0, 4.0}, {2.0, 4.0, 6.0, 8.0}};
    auto residuals = calculate_style_residuals(y, x);

    CHECK_EQ(residuals.size(), y.size());
    for (size_t i = 0; i < residuals.size(); i++) {
        CHECK_UNARY(!std::isinf(residuals[i]));
        CHECK_UNARY(!std::isnan(residuals[i]) || residuals[i] == residuals[i]);
    }
}

/** @par 全局状态：并发调用前后 Eigen 线程配置不变
 *  Eigen::nbThreads() 在 <Eigen/Core> 中无条件存在（未启用 OpenMP 时返回 1），
 *  因此无条件断言：无论初始值是 1 还是 N，并发结束后必须保持不变。
 *  防回归：若重新引入运行时 Eigen::setNbThreads 全局切换，本测试将失败。
 */
TEST_CASE("test_style_regression_eigen_threads_unchanged") {
    PriceList y{2.0, 1.0, 4.0, 3.0, 6.0, 5.0, 8.0, 7.0};
    vector<PriceList> x{{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0}};

    int threads_before = Eigen::nbThreads();
    std::vector<PriceList> results(8);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < 8; i++) {
        threads.emplace_back([&, i]() {
            // 每线程写自己的槽，主线程 join 后再校验
            for (size_t j = 0; j < 20; j++) {
                results[i] = calculate_style_residuals(y, x);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    // 并发调用前后 Eigen 全局线程配置必须不变
    CHECK_EQ(Eigen::nbThreads(), threads_before);
    for (const auto& r : results) {
        CHECK_EQ(r.size(), y.size());
    }
}

/** @} */
