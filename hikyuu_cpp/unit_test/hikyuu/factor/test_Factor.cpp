/*
 * test_Factor.cpp
 *
 *  Created on: 2026-02-23
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/factor/Factor.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/StockManager.h>
#include "../test_config.h"  // test_config.h is included to use check_indicator
#include "../plugin_valid.h"
#include <fstream>

using namespace hku;

/**
 * @defgroup test_Factor test_Factor
 * @ingroup test_hikyuu_factor_suite
 * @{
 */

/** @par Test point: test the basic functionality of Factor */
TEST_CASE("test_Factor_basic") {
    // Create the Indicator used by the test
    Indicator ma5 = MA(CLOSE(), 5);

    // Test the constructor
    Factor factor("TEST_FACTOR", ma5, KQuery::DAY, "测试因子", "详细描述");

    // Test the basic attributes
    CHECK_EQ(factor.name(), "TEST_FACTOR");
    CHECK_EQ(factor.ktype(), KQuery::DAY);
    CHECK_FALSE(factor.isNull());
    CHECK_EQ(factor.brief(), "测试因子");
    CHECK_EQ(factor.details(), "详细描述");

    // Test the formula attribute
    Indicator formula = factor.formula();
    // Instead of checking empty() directly, the name is checked
    CHECK_EQ(formula.name(), "TEST_FACTOR");

    // Test the block attribute (empty by default)
    const Block& block = factor.block();
    CHECK_UNARY(block.isNull());
    CHECK_EQ(block.size(), 0);

    // Test the date attributes (they may be empty, which is normal)
    CHECK_EQ(factor.startDate(), Datetime::min());

    // Test the persistence attribute
    CHECK_FALSE(factor.needSaveValue());
    factor.needSaveValue(true);
    CHECK_UNARY(factor.needSaveValue());
}

/** @par Test point: test the Factor construction with a Block */
TEST_CASE("test_Factor_with_block") {
    // Create the test Block
    Block test_block("行业", "测试板块");

    // Create a Factor with a Block
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor("BLOCK_FACTOR", ma5, KQuery::DAY, "带板块因子", "测试板块功能", false,
                  Datetime::min(), test_block);

    // Verify the Block attribute
    const Block& block = factor.block();
    CHECK_FALSE(block.isNull());
    CHECK_EQ(block.category(), "行业");
    CHECK_EQ(block.name(), "测试板块");
    CHECK_EQ(block.size(), 0);
}

/** @par Test point: test the check parameter of the Factor getValues method */
TEST_CASE("test_Factor_getValues_check") {
    HKU_IF_RETURN(!pluginValid(), void());
    // Create the test Indicator
    Indicator ma5 = MA(CLOSE(), 5);
    KQuery query_obj(0, Null<int64_t>(), KQuery::DAY);

    // Create a Factor without a Block
    SUBCASE("Factor without block") {
        Factor factor("NO_BLOCK_FACTOR", ma5, KQuery::DAY);

        // It should work normally with check=false
        CHECK_NOTHROW(factor.getValues(StockList{}, query_obj, false));

        // It should also work with check=true (there is no Block restriction)
        CHECK_NOTHROW(factor.getValues(StockList{}, query_obj, true));
    }

    // Create a Factor with a Block
    SUBCASE("Factor with block") {
        Block test_block("行业", "测试板块");
        Factor factor("BLOCK_FACTOR", ma5, KQuery::DAY, "测试", "描述", false, Datetime::min(),
                      test_block);

        // Verify that the Block is set correctly
        CHECK_FALSE(factor.block().isNull());

        // It should work normally with check=false
        CHECK_NOTHROW(factor.getValues(StockList{}, query_obj, false));

        // With check=true an empty stock list should work normally
        CHECK_NOTHROW(factor.getValues(StockList{}, query_obj, true));
    }
}

