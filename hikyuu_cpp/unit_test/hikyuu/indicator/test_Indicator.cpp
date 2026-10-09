/*
 * test_Indicator.cpp
 *
 *  Created on: 2013-4-11
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/indicator/build_in.h>
#include <hikyuu/StockManager.h>

/**
 * @defgroup test_indicator_Indicator test_indicator_Indicator
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_indicator_other") {
    double dx = Null<double>();
    size_t ix = size_t(dx);
    // A random value
    HKU_INFO("double nan to size_t: {}", ix);

    float fx = Null<float>();
    ix = size_t(fx);
    // A random value
    HKU_INFO("float nan to size_t: {}", ix);
}

/** @par Test points */
TEST_CASE("test_indicator_alike") {
    /** @arg The empty indicator comparison */
    CHECK_UNARY(Indicator().alike(Indicator()));

    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    for (size_t i = 0, len = data1.size(); i < len; i++) {
        CHECK(data1[i] != data2[i]);
    }

    CHECK_UNARY(!data1.alike(data2));

    for (size_t i = 0; i < 10; ++i) {
        d2[i] = i;
    }
    data2 = PRICELIST(d2);
    for (size_t i = 0, len = data1.size(); i < len; i++) {
        CHECK(data1[i] == data2[i]);
    }
    CHECK_UNARY(data1.alike(data2));
}

TEST_CASE("test_indicator_alike_dynamic_parameters") {
    Indicator ma5 = MA(CLOSE(), IndParam(CVAL(CLOSE(), 5)));
    Indicator ma10 = MA(CLOSE(), IndParam(CVAL(CLOSE(), 10)));
    Indicator ma5_copy = MA(CLOSE(), IndParam(CVAL(CLOSE(), 5)));

    CHECK_FALSE(ma5.alike(ma10));
    CHECK_UNARY(ma5.alike(ma5_copy));

    Indicator corr_close = CORR(CLOSE(), OPEN(), 10);
    Indicator corr_high = CORR(HIGH(), OPEN(), 10);
    Indicator corr_close_copy = CORR(CLOSE(), OPEN(), 10);

    CHECK_FALSE(corr_close.alike(corr_high));
    CHECK_UNARY(corr_close.alike(corr_close_copy));

    Indicator corr_open_template = CORR(OPEN(), 10);
    Indicator corr_high_template = CORR(HIGH(), 10);
    Indicator corr_open_template_copy = CORR(OPEN(), 10);

    CHECK_FALSE(corr_open_template.alike(corr_high_template));
    CHECK_UNARY(corr_open_template.alike(corr_open_template_copy));

    CHECK_FALSE(DROPNA(CLOSE()).alike(DROPNA(CLOSE())));
}

/** @par Test points */
TEST_CASE("test_operator_add") {
    /** @arg The normal addition */
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = data1 + data2;

    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], i + i + 1);
    }

    /** @arg The two ind to add have different sizes and one of them is 0 */
    Indicator data3;
    result = data1 + data3;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind to add have different sizes and one of them is 0 */
    PriceList d3;
    for (size_t i = 0; i < 20; ++i) {
        d3.push_back(i);
    }
    data3 = PRICELIST(d3);
    result = data1 + data3;
    CHECK_EQ(data1.size(), 10);
    CHECK_EQ(data3.size(), 20);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.discard(), 10);
    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < 20; ++i) {
        CHECK_EQ(result[i], i + i - 10);
    }

    /** @arg The two ind to add have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = k + data1;
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.getResultNumber(), 1);
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], (k[i] + data1[i]));
    }
}

#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_operator_add_benchmark") {
    PriceList d1, d2;
    for (size_t i = 0; i < 10000; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);

    int cycle = 10000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(Indicator_add, cycle, HKU_CSTR(""));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = data1 + data2;
            DO_NOT_OPTIMIZE(result);
        }
    }
}
#endif

/** @par Test points */
TEST_CASE("test_operator_sub") {
    /** @arg The normal subtraction */
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = data1 - data2;
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data1[i] - data2[i]);
    }

    /** @arg The two ind to subtract have different sizes */
    Indicator data3;
    result = data1 - data3;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind to subtract have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = k - data1;
    CHECK_EQ(result.size(), k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], (k[i] - data1[i]));
    }
}

#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_operator_sub_benchmark") {
    PriceList d1, d2;
    for (size_t i = 0; i < 10000; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);

    int cycle = 10000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(Indicator_sub, cycle, HKU_CSTR(""));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = data1 - data2;
        }
    }
}
#endif

/** @par Test points */
TEST_CASE("test_operator_multi") {
    /** @arg The normal multiplication */
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = data1 * data2;
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data1[i] * data2[i]);
    }

    /** @arg The two ind to multiply have different sizes */
    Indicator data3;
    result = data1 * data3;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind to multiply have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = k * data1;
    CHECK_EQ(result.size(), k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], (k[i] * data1[i]));
    }
}

#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_operator_multi_benchmark") {
    PriceList d1, d2;
    for (size_t i = 0; i < 10000; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);

    int cycle = 10000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(Indicator_multi, cycle, HKU_CSTR(""));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = data1 * data2;
        }
    }
}
#endif

/** @par Test points */
TEST_CASE("test_operator_division") {
    /** @arg The normal division */
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = data2 / data1;
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        if (data1[i] == 0.0) {
            CHECK_UNARY((std::isinf(result[i]) || std::isnan(result[i])));
        } else {
            CHECK_EQ(result[i], doctest::Approx(data2[i] / data1[i]));
        }
    }

    /** @arg The two ind to divide have different sizes */
    Indicator data3;
    result = data1 / data3;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind to divide have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = k / data1;
    CHECK_EQ(result.size(), k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        if (data1[i] == 0.0) {
            CHECK_UNARY(std::isinf(result[i]) || std::isnan(result[i]));
        } else {
            CHECK_EQ(result[i], doctest::Approx(k[i] / data1[i]));
        }
    }
}

#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_operator_division_benchmark") {
    PriceList d1, d2;
    for (size_t i = 0; i < 10000; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);

    int cycle = 10000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(Indicator_div, cycle, HKU_CSTR(""));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = data1 / data2;
        }
    }
}
#endif

/** @par Test points */
TEST_CASE("test_operator_mod") {
    /** @arg The normal modulo */
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i + 2);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = data2 % data1;
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 2);
    CHECK_EQ(result[4], 2);
    CHECK_EQ(result[5], 2);
    CHECK_EQ(result[6], 2);
    CHECK_EQ(result[7], 2);
    CHECK_EQ(result[8], 2);
    CHECK_EQ(result[9], 2);

    /** @arg The two ind to divide have different sizes */
    Indicator data3;
    result = data1 % data3;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind to divide have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = k % data1;
    CHECK_EQ(result.size(), k.size());
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 1);
    CHECK_EQ(result[3], 1);
    CHECK_EQ(result[4], 3);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 3);
    CHECK_EQ(result[7], 6);
    CHECK_EQ(result[8], 2);
    CHECK_EQ(result[9], 8);
}

#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_operator_mod_benchmark") {
    PriceList d1, d2;
    for (size_t i = 0; i < 10000; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);

    int cycle = 10000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(Indicator_mod, cycle, HKU_CSTR(""));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = data1 % data2;
        }
    }
}
#endif

/** @par Test points */
TEST_CASE("test_operator_eq") {
    /** @arg The normal equality */
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = (data2 == data1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], true);
    }

    /** @arg The two ind have different sizes */
    Indicator data3;
    result = (data1 == data3);
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = (k == data1);
    CHECK_EQ(result.size(), k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], false);
    }
}

#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_operator_eq_benchmark") {
    PriceList d1, d2;
    for (size_t i = 0; i < 10000; ++i) {
        d1.push_back(i);
        d2.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);

    int cycle = 10000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(Indicator_eq, cycle, HKU_CSTR(""));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = data1 == data2;
        }
    }
}
#endif

/** @par Test points */
TEST_CASE("test_operator_ne") {
    /** @arg The normal inequality */
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = (data2 != data1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], false);
    }

    /** @arg The two ind have different sizes */
    Indicator data3;
    result = (data1 != data3);
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = (k != data1);
    CHECK_EQ(result.size(), k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], true);
    }
}

/** @par Test points */
TEST_CASE("test_operator_gt") {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
        d3.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1 > ind2*/
    Indicator result = (data3 > data1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 1.0);
    }

    /** @arg ind1 < ind2 */
    result = (data1 > data3);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0.0);
    }

    /** @arg ind1 == ind2 */
    result = (data1 > data2);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0.0);
    }

    /** @arg The two ind have different sizes */
    Indicator data4;
    result = data1 > data4;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = (k > data1);
    CHECK_EQ(result.size(), k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], 1.0);
    }
}