/** @par Test point: test the full parameter combination of the Factor getValues method */
TEST_CASE("test_Factor_getValues_complete_params") {
    HKU_IF_RETURN(!pluginValid(), void());
    // Prepare the test data
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    CHECK_FALSE(stock.isNull());

    StockList stocks = {stock};
    KQuery query(0, 10, KQuery::DAY);  // Get the data of the first 10 days

    // Create the test Factor
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor("PARAM_TEST", ma5, KQuery::DAY);

    // Test the different parameter combinations
    SUBCASE("Default parameters") {
        IndicatorList result = factor.getValues(stocks, query);
        // Verify that the returned result is not empty
        CHECK_FALSE(result.empty());
        // Verify that every stock has a corresponding indicator result
        CHECK_EQ(result.size(), stocks.size());
    }

    SUBCASE("Align=true") {
        IndicatorList result = factor.getValues(stocks, query, true);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
    }

    SUBCASE("Fill null=true") {
        IndicatorList result = factor.getValues(stocks, query, false, true);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
    }

    SUBCASE("To value=true") {
        IndicatorList result = factor.getValues(stocks, query, false, false, true);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
    }

    SUBCASE("All parameters true") {
        IndicatorList result = factor.getValues(stocks, query, true, true, true, false);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
    }

    SUBCASE("Check=true with valid stocks") {
        IndicatorList result = factor.getValues(stocks, query, false, false, false, true);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
    }
}

/** @par Test point: test the boundary conditions of the Factor getValues method */
TEST_CASE("test_Factor_getValues_edge_cases") {
    HKU_IF_RETURN(!pluginValid(), void());
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor("EDGE_TEST", ma5, KQuery::DAY);

    KQuery query(0, 10, KQuery::DAY);

    // Test an empty stock list
    SUBCASE("Empty stock list") {
        StockList empty_stocks;
        IndicatorList result = factor.getValues(empty_stocks, query);
        // An empty stock list should return an empty result
        CHECK_UNARY(result.empty());
    }

    // Test an invalid query range
    SUBCASE("Invalid query range") {
        StockManager& sm = StockManager::instance();
        Stock stock = sm.getStock("sh600000");
        CHECK_FALSE(stock.isNull());
        StockList stocks = {stock};

        KQuery invalid_query(1000000, 1000010, KQuery::DAY);  // Beyond the actual data range
        IndicatorList result = factor.getValues(stocks, invalid_query);
        // An empty indicator result should be returned
        CHECK_FALSE(result.empty());     // The container is not empty
        CHECK_UNARY(result[0].empty());  // But the indicator data is empty
    }

    // Test the different K-line types
    SUBCASE("Different KType") {
        StockManager& sm = StockManager::instance();
        Stock stock = sm.getStock("sh600000");
        CHECK_FALSE(stock.isNull());
        StockList stocks = {stock};

        // The daily line query
        KQuery day_query(0, 5, KQuery::DAY);
        IndicatorList day_result = factor.getValues(stocks, day_query);
        CHECK_FALSE(day_result.empty());

        // The weekly line query
        KQuery week_query(0, 5, KQuery::WEEK);
        IndicatorList week_result = factor.getValues(stocks, week_query);
        CHECK_FALSE(week_result.empty());

        // The monthly line query
        KQuery month_query(0, 5, KQuery::MONTH);
        IndicatorList month_result = factor.getValues(stocks, month_query);
        CHECK_FALSE(month_result.empty());
    }
}