/** @par Test points */
TEST_CASE("test_operator_ge") {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
        d3.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1 > ind2*/
    Indicator result = (data3 >= data1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 1.0);
    }

    /** @arg ind1 < ind2 */
    result = (data1 >= data3);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0.0);
    }

    /** @arg ind1 == ind2 */
    result = (data1 >= data2);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 1.0);
    }

    /** @arg The two ind have different sizes */
    Indicator data4;
    result = data1 >= data4;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = (k >= data1);
    CHECK_EQ(result.size(), k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], 1.0);
    }
}

/** @par Test points */
TEST_CASE("test_operator_lt") {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
        d3.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1 > ind2*/
    Indicator result = (data3 < data1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0.0);
    }

    /** @arg ind1 < ind2 */
    result = (data1 < data3);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 1.0);
    }

    /** @arg ind1 == ind2 */
    result = (data1 < data2);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0.0);
    }

    /** @arg The two ind have different sizes */
    Indicator data4;
    result = data1 < data4;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = (k < data1);
    CHECK_EQ(result.size(), k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], 0.0);
    }
}

/** @par Test points */
TEST_CASE("test_operator_le") {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
        d3.push_back(i + 1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1 > ind2*/
    Indicator result = (data3 <= data1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0.0);
    }

    /** @arg ind1 < ind2 */
    result = (data1 <= data3);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 1.0);
    }

    /** @arg ind1 == ind2 */
    result = (data1 <= data2);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 1.0);
    }

    /** @arg The two ind have different sizes */
    Indicator data4;
    result = data1 <= data4;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);

    /** @arg The two ind have the same size but a different result_number */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    CHECK_EQ(k.size(), data1.size());
    result = (k <= data1);
    CHECK_EQ(result.size(), k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], 0.0);
    }
}

/** @par Test points */
TEST_CASE("test_getResult_getResultAsPriceList") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query;
    KData kdata;
    Indicator ikdata, result1;
    PriceList result2;

    /** @arg The source data is empty */
    ikdata = KDATA(kdata);
    result1 = ikdata.getResult(0);
    result2 = ikdata.getResultAsPriceList(0);
    CHECK_EQ(result1.size(), 0);
    CHECK_EQ(result2.size(), 0);

    /** @arg The invalid result_num parameter */
    query = KQuery(0, 10);
    kdata = stock.getKData(query);
    ikdata = KDATA(kdata);
    CHECK_EQ(ikdata.size(), 10);
    result1 = ikdata.getResult(6);
    result2 = ikdata.getResultAsPriceList(6);
    CHECK_EQ(result1.size(), 0);
    CHECK_EQ(result2.size(), 0);

    /** @arg The normal getting */
    result1 = ikdata.getResult(0);
    result2 = ikdata.getResultAsPriceList(1);
    CHECK_EQ(result1.size(), 10);
    CHECK_EQ(result2.size(), 10);
    CHECK_EQ(result1[0], 29.5);
    CHECK_LT(std::fabs(result1[1] - 27.58), 0.0001);
    CHECK_EQ(result1[9], doctest::Approx(26.45));

    CHECK_EQ(result2[0], doctest::Approx(29.8));
    CHECK_EQ(result2[1], doctest::Approx(28.38));
    CHECK_EQ(result2[9], doctest::Approx(26.55));
}

/** @par Test points */
TEST_CASE("test_LOGIC_AND") {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(0);
        d2.push_back(1);
        d3.push_back(i);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1 is all 0 and ind2 is all 1 */
    Indicator result = data1 & data2;
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0.0);
    }

    /** @arg ind is all 0 and val is 1 */
    /*result = IND_AND(data1, 1.0);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }*/

    /** @arg ind1 is all 0 and ind2 is an integer starting from 0 */
    result = data1 & data3;
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0.0);
    }

    /** @arg ind1 is all 1 and ind2 is an integer starting from 0 */
    result = data2 & data3;
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0.0);
    for (size_t i = 1; i < 10; ++i) {
        CHECK_EQ(result[i], 1.0);
    }

    /** @arg The two ind have different sizes */
    Indicator data4;
    result = data1 & data4;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);
}