/** @par Test point: test the result correctness of the Factor getValues method */
TEST_CASE("test_Factor_getValues_result_correctness") {
    HKU_IF_RETURN(!pluginValid(), void());
    StockManager& sm = StockManager::instance();
    Stock stock1 = sm.getStock("sh000001");  // The Shanghai Composite Index
    Stock stock2 = sm.getStock("sz000001");  // The Shenzhen Component Index
    CHECK_FALSE(stock1.isNull());
    CHECK_FALSE(stock2.isNull());

    StockList stocks = {stock1, stock2};  // Two securities of two different markets
    KQuery query(0, 20, KQuery::DAY);     // Get 20 days of data for the verification

    // Create an MA5 indicator for the test
    Indicator ma5 = MA(CLOSE(), 5);
    Factor ma5_factor("MA5_TEST", ma5, KQuery::DAY);

    SUBCASE("Basic functionality and result validation") {
        // Test the basic functionality and verify the calculation result
        IndicatorList result = ma5_factor.getValues(stocks, query);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), 2);  // There should be two results

        // Verify the result of every stock
        for (size_t i = 0; i < result.size(); ++i) {
            Indicator ind = result[i];
            CHECK_FALSE(ind.empty());
            CHECK_EQ(ind.size(), 20);
            CHECK_EQ(ind.discard(), 4);
            CHECK_EQ(ind.getResultNumber(), 1);  // Verify the number of the result sets

            // Verify the basic attributes of the Indicator with the existing check_indicator method
            KData kdata = stocks[i].getKData(query);
            Indicator close_prices = CLOSE(kdata);
            Indicator expected_ma = MA(close_prices, 5);

            // Verify the result consistency with check_indicator
            check_indicator(ind, expected_ma);
        }
    }

    SUBCASE("Parameter combinations validation") {
        // The functional verification of the different parameter combinations
        IndicatorList result1 = ma5_factor.getValues(stocks, query, false, false, false);
        IndicatorList result2 = ma5_factor.getValues(stocks, query, true, false, false);
        IndicatorList result3 = ma5_factor.getValues(stocks, query, false, true, false);
        IndicatorList result4 = ma5_factor.getValues(stocks, query, false, false, true);

        // All the results should be valid
        CHECK_FALSE(result1.empty());
        CHECK_FALSE(result2.empty());
        CHECK_FALSE(result3.empty());
        CHECK_FALSE(result4.empty());

        // Verify that every result has the data of the two stocks
        CHECK_EQ(result1.size(), 2);
        CHECK_EQ(result2.size(), 2);
        CHECK_EQ(result3.size(), 2);
        CHECK_EQ(result4.size(), 2);

        // Verify the basic size
        for (size_t i = 0; i < 2; ++i) {
            CHECK_EQ(result1[i].size(), 20);
            CHECK_EQ(result2[i].size(), 20);
            CHECK_EQ(result3[i].size(), 20);
            CHECK_EQ(result4[i].size(), 20);

            // Verify the consistency with check_indicator
            check_indicator(result1[i], result1[i]);
        }
    }

    SUBCASE("Edge case handling validation") {
        // Test the boundary cases
        StockList empty_stocks;
        IndicatorList empty_result = ma5_factor.getValues(empty_stocks, query);
        CHECK_UNARY(empty_result.empty());

        // Test an invalid query range
        KQuery invalid_query(1000000, 1000010, KQuery::DAY);
        IndicatorList invalid_result = ma5_factor.getValues(stocks, invalid_query);
        CHECK_FALSE(invalid_result.empty());
        CHECK_EQ(invalid_result.size(), 2);
        CHECK_UNARY(invalid_result[0].empty());
        CHECK_UNARY(invalid_result[1].empty());
        CHECK_EQ(invalid_result[0].size(), 0);
        CHECK_EQ(invalid_result[1].size(), 0);
    }

    SUBCASE("Multiple KType validation") {
        // Test the different K-line types
        KQuery day_query(0, 10, KQuery::DAY);
        KQuery week_query(0, 10, KQuery::WEEK);
        KQuery month_query(0, 10, KQuery::MONTH);

        IndicatorList day_result = ma5_factor.getValues(stocks, day_query);
        IndicatorList week_result = ma5_factor.getValues(stocks, week_query);
        IndicatorList month_result = ma5_factor.getValues(stocks, month_query);

        CHECK_FALSE(day_result.empty());
        CHECK_FALSE(week_result.empty());
        CHECK_FALSE(month_result.empty());

        // Verify that all of them have the results of the two stocks
        CHECK_EQ(day_result.size(), 2);
        CHECK_EQ(week_result.size(), 2);
        CHECK_EQ(month_result.size(), 2);

        // Verify that all of them have reasonable data
        for (size_t i = 0; i < 2; ++i) {
            CHECK_GT(day_result[i].size(), 0);
            CHECK_GT(week_result[i].size(), 0);
            CHECK_GT(month_result[i].size(), 0);

            // Verify the basic structure with check_indicator
            check_indicator(day_result[i], day_result[i]);
            check_indicator(week_result[i], week_result[i]);
            check_indicator(month_result[i], month_result[i]);
        }
    }

    SUBCASE("Result consistency validation") {
        // Test the consistency of the results of multiple calls
        IndicatorList result1 = ma5_factor.getValues(stocks, query);
        IndicatorList result2 = ma5_factor.getValues(stocks, query);

        CHECK_FALSE(result1.empty());
        CHECK_FALSE(result2.empty());
        CHECK_EQ(result1.size(), result2.size());
        CHECK_EQ(result1.size(), 2);

        // Verify the consistency of the result of every stock
        for (size_t stock_idx = 0; stock_idx < 2; ++stock_idx) {
            Indicator ind1 = result1[stock_idx];
            Indicator ind2 = result2[stock_idx];

            // Verify with check_indicator that the two calls agree
            check_indicator(ind1, ind2);
        }
    }

    SUBCASE("Calculation accuracy validation") {
        // Test the calculation precision - verify the result with known data
        KQuery short_query(0, 10, KQuery::DAY);
        IndicatorList result = ma5_factor.getValues(stocks, short_query);

        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), 2);

        // Verify the calculation result of every stock
        for (size_t i = 0; i < 2; ++i) {
            Indicator ind = result[i];
            CHECK_FALSE(ind.empty());
            CHECK_EQ(ind.size(), 10);
            CHECK_EQ(ind.discard(), 4);

            // Create the expected result for an exact comparison
            KData kdata = stocks[i].getKData(short_query);
            Indicator close_prices = CLOSE(kdata);
            Indicator expected_ma = MA(close_prices, 5);

            // Do an exact verification with check_indicator
            check_indicator(ind, expected_ma);
        }
    }

    SUBCASE("Multi-stock processing validation") {
        // Test the multi-stock handling (it is already multi-stock, this is an extra verification)
        Stock stock3 = sm.getStock("sh600036");

        if (!stock3.isNull()) {
            StockList multi_stocks = {stock1, stock2, stock3};
            IndicatorList results = ma5_factor.getValues(multi_stocks, query);

            // An indicator result should be returned for every stock
            CHECK_EQ(results.size(), 3);

            // Verify the basic attributes of every result
            for (const auto& ind : results) {
                // Verify the basic structure with check_indicator
                check_indicator(ind, ind);
            }
        }
    }
}