/** @par Test points */
TEST_CASE("test_LOGIC_OR") {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(0);
        d2.push_back(1);
        d3.push_back(i);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1 is all 0 and ind2 is all 1 */
    Indicator result = data1 | data2;
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 1.0);
    }

    /** @arg ind1 is all 0 and ind2 is an integer starting from 0 */
    result = data1 | data3;
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0.0);
    for (size_t i = 1; i < 10; ++i) {
        CHECK_EQ(result[i], 1.0);
    }

    /** @arg ind1 is all 1 and ind2 is an integer starting from 0 */
    result = data2 | data3;
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 1.0);
    }

    /** @arg The two ind have different sizes */
    Indicator data4;
    result = data1 | data4;
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.size(), 0);
}

/** @par Test points */
TEST_CASE("test_indicator_increment_calculate") {
    auto VAR1 = LLV(LOW(), 13);
    auto VAR2 = HHV(HIGH(), 13);
    auto VAR3 = SMA((CLOSE() - VAR1) / (VAR2 - VAR1) * 100, 5, 1);
    auto VAR4 = SMA((VAR2 - CLOSE()) / (VAR2 - VAR1) * 100, 5, 1);
    auto AA = VAR3;
    auto BB = VAR4;
    auto VAR5 = SMA(MAX(CLOSE() - REF(CLOSE(), 1), 0), 5, 1) /
                SMA(ABS(CLOSE() - REF(CLOSE(), 1)), 5, 1) * 100;
    auto CC = EMA(VAR5, 3);
    auto XG = CROSS(CC, BB) & (CC >= REF(CC, 1)) & (BB <= REF(BB, 3)) & (CC >= 49.5) &
              (MA(CLOSE(), 3) >= REF(MA(CLOSE(), 3), 1)) &
              (MA(CLOSE(), 7) >= REF(MA(CLOSE(), 7), 1)) &
              (MA(CLOSE(), 60) > REF(MA(CLOSE(), 60), 3));

    Stock stk = getStock("sh600000");
    KData k1 = stk.getKData(KQuery(100, 300));
    KData k2 = stk.getKData(KQuery(200, 500));
    auto x = XG(k1);
    auto y = XG(k2);
    x.setContext(k2);
    CHECK_EQ(x.size(), y.size());
    CHECK_EQ(x[159], 1.0);
    CHECK_EQ(x[159], y[159]);
}

/** @par Test points */
TEST_CASE("test_combineCalculateIndicators") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 20);
    KData kdata = stock.getKData(query);

    /** @arg An empty indicator list */
    IndicatorList empty_indicators;
    IndicatorList result = combineCalculateIndicators(empty_indicators, kdata);
    CHECK_EQ(result.size(), 0);

    /** @arg A single simple indicator */
    Indicator close_ind = CLOSE();
    IndicatorList single_indicator{close_ind};
    result = combineCalculateIndicators(single_indicator, kdata);
    check_indicator(result[0], CLOSE(kdata));

    /** @arg Multiple simple indicators */
    Indicator open_ind = OPEN();
    Indicator high_ind = HIGH();
    Indicator low_ind = LOW();
    IndicatorList multi_indicators{close_ind, open_ind, high_ind, low_ind};
    result = combineCalculateIndicators(multi_indicators, kdata);
    CHECK_EQ(result.size(), 4);
    check_indicator(result[0], CLOSE(kdata));
    check_indicator(result[1], OPEN(kdata));
    check_indicator(result[2], HIGH(kdata));
    check_indicator(result[3], LOW(kdata));

    /** @arg It contains a composite indicator */
    Indicator ma_close = MA(CLOSE(), 5);
    Indicator rsi_close = RSI(CLOSE(), 14);
    IndicatorList complex_indicators{ma_close, rsi_close};
    result = combineCalculateIndicators(complex_indicators, kdata);
    check_indicator(result[0], MA(CLOSE(kdata), 5));
    check_indicator(result[1], RSI(CLOSE(kdata), 14));

    /** @arg Test the tovalue parameter being true */
    result = combineCalculateIndicators(complex_indicators, kdata, true);
    CHECK_EQ(result.size(), 2);
    // When tovalue is true only the first result column should be returned
    CHECK_EQ(result[0].getResultNumber(), 1);
    CHECK_EQ(result[1].getResultNumber(), 1);
    CHECK_UNARY(result[0].equal(MA(CLOSE(kdata), 5)));
    CHECK_UNARY(result[1].equal(RSI(CLOSE(kdata), 14)));

    /** @arg Test the different KData contexts */
    KQuery query2(10, 30);
    KData kdata2 = stock.getKData(query2);
    result = combineCalculateIndicators(multi_indicators, kdata2);
    check_indicator(result[0], CLOSE(kdata2));
    check_indicator(result[1], OPEN(kdata2));
    check_indicator(result[2], HIGH(kdata2));
    check_indicator(result[3], LOW(kdata2));

    /** @arg Test the indicators containing the same child node (they should be deduplicated) */
    Indicator close1 = CLOSE();
    Indicator close2 = CLOSE();  // The same indicator
    IndicatorList duplicate_indicators{close1, close2};
    result = combineCalculateIndicators(duplicate_indicators, kdata);
    CHECK_EQ(result.size(), 2);
    // Although they are the same indicator, they are cloned into different instances
    CHECK_NE(result[0].getImp().get(), result[1].getImp().get());
    check_indicator(result[0], CLOSE(kdata));
    check_indicator(result[1], CLOSE(kdata));
}