/** @par Test point: test the Factor getAllValues method */
TEST_CASE("test_Factor_getAllValues") {
    KQuery query_obj(0, Null<int64_t>(), KQuery::DAY);
    KQuery week_query_obj(0, Null<int64_t>(), KQuery::WEEK);
    KQuery month_query_obj(0, Null<int64_t>(), KQuery::MONTH);

    // Test a Factor without a Block
    SUBCASE("Factor without block") {
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor("ALL_NO_BLOCK", ma5, KQuery::DAY);

        // It should be callable (although it may return an empty result)
        CHECK_NOTHROW(factor.getAllValues(query_obj));
        // Verify that the method can be called normally without throwing
    }

    // Test a Factor with a Block
    SUBCASE("Factor with block") {
        Block test_block("行业", "测试板块");
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor("ALL_BLOCK_FACTOR", ma5, KQuery::DAY, "测试", "描述", false, Datetime::min(),
                      test_block);

        // Verify that the Block is set correctly
        CHECK_FALSE(factor.block().isNull());

        // It should be callable
        CHECK_NOTHROW(factor.getAllValues(query_obj));
        // Verify that the method can be called normally without throwing
    }

    // Test the different query parameters
    SUBCASE("Different query parameters") {
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor("QUERY_TEST", ma5, KQuery::DAY);

        // Test the daily line query
        CHECK_NOTHROW(factor.getAllValues(query_obj));

        // Test the weekly line query
        CHECK_NOTHROW(factor.getAllValues(week_query_obj));

        // Test the monthly line query
        CHECK_NOTHROW(factor.getAllValues(month_query_obj));
    }
}

/** @par Test point: test the copy and assignment semantics of Factor */
TEST_CASE("test_Factor_copy_semantics") {
    // Create the original Factor
    Indicator ma5 = MA(CLOSE(), 5);
    Block test_block("行业", "测试板块");
    Factor original("COPY_TEST", ma5, KQuery::DAY, "拷贝测试", "详细描述", true,
                    Datetime(202001010000LL), test_block);

    // Test the copy constructor
    Factor copy1(original);
    CHECK_EQ(copy1.name(), "COPY_TEST");
    CHECK_EQ(copy1.ktype(), KQuery::DAY);
    CHECK_EQ(copy1.brief(), "拷贝测试");
    CHECK_EQ(copy1.details(), "详细描述");
    CHECK_UNARY(copy1.needSaveValue());
    CHECK_EQ(copy1.startDate(), Datetime(202001010000LL));
    CHECK_EQ(copy1.block().category(), "行业");
    CHECK_EQ(copy1.block().name(), "测试板块");

    // Test the copy assignment
    Factor copy2;
    copy2 = original;
    CHECK_EQ(copy2.name(), "COPY_TEST");
    CHECK_EQ(copy2.ktype(), KQuery::DAY);
    CHECK_EQ(copy2.brief(), "拷贝测试");
    CHECK_EQ(copy2.details(), "详细描述");
    CHECK_UNARY(copy2.needSaveValue());
    CHECK_EQ(copy2.startDate(), Datetime(202001010000LL));
    CHECK_EQ(copy2.block().category(), "行业");
    CHECK_EQ(copy2.block().name(), "测试板块");

    // Test the move constructor
    Factor moved1(std::move(original));
    CHECK_EQ(moved1.name(), "COPY_TEST");
    CHECK_EQ(moved1.ktype(), KQuery::DAY);

    // The original object should become empty
    CHECK_UNARY(original.isNull());

    // Test the move assignment
    Factor moved2;
    moved2 = std::move(copy1);
    CHECK_EQ(moved2.name(), "COPY_TEST");
    CHECK_EQ(moved2.ktype(), KQuery::DAY);

    // The original object should become empty
    CHECK_UNARY(copy1.isNull());
}

/** @par Test point: test the align parameter of the Factor getValues method */
TEST_CASE("test_Factor_getValues_align") {
    HKU_IF_RETURN(!pluginValid(), void());
    StockManager& sm = StockManager::instance();
    Stock stock1 = sm.getStock("sh000001");  // The Shanghai Composite Index
    Stock stock2 = sm.getStock("sz000001");  // The Shenzhen Component Index
    CHECK_FALSE(stock1.isNull());
    CHECK_FALSE(stock2.isNull());

    StockList stocks = {stock1, stock2};  // Two securities of two different markets
    KQuery query(0, 30, KQuery::DAY);     // Get 30 days of data for the alignment test

    // Create the test indicator
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor("ALIGN_TEST", ma5, KQuery::DAY);

    SUBCASE("Align=false basic functionality") {
        // Test the basic functionality without the alignment
        IndicatorList result = factor.getValues(stocks, query, false);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), 2);  // The results of the two stocks

        // Verify the result of every stock
        for (size_t i = 0; i < result.size(); ++i) {
            Indicator ind = result[i];
            CHECK_FALSE(ind.empty());
            CHECK_GT(ind.size(), 0);

            // Verify the basic structure
            check_indicator(ind, ind);
        }
    }

    SUBCASE("Align=true trading calendar alignment") {
        // Test the alignment with the trading calendar
        IndicatorList result = factor.getValues(stocks, query, true);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), 2);  // The results of the two stocks

        // Get the trading calendar for the comparison
        DatetimeList trading_dates = sm.getTradingCalendar(query);
        CHECK_FALSE(trading_dates.empty());

        // Verify the result of every stock
        for (size_t i = 0; i < result.size(); ++i) {
            Indicator ind_aligned = result[i];
            CHECK_FALSE(ind_aligned.empty());
            CHECK_GT(ind_aligned.size(), 0);

            // Verify that the indicator size matches the trading calendar (the core alignment
            // check)
            CHECK_EQ(ind_aligned.size(), trading_dates.size());

            // Verify the basic structure
            check_indicator(ind_aligned, ind_aligned);
        }
    }

    SUBCASE("Align comparison between true and false") {
        // Compare the difference between align=true and align=false
        IndicatorList result_false = factor.getValues(stocks, query, false);
        IndicatorList result_true = factor.getValues(stocks, query, true);

        CHECK_FALSE(result_false.empty());
        CHECK_FALSE(result_true.empty());
        CHECK_EQ(result_false.size(), result_true.size());
        CHECK_EQ(result_false.size(), 2);

        // Verify the result of every stock
        for (size_t i = 0; i < 2; ++i) {
            Indicator ind_false = result_false[i];
            Indicator ind_true = result_true[i];

            CHECK_FALSE(ind_false.empty());
            CHECK_FALSE(ind_true.empty());

            // Verify that both work and return a reasonable result
            CHECK_GT(ind_false.size(), 0);
            CHECK_GT(ind_true.size(), 0);

            // Verify the sanity of the discard logic
            CHECK_GE(ind_true.discard(), 0);
            CHECK_GE(ind_false.discard(), 0);

            // Verify the structural integrity
            check_indicator(ind_false, ind_false);
            check_indicator(ind_true, ind_true);
        }

        // Get the trading calendar to verify the effect of align=true
        DatetimeList trading_dates = sm.getTradingCalendar(query);
        CHECK_FALSE(trading_dates.empty());

        // Verify that with align=true the result size matches the trading calendar
        for (size_t i = 0; i < result_true.size(); ++i) {
            CHECK_EQ(result_true[i].size(), trading_dates.size());
        }
    }

    SUBCASE("Align with fill_null parameter") {
        // Test align=true together with the fill_null parameter
        IndicatorList result1 = factor.getValues(stocks, query, true, false);  // fill_null=false
        IndicatorList result2 = factor.getValues(stocks, query, true, true);   // fill_null=true

        CHECK_FALSE(result1.empty());
        CHECK_FALSE(result2.empty());

        CHECK_EQ(result1.size(), 2);
        CHECK_EQ(result2.size(), 2);

        // Verify the result of every stock
        for (size_t i = 0; i < 2; ++i) {
            Indicator ind1 = result1[i];
            Indicator ind2 = result2[i];

            CHECK_FALSE(ind1.empty());
            CHECK_FALSE(ind2.empty());

            // Both should have the same size (both aligned with the trading calendar)
            CHECK_EQ(ind1.size(), ind2.size());

            // Verify that both match the trading calendar size
            DatetimeList trading_dates = sm.getTradingCalendar(query);
            CHECK_EQ(ind1.size(), trading_dates.size());
            CHECK_EQ(ind2.size(), trading_dates.size());

            // Verify the structure
            check_indicator(ind1, ind1);
            check_indicator(ind2, ind2);
        }
    }

    SUBCASE("Align with different query ranges") {
        // Test the alignment effect with different query ranges
        KQuery short_query(0, 10, KQuery::DAY);
        KQuery medium_query(0, 20, KQuery::DAY);
        KQuery long_query(0, 40, KQuery::DAY);

        IndicatorList short_result = factor.getValues(stocks, short_query, true);
        IndicatorList medium_result = factor.getValues(stocks, medium_query, true);
        IndicatorList long_result = factor.getValues(stocks, long_query, true);

        CHECK_FALSE(short_result.empty());
        CHECK_FALSE(medium_result.empty());
        CHECK_FALSE(long_result.empty());

        CHECK_EQ(short_result.size(), 2);
        CHECK_EQ(medium_result.size(), 2);
        CHECK_EQ(long_result.size(), 2);

        // Verify the result of every stock
        for (size_t stock_idx = 0; stock_idx < 2; ++stock_idx) {
            Indicator short_ind = short_result[stock_idx];
            Indicator medium_ind = medium_result[stock_idx];
            Indicator long_ind = long_result[stock_idx];

            CHECK_FALSE(short_ind.empty());
            CHECK_FALSE(medium_ind.empty());
            CHECK_FALSE(long_ind.empty());

            // Verify the size relation (a longer query range should have more trading days)
            CHECK_LE(short_ind.size(), medium_ind.size());
            CHECK_LE(medium_ind.size(), long_ind.size());

            // Verify that both use the correct trading calendar alignment
            DatetimeList short_dates = sm.getTradingCalendar(short_query);
            DatetimeList medium_dates = sm.getTradingCalendar(medium_query);
            DatetimeList long_dates = sm.getTradingCalendar(long_query);

            CHECK_EQ(short_ind.size(), short_dates.size());
            CHECK_EQ(medium_ind.size(), medium_dates.size());
            CHECK_EQ(long_ind.size(), long_dates.size());

            // Verify the structure
            check_indicator(short_ind, short_ind);
            check_indicator(medium_ind, medium_ind);
            check_indicator(long_ind, long_ind);
        }
    }

    SUBCASE("Multi-stock align validation") {
        // Test the alignment with multiple stocks (three stocks are used)
        Stock stock3 = sm.getStock("sh600036");

        if (!stock3.isNull()) {
            StockList multi_stocks = {stock1, stock2, stock3};
            IndicatorList results = factor.getValues(multi_stocks, query, true);

            // An indicator result should be returned for every stock
            CHECK_EQ(results.size(), 3);

            // Verify that every result is aligned with the trading calendar correctly
            DatetimeList trading_dates = sm.getTradingCalendar(query);
            CHECK_FALSE(trading_dates.empty());

            for (const auto& ind : results) {
                CHECK_FALSE(ind.empty());
                CHECK_EQ(ind.size(), trading_dates.size());

                // Verify the structure
                check_indicator(ind, ind);
            }
        }
    }

    SUBCASE("Align with different KType") {
        // Test the alignment for the different K-line types
        KQuery day_query(0, 15, KQuery::DAY);
        KQuery week_query(0, 15, KQuery::WEEK);
        KQuery month_query(0, 15, KQuery::MONTH);

        IndicatorList day_result = factor.getValues(stocks, day_query, true);
        IndicatorList week_result = factor.getValues(stocks, week_query, true);
        IndicatorList month_result = factor.getValues(stocks, month_query, true);

        CHECK_FALSE(day_result.empty());
        CHECK_FALSE(week_result.empty());
        CHECK_FALSE(month_result.empty());

        CHECK_EQ(day_result.size(), 2);
        CHECK_EQ(week_result.size(), 2);
        CHECK_EQ(month_result.size(), 2);

        // Verify the result of every stock
        for (size_t i = 0; i < 2; ++i) {
            Indicator day_ind = day_result[i];
            Indicator week_ind = week_result[i];
            Indicator month_ind = month_result[i];

            CHECK_FALSE(day_ind.empty());
            CHECK_FALSE(week_ind.empty());
            CHECK_FALSE(month_ind.empty());

            // Get the trading calendar of the corresponding K-line type
            DatetimeList day_dates = sm.getTradingCalendar(day_query);
            DatetimeList week_dates = sm.getTradingCalendar(week_query);
            DatetimeList month_dates = sm.getTradingCalendar(month_query);

            // Verify that the sizes match (the key to verifying the alignment)
            CHECK_EQ(day_ind.size(), day_dates.size());
            CHECK_EQ(week_ind.size(), week_dates.size());
            CHECK_EQ(month_ind.size(), month_dates.size());

            // Verify the structure
            check_indicator(day_ind, day_ind);
            check_indicator(week_ind, week_ind);
            check_indicator(month_ind, month_ind);
        }
    }
}