/** @par Test points */
TEST_CASE("test_Indicator_operator_alike_non_cval") {
    // Verify that the alike short-circuit fix in Indicator::operator() also applies to the non-CVAL
    // operators. For a PRICELIST leaf of the same type and parameters, the base class alike goes
    // through the leaf size/data comparison (not the selfAlike of ICval) Two identical nested
    // PRICELIST -> alike true -> after the fix it returns ind (reusing the argument)

    PriceList d;
    for (int i = 0; i < 5; i++) {
        d.push_back(i + 1);  // [1,2,3,4,5]
    }
    Indicator pl1 = PRICELIST(d);
    Indicator pl2 = PRICELIST(d);
    CHECK_EQ(pl1.size(), 5);
    CHECK_EQ(pl2.size(), 5);
    CHECK_NE(pl1.getImp().get(), pl2.getImp().get());  // Two independent instances

    // pl1(pl2): pl1 is the operator and pl2 the operand, both alike true -> return pl2
    Indicator result = pl1(pl2);
    CHECK_EQ(result.size(), pl2.size());
    for (size_t i = 0; i < pl2.size(); ++i) {
        CHECK_EQ(result[i], pl2[i]);
    }
    // A white box assertion: pl2 is reused (pl1 is not cloned)
    CHECK_EQ(result.getImp().get(), pl2.getImp().get());
}

/** @par Test points */
TEST_CASE("test_indicator_access_result_num_bound") {
    PriceList d;
    for (size_t i = 0; i < 5; ++i) {
        d.push_back(i + 1);  // [1,2,3,4,5]
    }
    Indicator ind = PRICELIST(d);
    IndicatorImpPtr imp = ind.getImp();

    /** @arg num within [0, MAX_RESULT_NUM) is still accessible */
    CHECK_EQ(ind.get(0, 0), 1.0);
    CHECK_EQ(ind.front(0), 1.0);
    CHECK_EQ(ind.back(0), 5.0);

    /** @arg data with an out-of-bounds result_idx returns nullptr instead of reading OOB */
    CHECK_UNARY(imp->data(MAX_RESULT_NUM) == nullptr);
    CHECK_UNARY(imp->data(0) != nullptr);

#if CHECK_ACCESS_BOUND
    /** @arg num == MAX_RESULT_NUM is an out-of-bounds index and should throw (the throw
     *  guard of get/front/back is only compiled when CHECK_ACCESS_BOUND is enabled) */
    CHECK_THROWS_AS(ind.get(0, MAX_RESULT_NUM), std::out_of_range);
    CHECK_THROWS_AS(ind.front(MAX_RESULT_NUM), std::out_of_range);
    CHECK_THROWS_AS(ind.back(MAX_RESULT_NUM), std::out_of_range);
#endif
}