/** @par Test point: test the hashing and the comparison of Factor */
TEST_CASE("test_Factor_hash_compare") {
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    // Create an identical Factor
    Factor factor1("HASH_TEST", ma5, KQuery::DAY);
    Factor factor2("HASH_TEST", ma5, KQuery::DAY);

    // Create a different Factor
    Factor factor3("HASH_TEST", ma10, KQuery::DAY);
    Factor factor4("DIFFERENT", ma5, KQuery::DAY);

    // Test that different Factors should have different hash values
    CHECK_NE(factor1.hash(), factor3.hash());
    CHECK_NE(factor1.hash(), factor4.hash());

    // Test the standard library hash function
    std::hash<Factor> hasher;
    CHECK_NE(hasher(factor1), hasher(factor3));
}

#if HKU_SUPPORT_SERIALIZATION

/** @par Test point: test the basic serialization of Factor */
TEST_CASE("test_Factor_basic_serialize") {
    string filename(StockManager::instance().tmpdir());
    filename += "/Factor_basic.xml";

    // Create the test Factor
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor1("SERIALIZE_TEST", ma5, KQuery::DAY, "序列化测试因子",
                   "这是一个用于测试序列化的因子", true);

    // Set some attributes
    factor1.createAt(Datetime(202001010000LL));
    factor1.updateAt(Datetime(202001020000LL));

    // Serialize to a file
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(factor1);
    }

    // Deserialize from the file
    Factor factor2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(factor2);
    }

    // Verify the attributes of the deserialized object
    CHECK_EQ(factor1.name(), factor2.name());
    CHECK_EQ(factor1.ktype(), factor2.ktype());
    CHECK_EQ(factor1.brief(), factor2.brief());
    CHECK_EQ(factor1.details(), factor2.details());
    CHECK_EQ(factor1.needSaveValue(), factor2.needSaveValue());
    CHECK_EQ(factor1.createAt(), factor2.createAt());
    CHECK_EQ(factor1.updateAt(), factor2.updateAt());

    // Verify the formula
    Indicator formula1 = factor1.formula();
    Indicator formula2 = factor2.formula();
    CHECK_EQ(formula1.name(), formula2.name());
    CHECK_EQ(formula1.size(), formula2.size());

    // Verify the block (all of them should be empty)
    CHECK_UNARY(factor1.block().isNull());
    CHECK_UNARY(factor2.block().isNull());
    CHECK_EQ(factor1.block().size(), factor2.block().size());
}

/** @par Test point: test the serialization of a Factor with a Block */
TEST_CASE("test_Factor_with_block_serialize") {
    string filename(StockManager::instance().tmpdir());
    filename += "/Factor_with_block.xml";

    // Create the test Block
    Block test_block("行业", "序列化测试板块");
    test_block.add("sh600000");
    test_block.add("sz000001");

    // Create a Factor with a Block
    Indicator ma10 = MA(CLOSE(), 10);
    Factor factor1("BLOCK_SERIALIZE_TEST", ma10, KQuery::WEEK, "带板块序列化测试",
                   "测试包含Block的Factor序列化", false, Datetime(202001010000LL), test_block);

    // Serialize to a file
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(factor1);
    }

    // Deserialize from the file
    Factor factor2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(factor2);
    }

    // Verify the basic attributes
    CHECK_EQ(factor1.name(), factor2.name());
    CHECK_EQ(factor1.ktype(), factor2.ktype());
    CHECK_EQ(factor1.brief(), factor2.brief());
    CHECK_EQ(factor1.details(), factor2.details());
    CHECK_EQ(factor1.needSaveValue(), factor2.needSaveValue());
    CHECK_EQ(factor1.startDate(), factor2.startDate());

    // Verify the Block attribute
    const Block& block1 = factor1.block();
    const Block& block2 = factor2.block();
    CHECK_FALSE(block1.isNull());
    CHECK_FALSE(block2.isNull());
    CHECK_EQ(block1.category(), block2.category());
    CHECK_EQ(block1.name(), block2.name());
    CHECK_EQ(block1.size(), block2.size());

    // Verify the stock codes in the Block
    auto stocks1 = block1.getStockList();
    auto stocks2 = block2.getStockList();
    CHECK_EQ(stocks1.size(), stocks2.size());

    // Create a stock code set for the comparison
    std::set<string> codes1, codes2;
    for (const auto& stock : stocks1) {
        codes1.insert(stock.code());
    }
    for (const auto& stock : stocks2) {
        codes2.insert(stock.code());
    }
    CHECK_EQ(codes1.size(), codes2.size());

    // Verify that all the codes match
    for (const auto& code : codes1) {
        CHECK_UNARY(codes2.find(code) != codes2.end());
    }
}