/** @par Test points */
TEST_CASE("test_indicator_imp_not_copyable") {
    /** @arg IndicatorImp owns raw result buffers, so it must not be copy constructible/assignable
     *  (an implicit copy would be a shallow copy causing a double free) */
    static_assert(!std::is_copy_constructible<IndicatorImp>::value,
                  "IndicatorImp must not be copy constructible");
    static_assert(!std::is_copy_assignable<IndicatorImp>::value,
                  "IndicatorImp must not be copy assignable");

    /** @arg the supported copy path is clone(), which yields an independent instance */
    IndicatorImpPtr imp = Indicator().getImp();
    IndicatorImpPtr cloned = imp->clone();
    CHECK_UNARY(cloned != nullptr);
    CHECK_UNARY(cloned.get() != imp.get());
}

/** @par Test points */
TEST_CASE("test_indicator_execute_mod") {
    const double nan = Null<double>();
    const double inf = std::numeric_limits<double>::infinity();
    const double i64_min = -9223372036854775808.0;   // -2^63, exactly INT64_MIN
    const double over = 9223372036854775808.0;       // 2^63, out of int64 range

    PriceList a, b;
    for (double v : {7.0, 7.0, nan, inf, i64_min, over}) {
        a.push_back(v);
    }
    for (double v : {3.0, 0.0, 2.0, 2.0, -1.0, 2.0}) {
        b.push_back(v);
    }
    Indicator r = PRICELIST(a) % PRICELIST(b);
    CHECK_EQ(r.size(), 6);

    /** @arg normal integer modulo */
    CHECK_EQ(r[0], 1.0);
    /** @arg zero divisor -> null */
    CHECK_UNARY(std::isnan(r[1]));
    /** @arg NaN operand -> null instead of UB in double to int64 conversion */
    CHECK_UNARY(std::isnan(r[2]));
    /** @arg Inf operand -> null instead of UB */
    CHECK_UNARY(std::isnan(r[3]));
    /** @arg INT64_MIN % -1 -> 0 without SIGFPE */
    CHECK_EQ(r[4], 0.0);
    /** @arg operand beyond int64 range -> null */
    CHECK_UNARY(std::isnan(r[5]));
}

namespace {

/** Compare the result of an incremental context switch with a plain full calculation */
template <typename Factory>
void checkIncrementEqualsFull(const Factory& make, const KData& first, const KData& then) {
    Indicator expect = make();
    expect.setContext(then);

    Indicator got = make();
    got.setContext(first);
    got.setContext(then);

    // One aggregated assertion per call: the runner stops after a few failed asserts, so counting
    // the mismatched points keeps every scenario observable in a single run
    bool same_shape =
      got.size() == expect.size() && got.getResultNumber() == expect.getResultNumber();
    size_t diff = same_shape ? 0 : expect.size() + 1;
    if (same_shape) {
        if (got.discard() != expect.discard()) {
            ++diff;
        }
        for (size_t r = 0; r < expect.getResultNumber(); ++r) {
            for (size_t i = 0; i < expect.size(); ++i) {
                double a = expect.get(i, r);
                double b = got.get(i, r);
                if (std::isnan(a) && std::isnan(b)) {
                    continue;
                }
                if (!(b == doctest::Approx(a).epsilon(0.0001))) {
                    ++diff;
                }
            }
        }
    }
    CHECK_MESSAGE(diff == 0, "mismatched points " << diff);
}

}  // namespace

/** @par Test points */
TEST_CASE("test_indicator_operand_length_must_match_context") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 20));
    CHECK_EQ(k_full.size(), 20);

    PriceList px;
    for (size_t i = 0; i < 40; ++i) {
        px.push_back(double(i + 1));
    }

    // An indicator carrying its own data length (SLICE, DROPNA) has no value per bar of a context,
    // so binding it to a context node is rejected instead of being guessed by position
    auto bind = [&](const Indicator& ind) {
        Indicator x = ind;
        x.setContext(k_full);
        return x[0];
    };

    /** @arg a binary node rejects an operand of another length */
    CHECK_THROWS(bind(CLOSE() + SLICE(px, 0, 12)));
    /** @arg a weave node rejects an operand of another length */
    CHECK_THROWS(bind(WEAVE(CLOSE(), SLICE(px, 0, 12))));
    /** @arg a condition node rejects an operand of another length */
    CHECK_THROWS(bind(IF(CLOSE(), OPEN(), SLICE(px, 0, 12))));
    /** @arg a unary node rejects an input of another length */
    CHECK_THROWS(bind(EMA(SLICE(px, 0, 12), 3)));

    // A data only tree has no context to disagree with, so the longest operand stays the anchor and
    // the others are merged at its right end
    Indicator data = PRICELIST(px) + SLICE(px, 0, 12);

    /** @arg a data only tree still merges at the right end, with a null leading part */
    CHECK_EQ(data.size(), 40);
    CHECK_EQ(data.discard(), 28);
    CHECK_UNARY(std::isnan(data[0]));
    CHECK_EQ(data[28], doctest::Approx(px[0] + px[28]));
    CHECK_EQ(data[39], doctest::Approx(px[11] + px[39]));

    /** @arg a unary node over plain data keeps the length of its input */
    CHECK_EQ(EMA(SLICE(px, 0, 12), 3).size(), 12);
}
/** @par Test points */
TEST_CASE("test_indicator_cval_if_and_context_padding") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 20));
    Indicator closes = k_full.close();
    Indicator opens = k_full.open();

    PriceList px12;
    for (size_t i = 0; i < 12; ++i) {
        px12.push_back(double(i + 1));
    }

    // IF takes a scalar branch by wrapping it into CVAL over the other branch, which follows its
    // length, so every form keeps the context length
    Indicator i1 = IF(CLOSE() > OPEN(), HIGH(), LOW());
    Indicator i2 = IF(CLOSE() > OPEN(), 1.0, 0.0);
    Indicator i3 = IF(CLOSE() > OPEN(), CLOSE(), 1.0);
    Indicator i4 = IF(CLOSE() > OPEN(), 1.0, CLOSE());
    i1.setContext(k_full);
    i2.setContext(k_full);
    i3.setContext(k_full);
    i4.setContext(k_full);

    /** @arg every IF form with scalar branches keeps the context length and its values */
    for (size_t i = 0; i < 20; ++i) {
        bool up = closes[i] > opens[i];
        CHECK_UNARY(!std::isnan(i2[i]));
        CHECK_EQ(i2[i], up ? 1.0 : 0.0);
        CHECK_EQ(i3[i], up ? closes[i] : 1.0);
        CHECK_EQ(i4[i], up ? 1.0 : closes[i]);
    }
    CHECK_EQ(i1.size(), 20);

    /** @arg a bare CVAL leaf takes the context length once bound */
    Indicator c1 = CVAL(2.0);
    c1.setContext(k_full);
    CHECK_EQ(c1.size(), 20);
    CHECK_EQ(c1[0], 2.0);

    // CONTEXT and the two-input indicators pad a dateless reference series with CVAL plus an
    // addition; the padding subtree is built without a context of its own, so the binding check
    // does not apply to it and the merged length still matches the context
    Indicator padded = CONTEXT(PRICELIST(px12));
    padded.setContext(k_full);

    /** @arg a dateless context reference keeps the context length, its leading part stays null (the
     *  zero filled CVAL is never written below the discard of the addition) */
    CHECK_EQ(padded.size(), 20);
    CHECK_EQ(padded.discard(), 8);
    CHECK_UNARY(std::isnan(padded[0]));
    CHECK_EQ(padded[8], doctest::Approx(px12[0]));
    CHECK_EQ(padded[19], doctest::Approx(px12[11]));

    // An outer addition skips the invalid leading part of its operands
    Indicator ctx1 = CLOSE() + CONTEXT(PRICELIST(px12));
    ctx1.setContext(k_full);

    /** @arg the addition of a padded reference keeps the context length from its discard on */
    CHECK_EQ(ctx1.size(), 20);
    for (size_t i = ctx1.discard(); i < 20; ++i) {
        CHECK_EQ(ctx1[i], doctest::Approx(closes[i] + px12[i - 8]));
    }

    /** @arg a dateless reference of a two-input indicator keeps the context length as well */
    Indicator corr1 = CORR(CLOSE(), PRICELIST(px12), 5);
    corr1.setContext(k_full);
    CHECK_EQ(corr1.size(), 20);
    Indicator beta1 = BETA(CLOSE(), PRICELIST(px12), 5);
    beta1.setContext(k_full);
    CHECK_EQ(beta1.size(), 20);

    /** @arg an independent context of another stock aligns by date and keeps the context length */
    Indicator cross = CLOSE() + CONTEXT(SMA(CLOSE(), 5), sm.getStock("sh000001"), true);
    cross.setContext(k_full);
    CHECK_EQ(cross.size(), 20);
}