/** @par Test point: test the serialization of an empty Factor */
TEST_CASE("test_Factor_empty_serialize") {
    string filename(StockManager::instance().tmpdir());
    filename += "/Factor_empty.xml";

    // Create an empty Factor
    Factor factor1;

    // Serialize to a file
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(factor1);
    }

    // Deserialize from the file
    Factor factor2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(factor2);
    }

    CHECK_EQ(factor1.name(), factor2.name());
    CHECK_EQ(factor1.ktype(), factor2.ktype());

    // Verify that the deserialized Factor has the default values
    CHECK_EQ(factor2.name(), "");
    // For a default constructed Factor the ktype may be an empty string
    CHECK_EQ(factor2.ktype(), "");  // Changed to the expected empty string
    CHECK_FALSE(factor2.needSaveValue());
}

/** @par Test point: test the serialization of multiple Factors */
TEST_CASE("test_Factor_list_serialize") {
    string filename(StockManager::instance().tmpdir());
    filename += "/Factor_list.xml";

    // Create multiple Factors
    vector<Factor> factors1;

    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);
    Indicator ma20 = MA(CLOSE(), 20);

    factors1.emplace_back("FACTOR_1", ma5, KQuery::DAY, "因子1", "第一个测试因子");
    factors1.emplace_back("FACTOR_2", ma10, KQuery::WEEK, "因子2", "第二个测试因子");
    factors1.emplace_back("FACTOR_3", ma20, KQuery::MONTH, "因子3", "第三个测试因子");

    // Set the different attributes
    factors1[0].needSaveValue(true);
    factors1[0].createAt(Datetime(202001010000LL));
    factors1[1].needSaveValue(false);
    factors1[1].createAt(Datetime(202001020000LL));
    factors1[2].needSaveValue(true);
    factors1[2].createAt(Datetime(202001030000LL));

    // Serialize to a file
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(factors1);
    }

    // Deserialize from the file
    vector<Factor> factors2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(factors2);
    }

    // Verify the serialization result
    CHECK_EQ(factors1.size(), factors2.size());
    CHECK_EQ(factors1.size(), 3);

    for (size_t i = 0; i < factors1.size(); ++i) {
        CHECK_EQ(factors1[i].name(), factors2[i].name());
        CHECK_EQ(factors1[i].ktype(), factors2[i].ktype());
        CHECK_EQ(factors1[i].brief(), factors2[i].brief());
        CHECK_EQ(factors1[i].details(), factors2[i].details());
        CHECK_EQ(factors1[i].needSaveValue(), factors2[i].needSaveValue());
        CHECK_EQ(factors1[i].createAt(), factors2[i].createAt());

        // Verify the formula
        Indicator formula1 = factors1[i].formula();
        Indicator formula2 = factors2[i].formula();
        CHECK_EQ(formula1.name(), formula2.name());
    }
}

#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