/** @par Test points */
TEST_CASE("test_indicator_increment_shift_length_match") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 20));
    KData k_inner = stock.getKData(KQuery(10, 20));
    KData k_append = stock.getKData(KQuery(0, 30));  // the same start, more bars at the tail
    KData k_move = stock.getKData(KQuery(10, 30));   // shifted front and extended tail

    // CLOSE() is a context leaf, so every node of these trees is laid out as the context and the
    // old results may be shifted in place; the guards must not disable the incremental path
    auto normal_unary = [&]() { return EMA(CLOSE(), 3); };
    auto normal_binary = [&]() { return CLOSE() + OPEN(); };
    auto normal_if = [&]() { return IF(CLOSE(), OPEN(), HIGH()); };
    auto normal_abs = [&]() { return ABS(CLOSE()); };
    auto normal_weave = [&]() { return WEAVE(CLOSE(), OPEN()); };

    /** @arg a context-length unary node still matches the full result on a tail append */
    checkIncrementEqualsFull(normal_unary, k_full, k_append);

    /** @arg a context-length binary node still matches the full result on a tail append */
    checkIncrementEqualsFull(normal_binary, k_full, k_append);

    /** @arg a context-length condition node still matches the full result on a tail append */
    checkIncrementEqualsFull(normal_if, k_full, k_append);

    // Element-wise nodes do not depend on where the recurrence starts, so they are compared under
    // a shifted window too. A recursive one is not: a full recalculation seeds the recurrence at
    // the first bar of the new window while the incremental path continues the previous one, which
    // is a separate semantic issue, not the buffer length premise
    /** @arg an element-wise binary node still matches the full result when the window is shifted
     *  and appended */
    checkIncrementEqualsFull(normal_binary, k_full, k_move);

    /** @arg an element-wise condition node still matches the full result when the window is
     *  shifted and appended */
    checkIncrementEqualsFull(normal_if, k_full, k_move);

    /** @arg the shift path still runs: a recursive node continues its recurrence on a shifted
     *  window instead of being reseeded, so its incremental result legitimately differs from a
     *  fresh full calculation. This guards against a too strict guard silently disabling the
     *  incremental mechanism, which the equality checks above could not detect */
    Indicator shifted = EMA(CLOSE(), 3);
    shifted.setContext(k_full);
    shifted.setContext(k_move);
    Indicator reseeded = EMA(CLOSE(), 3);
    reseeded.setContext(k_move);
    CHECK_EQ(shifted.size(), reseeded.size());
    size_t recalc_points = 0;
    for (size_t i = 0; i < reseeded.size(); ++i) {
        double a = reseeded[i];
        double b = shifted[i];
        if (std::isnan(a) && std::isnan(b)) {
            continue;
        }
        if (!(b == doctest::Approx(a).epsilon(0.0001))) {
            ++recalc_points;
        }
    }
    CHECK_MESSAGE(recalc_points > 0,
                  "the incremental shift path did not run, every point was recalculated");

    // The renew path (can_inner_calculate) never recalculates the children, so it is only asserted
    // with an element-wise node
    /** @arg an element-wise node still matches the full result when the context shrinks into the
     *  old window (renew fast path) */
    checkIncrementEqualsFull(normal_abs, k_full, k_inner);

    /** @arg an aligned weave node still matches the full result on a tail append */
    checkIncrementEqualsFull(normal_weave, k_full, k_append);
}

/** @par Test points */
TEST_CASE("test_indicator_is_same") {
    PriceList d;
    for (size_t i = 0; i < 5; ++i) {
        d.push_back(i + 1);
    }
    Indicator a = PRICELIST(d);

    /** @arg an indicator is the same as itself */
    CHECK_UNARY(a.isSame(a));

    /** @arg a copy shares the underlying imp, so it is the same instance */
    Indicator copy = a;
    CHECK_UNARY(a.isSame(copy));

    /** @arg two distinct instances with the same formula are not the same instance */
    Indicator another = PRICELIST(d);
    CHECK_UNARY(!a.isSame(another));

    /** @arg a null-imp indicator is never the same instance */
    IndicatorImpPtr null_imp;
    Indicator empty(null_imp);
    CHECK_UNARY(!empty.isSame(a));
    CHECK_UNARY(!a.isSame(empty));
    CHECK_UNARY(!empty.isSame(empty));
}

/** @} */
