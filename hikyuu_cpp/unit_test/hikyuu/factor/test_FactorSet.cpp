/*
 * test_FactorSet.cpp
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <atomic>
#include <hikyuu/factor/FactorSet.h>
#include <hikyuu/factor/Factor.h>
#include <hikyuu/factor/imp/CompiledFactorPlan.h>
#include <hikyuu/indicator/IndicatorImp.h>
#include <hikyuu/indicator/crt/ALIGN.h>
#include <hikyuu/indicator/crt/CONTEXT.h>
#include <hikyuu/indicator/crt/CORR.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/indicator/crt/FACTOR.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/REF.h>
#include <hikyuu/indicator/crt/STDEV.h>
#include <hikyuu/StockManager.h>
#include "../test_config.h"
#include "../plugin_valid.h"
#include <algorithm>
#include <utility>  // for std::pair
#include <string>   // for std::to_string

using namespace hku;

namespace {

class PythonLikeIndicatorImp : public IndicatorImp {
public:
    PythonLikeIndicatorImp() : IndicatorImp("PYTHON_LIKE") {
        m_is_python_object = true;
    }

    IndicatorImpPtr _clone() override {
        return make_shared<PythonLikeIndicatorImp>();
    }
};

std::atomic<size_t> g_counting_indicator_calculations{0};
std::atomic<size_t> g_counting_indicator_clones{0};

class CountingContextIndicatorImp : public IndicatorImp {
public:
    CountingContextIndicatorImp() : IndicatorImp("COUNTING_CONTEXT") {
        m_need_context = true;
    }

    void _calculate(const Indicator&) override {
        const KData& kdata = getContext();
        HKU_IF_RETURN(kdata.empty(), void());
        ++g_counting_indicator_calculations;
        _readyBuffer(kdata.size(), 1);
        m_discard = 0;
        auto* dst = data();
        for (size_t i = 0; i < kdata.size(); ++i) {
            dst[i] = static_cast<price_t>(i);
        }
    }

    IndicatorImpPtr _clone() override {
        ++g_counting_indicator_clones;
        return make_shared<CountingContextIndicatorImp>();
    }
};

}  // namespace

/**
 * @defgroup test_FactorSet test_FactorSet
 * @ingroup test_hikyuu_factor_suite
 * @{
 */

/** @par Test point: test the basic functionality of FactorSet */
TEST_CASE("test_FactorSet_basic") {
    // Create the Factor object used by the test
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "测试5日均线");
    Factor factor2("MA10", ma10, KQuery::DAY, "10日均线因子", "测试10日均线");

    // Test the constructor and the basic attributes
    FactorSet fs("TestFactorSet");
    CHECK_EQ(fs.name(), "TESTFACTORSET");

    // Test the empty state
    CHECK_EQ(fs.size(), 0);
    CHECK_UNARY(fs.empty());

    // Add the factor
    fs.add(factor1);
    fs.add(factor2);

    CHECK_EQ(fs.size(), 2);
    CHECK_FALSE(fs.empty());

    // Test the factor query
    CHECK_UNARY(fs.have("MA5"));
    CHECK_UNARY(fs.have("MA10"));
    CHECK_FALSE(fs.have("NONEXIST"));

    // Test getting the factor
    Factor retrieved1 = fs.get("MA5");
    CHECK_EQ(retrieved1.name(), "MA5");
    CHECK_EQ(retrieved1.brief(), "5日均线因子");

    Factor retrieved2 = fs.get("MA10");
    CHECK_EQ(retrieved2.name(), "MA10");
    CHECK_EQ(retrieved2.brief(), "10日均线因子");

    // Test a factor that does not exist
    CHECK_THROWS(fs.get("NONEXIST"));

    // Test removing the factor
    fs.remove("MA5");
    CHECK_EQ(fs.size(), 1);
    CHECK_FALSE(fs.have("MA5"));
    CHECK_UNARY(fs.have("MA10"));

    // Test clear
    fs.clear();
    CHECK_EQ(fs.size(), 0);
    CHECK_UNARY(fs.empty());
    CHECK_FALSE(fs.have("MA10"));
}

/** @par Test point: test the ktype match check of FactorSet */
TEST_CASE("test_FactorSet_ktype_check") {
    // Create the factors of the different periods
    Indicator day_ma = MA(CLOSE(), 5);
    Indicator week_ma = MA(CLOSE(), 10);
    Indicator month_ma = MA(CLOSE(), 20);

    Factor day_factor("DAY_MA", day_ma, KQuery::DAY, "日线因子");
    Factor week_factor("WEEK_MA", week_ma, KQuery::WEEK, "周线因子");
    Factor month_factor("MONTH_MA", month_ma, KQuery::MONTH, "月线因子");

    // Test the daily line FactorSet
    SUBCASE("DAY FactorSet") {
        FactorSet day_fs("DayFactors", KQuery::DAY);

        // A daily line factor should be added normally
        CHECK_NOTHROW(day_fs.add(day_factor));
        CHECK_EQ(day_fs.size(), 1);

        // Adding a weekly line factor should throw an exception
        CHECK_THROWS(day_fs.add(week_factor));
        CHECK_EQ(day_fs.size(), 1);  // The size should stay unchanged

        // Adding a monthly line factor should also throw
        CHECK_THROWS(day_fs.add(month_factor));
        CHECK_EQ(day_fs.size(), 1);  // The size should stay unchanged
    }

    // Test the weekly line FactorSet
    SUBCASE("WEEK FactorSet") {
        FactorSet week_fs("WeekFactors", KQuery::WEEK);

        // A weekly line factor should be added normally
        CHECK_NOTHROW(week_fs.add(week_factor));
        CHECK_EQ(week_fs.size(), 1);

        // Adding a daily line factor should throw an exception
        CHECK_THROWS(week_fs.add(day_factor));
        CHECK_EQ(week_fs.size(), 1);  // The size should stay unchanged

        // Adding a monthly line factor should also throw
        CHECK_THROWS(week_fs.add(month_factor));
        CHECK_EQ(week_fs.size(), 1);  // The size should stay unchanged
    }

    // Test the monthly line FactorSet
    SUBCASE("MONTH FactorSet") {
        FactorSet month_fs("MonthFactors", KQuery::MONTH);

        // A monthly line factor should be added normally
        CHECK_NOTHROW(month_fs.add(month_factor));
        CHECK_EQ(month_fs.size(), 1);

        // Adding a daily line factor should throw an exception
        CHECK_THROWS(month_fs.add(day_factor));
        CHECK_EQ(month_fs.size(), 1);  // The size should stay unchanged

        // Adding a weekly line factor should also throw
        CHECK_THROWS(month_fs.add(week_factor));
        CHECK_EQ(month_fs.size(), 1);  // The size should stay unchanged
    }

    // Test the default constructed FactorSet (DAY)
    SUBCASE("Default FactorSet") {
        FactorSet default_fs("DefaultFactors");
        // The default should be the DAY type
        CHECK_NOTHROW(default_fs.add(day_factor));
        CHECK_EQ(default_fs.size(), 1);

        CHECK_THROWS(default_fs.add(week_factor));
        CHECK_EQ(default_fs.size(), 1);
    }
}

/** @par Test point: test the overwrite of a FactorSet factor with the same name */
TEST_CASE("test_FactorSet_duplicate_name") {
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma5_new = MA(CLOSE(), 5);  // The same name but a different indicator

    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "原始版本");
    Factor factor2("MA5", ma5_new, KQuery::DAY, "5日均线因子", "更新版本");

    FactorSet fs("TestFactorSet");

    // Add the first factor
    fs.add(factor1);
    CHECK_EQ(fs.size(), 1);

    // Get and verify the first factor
    Factor retrieved1 = fs.get("MA5");
    CHECK_EQ(retrieved1.brief(), "5日均线因子");
    CHECK_EQ(retrieved1.details(), "原始版本");

    // Adding a factor with the same name should overwrite the existing one
    fs.add(factor2);
    CHECK_EQ(fs.size(), 1);  // The size stays because it is an overwrite, not a new entry

    // Verify the overwritten factor
    Factor retrieved2 = fs.get("MA5");
    CHECK_EQ(retrieved2.brief(), "5日均线因子");
    CHECK_EQ(retrieved2.details(), "更新版本");  // 应该是新的值

    // Verify the iteration order (there should be one element only)
    size_t count = 0;
    for (const auto& factor : fs) {
        CHECK_EQ(factor.name(), "MA5");
        CHECK_EQ(factor.details(), "更新版本");
        count++;
    }
    CHECK_EQ(count, 1);
}

/** @par Test point: test the shallow copy semantics of FactorSet */
TEST_CASE("test_FactorSet_shallow_copy") {
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    Factor factor1("MA5", ma5, KQuery::DAY);
    Factor factor2("MA10", ma10, KQuery::DAY);

    // Create the original FactorSet
    FactorSet fs1("Original");
    fs1.add(factor1);
    fs1.add(factor2);
    CHECK_EQ(fs1.size(), 2);

    // Test the copy constructor (a shallow copy)
    FactorSet fs2(fs1);
    CHECK_EQ(fs2.name(), "ORIGINAL");
    CHECK_EQ(fs2.size(), 2);
    CHECK_UNARY(fs2.have("MA5"));
    CHECK_UNARY(fs2.have("MA10"));

    // Since it is a shallow copy, modifying the copy affects the original object
    fs2.remove("MA5");

    // Verify that the original object was modified too (the shallow copy property)
    CHECK_EQ(fs1.size(), 1);       // The size of the original object was modified too
    CHECK_FALSE(fs1.have("MA5"));  // The factor of the original object was removed

    // Test the copy assignment (a shallow copy)
    FactorSet fs3("Target");
    fs3 = fs1;
    CHECK_EQ(fs3.name(), "ORIGINAL");
    CHECK_EQ(fs3.size(), 1);

    // Test the move semantics - with a shallow copy the move constructor is a copy constructor
    FactorSet fs4(std::move(fs2));
    // fs4 should reflect the latest state, because all the objects share the same data
    CHECK_EQ(fs4.name(), "ORIGINAL");
    CHECK_EQ(fs4.size(), 1);

    FactorSet fs5("Target2");
    fs5 = std::move(fs3);
    CHECK_EQ(fs5.name(), "ORIGINAL");
    CHECK_EQ(fs5.size(), 1);

#if defined(_MSC_VER) && defined(__clang__)
#pragma warning(push)
#pragma warning(disable : 4552)  // The MSVC self assignment number
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
    // Test the self assignment (it should be safe with a shallow copy)
    fs1 = fs1;
    CHECK_EQ(fs1.size(), 1);
#if defined(_MSC_VER) && defined(__clang__)
#pragma warning(pop)
#endif
}

/** @par Test point: test the iterator functionality of FactorSet */
TEST_CASE("test_FactorSet_iterator") {
    // Create the Factor object used by the test
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "测试5日均线");
    Factor factor2("MA10", ma10, KQuery::DAY, "10日均线因子", "测试10日均线");

    // Create a FactorSet and add the factor
    FactorSet fs("TestFactorSet");
    fs.add(factor1);
    fs.add(factor2);

    CHECK_EQ(fs.size(), 2);
    CHECK_FALSE(fs.empty());

    // Test the range based for loop - the iterator returns a Factor reference directly
    size_t count = 0;
    for (const Factor& factor : fs) {
        CHECK_UNARY_FALSE(factor.isNull());
        CHECK_UNARY_FALSE(factor.name().empty());
        CHECK_UNARY(factor.name() == "MA5" || factor.name() == "MA10");
        count++;
    }
    CHECK_EQ(count, 2);

    // Test the explicit iterator - the dereference returns a Factor reference directly
    auto it = fs.begin();
    auto end = fs.end();
    count = 0;
    while (it != end) {
        const Factor& factor = *it;  // A direct dereference gives the Factor reference
        CHECK_UNARY_FALSE(factor.isNull());
        CHECK_UNARY(factor.name() == "MA5" || factor.name() == "MA10");
        count++;
        ++it;
    }
    CHECK_EQ(count, 2);

    // Test the iterator arrow operator
    for (auto it = fs.begin(); it != fs.end(); ++it) {
        const Factor* factor_ptr = it.operator->();  // The arrow operator returns a Factor pointer
        CHECK_UNARY_FALSE(factor_ptr->isNull());
        CHECK_UNARY_FALSE(factor_ptr->name().empty());
    }

    // Test the equivalence of const_iterator and iterator
    const FactorSet& const_fs = fs;
    count = 0;
    for (auto it = const_fs.begin(); it != const_fs.end(); ++it) {
        const Factor& factor = *it;
        CHECK_UNARY_FALSE(factor.isNull());
        count++;
    }
    CHECK_EQ(count, 2);

    // Test cbegin/cend
    count = 0;
    for (auto it = fs.cbegin(); it != fs.cend(); ++it) {
        const Factor& factor = *it;
        CHECK_UNARY_FALSE(factor.isNull());
        count++;
    }
    CHECK_EQ(count, 2);

    // Test the iterator comparison operations
    auto it1 = fs.begin();
    auto it2 = fs.begin();
    CHECK_UNARY(it1 == it2);
    ++it1;
    CHECK_UNARY(it1 != it2);

    // Test the iterator of an empty set
    FactorSet empty_fs;
    CHECK_UNARY(empty_fs.begin() == empty_fs.end());
    CHECK_UNARY(empty_fs.cbegin() == empty_fs.cend());

    // Test the iterator based factor collection
    vector<Factor> factors;
    for (const Factor& factor : fs) {
        factors.push_back(factor);
    }
    CHECK_EQ(factors.size(), 2);
    // Verify the collected factors
    bool has_ma5 = false, has_ma10 = false;
    for (const auto& factor : factors) {
        if (factor.name() == "MA5")
            has_ma5 = true;
        if (factor.name() == "MA10")
            has_ma10 = true;
    }
    CHECK_UNARY(has_ma5);
    CHECK_UNARY(has_ma10);
}

/** @par Test point: test the Block attribute functionality of FactorSet */
TEST_CASE("test_FactorSet_block") {
    // Test the Block of the default constructor
    FactorSet fs1;
    CHECK_UNARY(fs1.block().isNull());
    CHECK_EQ(fs1.block().size(), 0);

    // Test the constructor with parameters (using the default Block)
    FactorSet fs2("TestSet", KQuery::DAY);
    CHECK_EQ(fs2.name(), "TESTSET");
    CHECK_EQ(fs2.ktype(), KQuery::DAY);
    CHECK_UNARY(fs2.block().isNull());
    CHECK_EQ(fs2.block().size(), 0);

    // Create the Block for the test
    Block test_block("行业", "测试板块");

    // Test the constructor with an explicit Block parameter
    FactorSet fs3("TestWithBlock", KQuery::WEEK, test_block);
    CHECK_EQ(fs3.name(), "TESTWITHBLOCK");
    CHECK_EQ(fs3.ktype(), KQuery::WEEK);
    CHECK_FALSE(fs3.block().isNull());
    CHECK_EQ(fs3.block().category(), "行业");
    CHECK_EQ(fs3.block().name(), "测试板块");
    CHECK_EQ(fs3.block().size(), 0);  // A newly created Block should be empty

    // Test the Block setter / getter
    Block new_block("概念", "新概念板块");
    fs1.block(new_block);
    CHECK_FALSE(fs1.block().isNull());
    CHECK_EQ(fs1.block().category(), "概念");
    CHECK_EQ(fs1.block().name(), "新概念板块");

    // Test the Block sharing under the copy semantics
    FactorSet fs4(fs3);
    CHECK_EQ(fs4.block().category(), "行业");
    CHECK_EQ(fs4.block().name(), "测试板块");

    // Modifying the Block of the copy should affect the original object (a shallow copy)
    Block modified_block = fs4.block();
    modified_block.category("地域");
    modified_block.name("修改后的板块");
    fs4.block(modified_block);

    CHECK_EQ(fs3.block().category(), "地域");
    CHECK_EQ(fs3.block().name(), "修改后的板块");
}

/** @par Test point: test the block match check of FactorSet */
TEST_CASE("test_FactorSet_block_check") {
    // Create the Blocks for the test
    Block block1("行业", "科技板块");
    Block block2("行业", "金融板块");
    Block block3("概念", "新能源概念");

    // Create the test factor (note that the Block parameter is the last one)
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor1("MA5_B1", ma5, KQuery::DAY, "MA5因子", "block1测试", false, Datetime::min(),
                   block1);
    Factor factor2("MA5_B2", ma5, KQuery::DAY, "MA5因子", "block2测试", false, Datetime::min(),
                   block2);
    Factor factor3("MA5_B3", ma5, KQuery::DAY, "MA5因子", "block3测试", false, Datetime::min(),
                   block3);

    // Test the FactorSet constructed with block1
    SUBCASE("FactorSet with block1") {
        FactorSet fs("TestFS", KQuery::DAY, block1);

        // A factor matching block1 should be added normally
        CHECK_NOTHROW(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);

        // Adding a factor not matching block1 should throw an exception
        CHECK_THROWS(fs.add(factor2));
        CHECK_EQ(fs.size(), 1);  // The size should stay unchanged

        CHECK_THROWS(fs.add(factor3));
        CHECK_EQ(fs.size(), 1);  // The size should stay unchanged
    }

    // Test the FactorSet constructed with block2
    SUBCASE("FactorSet with block2") {
        FactorSet fs("TestFS", KQuery::DAY, block2);

        // A factor matching block2 should be added normally
        CHECK_NOTHROW(fs.add(factor2));
        CHECK_EQ(fs.size(), 1);

        // Adding a factor not matching block2 should throw an exception
        CHECK_THROWS(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);  // The size should stay unchanged

        CHECK_THROWS(fs.add(factor3));
        CHECK_EQ(fs.size(), 1);  // The size should stay unchanged
    }

    // Test the FactorSet constructed with block3
    SUBCASE("FactorSet with block3") {
        FactorSet fs("TestFS", KQuery::DAY, block3);

        // A factor matching block3 should be added normally
        CHECK_NOTHROW(fs.add(factor3));
        CHECK_EQ(fs.size(), 1);

        // Adding a factor not matching block3 should throw an exception
        CHECK_THROWS(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);  // The size should stay unchanged

        CHECK_THROWS(fs.add(factor2));
        CHECK_EQ(fs.size(), 1);  // The size should stay unchanged
    }

    // Test the default constructed FactorSet (an empty Block)
    SUBCASE("FactorSet with default empty block") {
        FactorSet fs("TestFS");

        // Create a factor with an empty Block
        Block empty_block;
        Factor factor_empty("EMPTY", ma5, KQuery::DAY, "空Block因子", "", false, Datetime::min(),
                            empty_block);

        // A factor with an empty Block should be added normally
        CHECK_NOTHROW(fs.add(factor_empty));
        CHECK_EQ(fs.size(), 1);

        // Adding a factor with a non-empty Block should throw an exception
        CHECK_THROWS(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);

        CHECK_THROWS(fs.add(factor2));
        CHECK_EQ(fs.size(), 1);

        CHECK_THROWS(fs.add(factor3));
        CHECK_EQ(fs.size(), 1);
    }

    // Test the case of modifying the Block at runtime
    SUBCASE("FactorSet with runtime block modification") {
        FactorSet fs("TestFS", KQuery::DAY, block1);

        // Add a matching factor first
        CHECK_NOTHROW(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);

        // Modify the Block of the FactorSet
        fs.block(block2);

        // Adding the factor that matched before but not now should fail
        CHECK_THROWS(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);

        // Adding a new matching factor should succeed
        CHECK_NOTHROW(fs.add(factor2));
        CHECK_EQ(fs.size(), 2);
    }
}

/** @par Test point: test the factor order preservation of FactorSet */
TEST_CASE("test_FactorSet_order_preservation") {
    // Create the Indicator used by the test
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);
    Indicator ma20 = MA(CLOSE(), 20);

    // Create the Factor objects
    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子");
    Factor factor2("MA10", ma10, KQuery::DAY, "10日均线因子");
    Factor factor3("MA20", ma20, KQuery::DAY, "20日均线因子");

    // Create a FactorSet and add the factors in a specific order
    FactorSet fs("ORDER_TEST", KQuery::DAY);
    fs.add(factor1);  // Added 1st
    fs.add(factor2);  // Added 2nd
    fs.add(factor3);  // Added 3rd

    CHECK_EQ(fs.size(), 3);

    // Verify that the iteration order matches the addition order
    vector<string> expected_order{"MA5", "MA10", "MA20"};
    size_t index = 0;
    for (const auto& factor : fs) {
        CHECK_EQ(factor.name(), expected_order[index]);
        index++;
    }
    CHECK_EQ(index, 3);

    // Test adding a factor with the same name repeatedly
    SUBCASE("Duplicate factor name handling") {
        Factor duplicate_factor("MA5", ma5, KQuery::DAY, "重复的5日均线");
        fs.add(duplicate_factor);
        // The size should stay unchanged
        CHECK_EQ(fs.size(), 3);
        // The order should stay unchanged
        index = 0;
        for (const auto& factor : fs) {
            CHECK_EQ(factor.name(), expected_order[index]);
            index++;
        }
        CHECK_EQ(index, 3);
    }

    // Test removing the last factor
    SUBCASE("Remove last factor") {
        fs.remove("MA20");  // Remove the last factor
        CHECK_EQ(fs.size(), 2);

        vector<string> expected_after_removal{"MA5", "MA10"};
        index = 0;
        for (const auto& factor : fs) {
            CHECK_EQ(factor.name(), expected_after_removal[index]);
            index++;
        }
        CHECK_EQ(index, 2);

        // Verify that the remaining factors can still be accessed
        CHECK_UNARY(fs.have("MA5"));
        CHECK_UNARY(fs.have("MA10"));
        CHECK_FALSE(fs.have("MA20"));
    }

    // Test removing a factor in the middle (the key test point)
    SUBCASE("Remove middle factor") {
        fs.remove("MA10");  // Remove the factor in the middle
        CHECK_EQ(fs.size(), 2);

        // Verify the order: the 1st and the 3rd factors stay and the 2nd was removed
        vector<string> expected_after_removal{"MA5", "MA20"};
        index = 0;
        for (const auto& factor : fs) {
            CHECK_EQ(factor.name(), expected_after_removal[index]);
            index++;
        }
        CHECK_EQ(index, 2);

        // Verify that the remaining factors can still be accessed
        CHECK_UNARY(fs.have("MA5"));
        CHECK_UNARY(fs.have("MA20"));
        CHECK_FALSE(fs.have("MA10"));

        // Verify that the factor can be obtained by name
        Factor retrieved_factor = fs.get("MA20");
        CHECK_EQ(retrieved_factor.name(), "MA20");
    }

    // Test removing all the factors
    SUBCASE("Remove all factors") {
        fs.remove("MA5");
        fs.remove("MA10");
        fs.remove("MA20");
        CHECK_EQ(fs.size(), 0);
        CHECK_UNARY(fs.empty());
        CHECK_FALSE(fs.have("MA5"));
        CHECK_FALSE(fs.have("MA10"));
        CHECK_FALSE(fs.have("MA20"));
    }
}

/** @par Test point: test the getValues functionality of FactorSet */
TEST_CASE("test_FactorSet_getValues") {
    HKU_IF_RETURN(!pluginValid(), void());
    // Create the Indicator used by the test
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    // Create the Factor objects
    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子");
    Factor factor2("MA10", ma10, KQuery::DAY, "10日均线因子");

    // Create the FactorSet
    FactorSet fs("TEST_SET", KQuery::DAY);
    fs.add(factor1);
    fs.add(factor2);

    CHECK_EQ(fs.size(), 2);

    // Create the test stock list
    StockList stocks;
    // Since StockManager is not initialized, an empty list is used here for the test
    // In the real usage it should contain valid stock objects

    KQuery query(0, Null<int64_t>(), KQuery::DAY);

    // Test the basic functionality of getValues
    SUBCASE("Basic getValues functionality") {
        vector<IndicatorList> results = fs.getValues(stocks, query, false);
        // The result count should equal the stock count
        CHECK_EQ(results.size(), stocks.size());
        // The result of every stock should contain the calculation result of every factor
        for (const auto& stock_results : results) {
            CHECK_EQ(stock_results.size(), fs.size());
        }
    }

    // Test the check parameter functionality
    SUBCASE("Check parameter functionality") {
        // It should work without the block check
        CHECK_NOTHROW(fs.getValues(stocks, query, false));

        // The current FactorSet has no block, so check=true should also work
        CHECK_NOTHROW(fs.getValues(stocks, query, true));
    }

    // Test the case of an empty FactorSet
    SUBCASE("Empty FactorSet") {
        FactorSet empty_fs("EMPTY", KQuery::DAY);
        vector<IndicatorList> results = empty_fs.getValues(stocks, query, false);
        CHECK_EQ(results.size(), stocks.size());
        // The result of an empty FactorSet should be an empty IndicatorList
        for (const auto& stock_results : results) {
            CHECK_EQ(stock_results.size(), 0);
        }
    }
}

/** @par Test point: test the full parameter combination of the FactorSet getValues method */
TEST_CASE("test_FactorSet_getValues_complete_params") {
    HKU_IF_RETURN(!pluginValid(), void());
    // Prepare the test data
    StockManager& sm = StockManager::instance();
    Stock stock1 = sm.getStock("sh000001");  // The Shanghai Composite Index
    Stock stock2 = sm.getStock("sz000001");  // The Shenzhen Component Index
    CHECK_FALSE(stock1.isNull());
    CHECK_FALSE(stock2.isNull());

    StockList stocks = {stock1, stock2};
    KQuery query(0, 10, KQuery::DAY);  // Get the data of the first 10 days

    // Create the test FactorSet
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    Factor factor1("MA5", ma5, KQuery::DAY);
    Factor factor2("MA10", ma10, KQuery::DAY);

    FactorSet factorset("PARAM_TEST", KQuery::DAY);
    factorset.add(factor1);
    factorset.add(factor2);

    CHECK_EQ(factorset.size(), 2);

    // Test the different parameter combinations
    SUBCASE("Default parameters") {
        vector<IndicatorList> result = factorset.getValues(stocks, query);
        // Verify that the returned result is not empty
        CHECK_FALSE(result.empty());
        // Verify that every stock has a corresponding indicator result
        CHECK_EQ(result.size(), stocks.size());
        // Verify that the result of every stock contains all the factors
        for (const auto& stock_result : result) {
            CHECK_EQ(stock_result.size(), factorset.size());
        }
    }

    SUBCASE("Align=true") {
        vector<IndicatorList> result = factorset.getValues(stocks, query, true);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
        for (const auto& stock_result : result) {
            CHECK_EQ(stock_result.size(), factorset.size());
        }
    }

    SUBCASE("Fill null=true") {
        vector<IndicatorList> result = factorset.getValues(stocks, query, false, true);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
        for (const auto& stock_result : result) {
            CHECK_EQ(stock_result.size(), factorset.size());
        }
    }

    SUBCASE("To value=true") {
        vector<IndicatorList> result = factorset.getValues(stocks, query, false, false, true);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
        for (const auto& stock_result : result) {
            CHECK_EQ(stock_result.size(), factorset.size());
        }
    }

    SUBCASE("Check=true") {
        vector<IndicatorList> result =
          factorset.getValues(stocks, query, false, false, false, true);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
        for (const auto& stock_result : result) {
            CHECK_EQ(stock_result.size(), factorset.size());
        }
    }

    SUBCASE("All parameters true") {
        vector<IndicatorList> result = factorset.getValues(stocks, query, true, true, true, true);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
        for (const auto& stock_result : result) {
            CHECK_EQ(stock_result.size(), factorset.size());
        }
    }

    SUBCASE("Mixed parameter combinations") {
        // Test the various parameter combinations
        vector<IndicatorList> result1 =
          factorset.getValues(stocks, query, true, false, false, false);
        vector<IndicatorList> result2 =
          factorset.getValues(stocks, query, false, true, false, true);
        vector<IndicatorList> result3 =
          factorset.getValues(stocks, query, false, false, true, false);

        CHECK_FALSE(result1.empty());
        CHECK_FALSE(result2.empty());
        CHECK_FALSE(result3.empty());

        CHECK_EQ(result1.size(), stocks.size());
        CHECK_EQ(result2.size(), stocks.size());
        CHECK_EQ(result3.size(), stocks.size());

        for (size_t i = 0; i < stocks.size(); ++i) {
            CHECK_EQ(result1[i].size(), factorset.size());
            CHECK_EQ(result2[i].size(), factorset.size());
            CHECK_EQ(result3[i].size(), factorset.size());
        }
    }
}

/** @par Test point: test the check parameter of the FactorSet getValues method */
TEST_CASE("test_FactorSet_getValues_check") {
    HKU_IF_RETURN(!pluginValid(), void());
    StockManager& sm = StockManager::instance();
    Stock stock1 = sm.getStock("sh000001");
    Stock stock2 = sm.getStock("sz000001");
    CHECK_FALSE(stock1.isNull());
    CHECK_FALSE(stock2.isNull());

    StockList test_stocks{stock1, stock2};
    KQuery query(0, Null<int64_t>(), KQuery::DAY);  // Use the default query range

    // Create a FactorSet without a Block
    SUBCASE("FactorSet without block") {
        // Create the Indicator used by the test
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor("TEST_FACTOR", ma5, KQuery::DAY);

        FactorSet factorset("NO_BLOCK_SET", KQuery::DAY);
        factorset.add(factor);

        // It should work normally with check=false
        CHECK_NOTHROW(factorset.getValues(test_stocks, query, false, false, false, false));

        // It should also work with check=true (there is no Block restriction)
        CHECK_NOTHROW(factorset.getValues(test_stocks, query, false, false, false, true));
    }

    // Create a FactorSet with a Block
    SUBCASE("FactorSet with block") {
        Block test_block("行业", "测试板块");
        test_block.add(stock1);  // Add the first stock only

        FactorSet factorset("BLOCK_SET", KQuery::DAY, test_block);

        // Create a Factor using the same Block
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor("TEST_FACTOR", ma5, KQuery::DAY, "测试因子", "", false, Datetime::min(),
                      test_block);

        factorset.add(factor);

        // Verify that the Block is set correctly
        CHECK_EQ(factorset.block().size(), 1);
        CHECK_UNARY(factorset.block().have(stock1));
        CHECK_FALSE(factorset.block().have(stock2));

        // It should work normally with check=false
        CHECK_NOTHROW(factorset.getValues(test_stocks, query, false, false, false, false));

        // With check=true, containing the stocks inside the Block only should work
        StockList block_stocks{stock1};
        CHECK_NOTHROW(factorset.getValues(block_stocks, query, false, false, false, true));

        // With check=true, containing a stock outside the Block should throw
        CHECK_THROWS(factorset.getValues(test_stocks, query, false, false, false, true));
    }
}

/** @par Test point: test the boundary conditions of the FactorSet getValues method */
TEST_CASE("test_FactorSet_getValues_edge_cases") {
    HKU_IF_RETURN(!pluginValid(), void());
    StockManager& sm = StockManager::instance();
    Stock stock1 = sm.getStock("sh000001");
    Stock stock2 = sm.getStock("sz000001");
    CHECK_FALSE(stock1.isNull());
    CHECK_FALSE(stock2.isNull());

    StockList stocks = {stock1, stock2};
    KQuery query(0, 10, KQuery::DAY);

    // Create the test FactorSet
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor("EDGE_TEST", ma5, KQuery::DAY);

    FactorSet factorset("EDGE_TEST_SET", KQuery::DAY);
    factorset.add(factor);

    // Test an empty stock list
    SUBCASE("Empty stock list") {
        StockList empty_stocks;
        vector<IndicatorList> result = factorset.getValues(empty_stocks, query);
        // An empty stock list should return an empty result
        CHECK_UNARY(result.empty());
    }

    // Test an invalid query range
    SUBCASE("Invalid query range") {
        KQuery invalid_query(1000000, 1000010, KQuery::DAY);  // Beyond the actual data range
        vector<IndicatorList> result = factorset.getValues(stocks, invalid_query);
        // An empty indicator result container should be returned
        CHECK_FALSE(result.empty());             // The container is not empty
        CHECK_EQ(result.size(), stocks.size());  // But the result of every stock is empty
        for (const auto& stock_result : result) {
            CHECK_EQ(stock_result.size(), factorset.size());
            for (const auto& ind : stock_result) {
                CHECK_UNARY(ind.empty());  // The indicator data is empty
            }
        }
    }

    // Test the different K-line types
    SUBCASE("Different KType") {
        // The daily line query
        KQuery day_query(0, 5, KQuery::DAY);
        vector<IndicatorList> day_result = factorset.getValues(stocks, day_query);
        CHECK_FALSE(day_result.empty());
        CHECK_EQ(day_result.size(), stocks.size());

        // The weekly line query
        KQuery week_query(0, 5, KQuery::WEEK);
        vector<IndicatorList> week_result = factorset.getValues(stocks, week_query);
        CHECK_FALSE(week_result.empty());
        CHECK_EQ(week_result.size(), stocks.size());

        // The monthly line query
        KQuery month_query(0, 5, KQuery::MONTH);
        vector<IndicatorList> month_result = factorset.getValues(stocks, month_query);
        CHECK_FALSE(month_result.empty());
        CHECK_EQ(month_result.size(), stocks.size());
    }

    // Test an empty FactorSet
    SUBCASE("Empty FactorSet") {
        FactorSet empty_set("EMPTY", KQuery::DAY);
        vector<IndicatorList> result = empty_set.getValues(stocks, query);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), stocks.size());
        // The result of an empty FactorSet should be an empty IndicatorList
        for (const auto& stock_result : result) {
            CHECK_EQ(stock_result.size(), 0);
        }
    }
}

/** @par Test point: test the result correctness of the FactorSet getValues method */
TEST_CASE("test_FactorSet_getValues_result_correctness") {
    HKU_IF_RETURN(!pluginValid(), void());
    StockManager& sm = StockManager::instance();
    Stock stock1 = sm.getStock("sh000001");  // The Shanghai Composite Index
    Stock stock2 = sm.getStock("sz000001");  // The Shenzhen Component Index
    CHECK_FALSE(stock1.isNull());
    CHECK_FALSE(stock2.isNull());

    StockList stocks = {stock1, stock2};
    KQuery query(0, 20, KQuery::DAY);  // Get 20 days of data for the verification

    // Create the test factor set
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    Factor factor1("MA5_TEST", ma5, KQuery::DAY);
    Factor factor2("MA10_TEST", ma10, KQuery::DAY);

    FactorSet factorset("RESULT_TEST", KQuery::DAY);
    factorset.add(factor1);
    factorset.add(factor2);

    CHECK_EQ(factorset.size(), 2);

    SUBCASE("Basic functionality and result validation") {
        // Test the basic functionality and verify the calculation result
        vector<IndicatorList> result = factorset.getValues(stocks, query);
        CHECK_FALSE(result.empty());
        CHECK_EQ(result.size(), 2);  // The results of the two stocks

        // Verify the result of every stock
        for (size_t stock_idx = 0; stock_idx < result.size(); ++stock_idx) {
            const IndicatorList& stock_result = result[stock_idx];
            CHECK_EQ(stock_result.size(), 2);  // Every stock has the results of the two factors

            // Verify the result of every factor
            for (size_t factor_idx = 0; factor_idx < stock_result.size(); ++factor_idx) {
                const Indicator& ind = stock_result[factor_idx];
                CHECK_FALSE(ind.empty());
                CHECK_EQ(ind.size(), 20);
                CHECK_GE(ind.discard(), 0);
                CHECK_EQ(ind.getResultNumber(), 1);

                // Verify the sanity of the values (skipping the discard part)
                for (size_t i = ind.discard(); i < ind.size(); ++i) {
                    if (!std::isnan(ind[i]) && !std::isinf(ind[i])) {
                        CHECK_GT(ind[i], 0.0);
                    }
                }

                // Verify that the discard part is really NaN
                for (size_t i = 0; i < ind.discard(); ++i) {
                    CHECK_UNARY(std::isnan(ind[i]));
                }
            }
        }
    }

    SUBCASE("Parameter combinations validation") {
        // The functional verification of the different parameter combinations
        vector<IndicatorList> result1 =
          factorset.getValues(stocks, query, false, false, false, false);
        vector<IndicatorList> result2 =
          factorset.getValues(stocks, query, true, false, false, false);
        vector<IndicatorList> result3 =
          factorset.getValues(stocks, query, false, true, false, false);
        vector<IndicatorList> result4 =
          factorset.getValues(stocks, query, false, false, true, false);

        // All the results should be valid
        CHECK_FALSE(result1.empty());
        CHECK_FALSE(result2.empty());
        CHECK_FALSE(result3.empty());
        CHECK_FALSE(result4.empty());

        // Verify the basic attributes
        CHECK_EQ(result1.size(), 2);
        CHECK_EQ(result2.size(), 2);
        CHECK_EQ(result3.size(), 2);
        CHECK_EQ(result4.size(), 2);

        // Verify that every result has the correct factor count
        for (size_t stock_idx = 0; stock_idx < 2; ++stock_idx) {
            CHECK_EQ(result1[stock_idx].size(), 2);
            CHECK_EQ(result2[stock_idx].size(), 2);
            CHECK_EQ(result3[stock_idx].size(), 2);
            CHECK_EQ(result4[stock_idx].size(), 2);

            // Verify the basic attributes of every factor result
            for (size_t factor_idx = 0; factor_idx < 2; ++factor_idx) {
                const Indicator& ind1 = result1[stock_idx][factor_idx];
                const Indicator& ind2 = result2[stock_idx][factor_idx];
                const Indicator& ind3 = result3[stock_idx][factor_idx];
                const Indicator& ind4 = result4[stock_idx][factor_idx];

                CHECK_FALSE(ind1.empty());
                CHECK_FALSE(ind2.empty());
                CHECK_FALSE(ind3.empty());
                CHECK_FALSE(ind4.empty());

                CHECK_EQ(ind1.size(), 20);
                CHECK_EQ(ind2.size(), 20);
                CHECK_EQ(ind3.size(), 20);
                CHECK_EQ(ind4.size(), 20);
            }
        }
    }

    SUBCASE("Edge case handling validation") {
        // Test the boundary cases
        StockList empty_stocks;
        vector<IndicatorList> empty_result = factorset.getValues(empty_stocks, query);
        CHECK_UNARY(empty_result.empty());

        // Test an invalid query range
        KQuery invalid_query(1000000, 1000010, KQuery::DAY);
        vector<IndicatorList> invalid_result = factorset.getValues(stocks, invalid_query);
        CHECK_FALSE(invalid_result.empty());
        CHECK_EQ(invalid_result.size(), 2);
        for (const auto& stock_result : invalid_result) {
            CHECK_EQ(stock_result.size(), 2);
            for (const auto& ind : stock_result) {
                CHECK_UNARY(ind.empty());
                CHECK_EQ(ind.size(), 0);
            }
        }
    }

    SUBCASE("Multiple KType validation") {
        // Test the different K-line types
        KQuery day_query(0, 10, KQuery::DAY);
        KQuery week_query(0, 10, KQuery::WEEK);
        KQuery month_query(0, 10, KQuery::MONTH);

        vector<IndicatorList> day_result = factorset.getValues(stocks, day_query);
        vector<IndicatorList> week_result = factorset.getValues(stocks, week_query);
        vector<IndicatorList> month_result = factorset.getValues(stocks, month_query);

        CHECK_FALSE(day_result.empty());
        CHECK_FALSE(week_result.empty());
        CHECK_FALSE(month_result.empty());

        CHECK_EQ(day_result.size(), 2);
        CHECK_EQ(week_result.size(), 2);
        CHECK_EQ(month_result.size(), 2);

        // Verify that all of them have reasonable data
        for (size_t stock_idx = 0; stock_idx < 2; ++stock_idx) {
            CHECK_GT(day_result[stock_idx].size(), 0);
            CHECK_GT(week_result[stock_idx].size(), 0);
            CHECK_GT(month_result[stock_idx].size(), 0);

            for (const auto& ind : day_result[stock_idx]) {
                CHECK_GT(ind.size(), 0);
            }
            for (const auto& ind : week_result[stock_idx]) {
                CHECK_GT(ind.size(), 0);
            }
            for (const auto& ind : month_result[stock_idx]) {
                CHECK_GT(ind.size(), 0);
            }
        }
    }

    SUBCASE("Result consistency validation") {
        // Test the consistency of the results of multiple calls
        vector<IndicatorList> result1 = factorset.getValues(stocks, query);
        vector<IndicatorList> result2 = factorset.getValues(stocks, query);

        CHECK_FALSE(result1.empty());
        CHECK_FALSE(result2.empty());
        CHECK_EQ(result1.size(), result2.size());
        CHECK_EQ(result1.size(), 2);

        // Verify the consistency of the result of every stock
        for (size_t stock_idx = 0; stock_idx < 2; ++stock_idx) {
            const IndicatorList& stock_result1 = result1[stock_idx];
            const IndicatorList& stock_result2 = result2[stock_idx];

            CHECK_EQ(stock_result1.size(), stock_result2.size());
            CHECK_EQ(stock_result1.size(), 2);

            // Verify the consistency of every factor result
            for (size_t factor_idx = 0; factor_idx < 2; ++factor_idx) {
                const Indicator& ind1 = stock_result1[factor_idx];
                const Indicator& ind2 = stock_result2[factor_idx];

                CHECK_EQ(ind1.size(), ind2.size());
                CHECK_EQ(ind1.discard(), ind2.discard());

                // Verify that the valid data is consistent
                for (size_t i = std::max(ind1.discard(), ind2.discard());
                     i < std::min(ind1.size(), ind2.size()); ++i) {
                    if (!std::isnan(ind1[i]) && !std::isnan(ind2[i])) {
                        CHECK_EQ(ind1[i], doctest::Approx(ind2[i]).epsilon(0.0001));
                    }
                }
            }
        }
    }

    SUBCASE("Align parameter validation") {
        // Test the effect of the align parameter
        vector<IndicatorList> result_false = factorset.getValues(stocks, query, false);
        vector<IndicatorList> result_true = factorset.getValues(stocks, query, true);

        CHECK_FALSE(result_false.empty());
        CHECK_FALSE(result_true.empty());
        CHECK_EQ(result_false.size(), result_true.size());
        CHECK_EQ(result_false.size(), 2);

        // Get the trading calendar to verify the effect of align=true
        DatetimeList trading_dates = sm.getTradingCalendar(query);
        CHECK_FALSE(trading_dates.empty());

        // Verify that with align=true the result size matches the trading calendar
        for (size_t stock_idx = 0; stock_idx < result_true.size(); ++stock_idx) {
            for (size_t factor_idx = 0; factor_idx < result_true[stock_idx].size(); ++factor_idx) {
                const Indicator& ind_aligned = result_true[stock_idx][factor_idx];
                CHECK_EQ(ind_aligned.size(), trading_dates.size());
            }
        }

        // Verify the basic structure
        for (size_t stock_idx = 0; stock_idx < 2; ++stock_idx) {
            for (size_t factor_idx = 0; factor_idx < 2; ++factor_idx) {
                const Indicator& ind_false = result_false[stock_idx][factor_idx];
                const Indicator& ind_true = result_true[stock_idx][factor_idx];

                CHECK_FALSE(ind_false.empty());
                CHECK_FALSE(ind_true.empty());
                CHECK_GT(ind_false.size(), 0);
                CHECK_GT(ind_true.size(), 0);
            }
        }
    }
}

TEST_CASE("test_CompiledFactorPlan_executor_reuse") {
    StockManager& sm = StockManager::instance();
    Stock stock1 = sm.getStock("sh000001");
    Stock stock2 = sm.getStock("sz000001");
    REQUIRE_FALSE(stock1.isNull());
    REQUIRE_FALSE(stock2.isNull());

    KData k1 = stock1.getKData(KQuery(0, 30, KQuery::DAY));
    KData k2 = stock2.getKData(KQuery(0, 300, KQuery::DAY));
    REQUIRE_FALSE(k1.empty());
    REQUIRE_FALSE(k2.empty());

    IndicatorList formulas{
      MA(CLOSE(), 5),
      EMA(CLOSE(), 13),
      STDEV(CLOSE(), 10),
      MA(CLOSE(), IndParam(CVAL(CLOSE(), 7))),
      CORR(CLOSE(), OPEN(), 10),
    };
    Indicator low = LOW();
    formulas.emplace_back(CORR(MA(VOL(), 20), low, 5) + (HIGH() + low) / 2.0 - CLOSE());
    Indicator open_close = OPEN() - CLOSE();
    formulas.emplace_back(CORR(REF(open_close, 1), CLOSE(), 200) + open_close);
    detail::CompiledFactorPlan plan(formulas);
    REQUIRE_UNARY(plan.isReusable());
    auto executor = plan.createExecutor();

    auto check_one = [&](const KData& kdata) {
        IndicatorList result = executor.executeValues(kdata);
        REQUIRE_EQ(result.size(), formulas.size());
        for (size_t i = 0; i < formulas.size(); ++i) {
            check_indicator(result[i], formulas[i](kdata).getResult(0));
        }
    };

    // Exercise buffer growth, shrinkage, and returning to the original stock on one executor.
    check_one(k1);
    Indicator preserved = executor.executeValues(k1)[0];
    check_one(k2);
    check_indicator(preserved, formulas[0](k1).getResult(0));
    check_one(k1);

    IndicatorList shrunk = executor.executeValues(k1);
    for (const auto& value : shrunk) {
        CHECK_EQ(value.size(), k1.size());
    }
}

TEST_CASE("test_CompiledFactorPlan_preserves_shared_formula_dag") {
    Indicator shared(make_shared<CountingContextIndicatorImp>());
    Indicator repeated = shared + shared;

    g_counting_indicator_clones = 0;
    detail::CompiledFactorPlan plan({repeated});
    REQUIRE_UNARY(plan.isReusable());
    CHECK_EQ(g_counting_indicator_clones.load(), 1);
}

TEST_CASE("test_CompiledFactorPlan_fallback_eligibility") {
    detail::CompiledFactorPlan context_plan({hku::CONTEXT(CLOSE())});
    CHECK_FALSE(context_plan.isReusable());

    Indicator python_like(make_shared<PythonLikeIndicatorImp>());
    detail::CompiledFactorPlan python_plan({python_like});
    CHECK_FALSE(python_plan.isReusable());

    Indicator custom(make_shared<CountingContextIndicatorImp>());
    custom.getImp()->supportBatchReuse(false);
    detail::CompiledFactorPlan custom_plan({custom});
    CHECK_FALSE(custom_plan.isReusable());

    detail::CompiledFactorPlan empty_data_plan({MA(CLOSE(), 5)});
    REQUIRE_UNARY(empty_data_plan.isReusable());
    auto executor = empty_data_plan.createExecutor();
    IndicatorList empty_values = executor.executeValues(KData());
    REQUIRE_EQ(empty_values.size(), 1);
    CHECK_UNARY(empty_values[0].empty());
}

TEST_CASE("test_CompiledFactorPlan_root_cse") {
    StockManager& sm = StockManager::instance();
    KData k1 = sm.getStock("sh000001").getKData(KQuery(0, 30, KQuery::DAY));
    KData k2 = sm.getStock("sz000001").getKData(KQuery(0, 30, KQuery::DAY));
    REQUIRE_FALSE(k1.empty());
    REQUIRE_FALSE(k2.empty());

    Indicator formula(make_shared<CountingContextIndicatorImp>());
    detail::CompiledFactorPlan plan({formula, formula, formula + 1.0});
    REQUIRE_UNARY(plan.isReusable());
    auto executor = plan.createExecutor();

    g_counting_indicator_calculations = 0;
    IndicatorList first = executor.executeValues(k1);
    REQUIRE_EQ(first.size(), 3);
    CHECK_EQ(g_counting_indicator_calculations.load(), 1);
    CHECK_NE(first[0].getImp().get(), first[1].getImp().get());
    check_indicator(first[0], first[1]);
    check_indicator(first[0] + 1.0, first[2]);

    IndicatorList second = executor.executeValues(k2);
    REQUIRE_EQ(second.size(), 3);
    CHECK_EQ(g_counting_indicator_calculations.load(), 2);
    check_indicator(second[0], second[1]);
    check_indicator(second[0] + 1.0, second[2]);
}

TEST_CASE("test_FactorSet_compiled_values_match_legacy") {
    StockManager& sm = StockManager::instance();
    StockList stocks{sm.getStock("sh000001"), sm.getStock("sz000001")};
    REQUIRE_FALSE(stocks[0].isNull());
    REQUIRE_FALSE(stocks[1].isNull());

    KQuery query(0, 120, KQuery::DAY);
    FactorSet factorset("COMPILED_VALUES", KQuery::DAY);
    factorset.add("MA5", MA(CLOSE(), 5));
    factorset.add("EMA13", EMA(CLOSE(), 13));
    factorset.add("STDEV20", STDEV(CLOSE(), 20));

    auto values = factorset.getValues(stocks, query, false, false, true);
    REQUIRE_EQ(values.size(), stocks.size());
    for (size_t si = 0; si < stocks.size(); ++si) {
        KData kdata = stocks[si].getKData(query);
        REQUIRE_EQ(values[si].size(), factorset.size());
        for (size_t fi = 0; fi < factorset.size(); ++fi) {
            CAPTURE(si);
            CAPTURE(fi);
            CAPTURE(factorset[fi].name());
            Indicator expected = factorset[fi].formula()(kdata).getResult(0);
            check_indicator(values[si][fi], expected);
        }
    }

    DatetimeList dates = sm.getTradingCalendar(query);
    auto aligned = factorset.getValues(stocks, query, true, false, true, false, dates);
    REQUIRE_EQ(aligned.size(), stocks.size());
    for (size_t si = 0; si < stocks.size(); ++si) {
        KData kdata = stocks[si].getKData(query);
        for (size_t fi = 0; fi < factorset.size(); ++fi) {
            CAPTURE(si);
            CAPTURE(fi);
            CAPTURE(factorset[fi].name());
            Indicator expected = ALIGN(factorset[fi].formula(), dates, false)(kdata).getResult(0);
            check_indicator(aligned[si][fi], expected);
        }
    }
}

TEST_CASE("test_FactorSet_compiled_values_keep_nested_factors_distinct") {
    Stock stock = StockManager::instance().getStock("sh000001");
    REQUIRE_FALSE(stock.isNull());

    Factor close_ma5("INNER_CLOSE_MA5", MA(CLOSE(), 5), KQuery::DAY);
    Factor high_ma3("INNER_HIGH_MA3", MA(HIGH(), 3), KQuery::DAY);
    FactorSet factorset("NESTED_FACTORS", KQuery::DAY);
    factorset.add("OUTER_CLOSE_MA5", FACTOR(close_ma5));
    factorset.add("OUTER_HIGH_MA3", FACTOR(high_ma3));

    KQuery query(0, 30, KQuery::DAY);
    KData kdata = stock.getKData(query);
    REQUIRE_FALSE(kdata.empty());

    auto compiled = factorset.getValues({stock}, query, false, false, true);
    auto legacy = factorset.getValues({stock}, query, false, false, false);
    REQUIRE_EQ(compiled.size(), 1);
    REQUIRE_EQ(compiled[0].size(), 2);
    REQUIRE_EQ(legacy.size(), 1);
    REQUIRE_EQ(legacy[0].size(), 2);

    Indicator expected_close_ma5 = FACTOR(close_ma5)(kdata).getResult(0);
    Indicator expected_high_ma3 = FACTOR(high_ma3)(kdata).getResult(0);
    check_indicator(compiled[0][0], expected_close_ma5);
    check_indicator(compiled[0][1], expected_high_ma3);
    check_indicator(legacy[0][0].getResult(0), expected_close_ma5);
    check_indicator(legacy[0][1].getResult(0), expected_high_ma3);
    CHECK_FALSE(compiled[0][0].equal(compiled[0][1]));
}

TEST_CASE("test_FactorSet_formula_results_keep_independent_graphs") {
    Stock stock = StockManager::instance().getStock("sh000001");
    REQUIRE_FALSE(stock.isNull());

    Indicator shared_formula = MA(CLOSE(), 5);
    FactorSet factorset("FORMULA_RESULTS", KQuery::DAY);
    factorset.add("FIRST", shared_formula);
    factorset.add("SECOND", shared_formula);

    auto result = factorset.getValues({stock}, KQuery(0, 30, KQuery::DAY), false, false, false);
    REQUIRE_EQ(result.size(), 1);
    REQUIRE_EQ(result[0].size(), 2);
    CHECK_NE(result[0][0].getImp().get(), result[0][1].getImp().get());
    check_indicator(result[0][0], result[0][1]);
}

TEST_CASE("test_FactorSet_compiled_path_nested_invocation") {
    Stock stock1 = StockManager::instance().getStock("sh000001");
    Stock stock2 = StockManager::instance().getStock("sz000001");
    REQUIRE_FALSE(stock1.isNull());
    REQUIRE_FALSE(stock2.isNull());
    StockList stocks{stock1, stock2};
    KQuery query(0, 30, KQuery::DAY);

    FactorSet factorset("NESTED_COMPILED", KQuery::DAY);
    factorset.add("MA5", MA(CLOSE(), 5));
    factorset.add("EMA13", EMA(CLOSE(), 13));

    vector<std::future<vector<IndicatorList>>> futures;
    for (size_t i = 0; i < 4; ++i) {
        futures.emplace_back(global_submit_task([factorset, stocks, query]() {
            return factorset.getValues(stocks, query, false, false, true);
        }));
    }

    for (auto& future : futures) {
        global_wait_task(future);
        auto result = future.get();
        REQUIRE_EQ(result.size(), stocks.size());
        for (size_t i = 0; i < stocks.size(); ++i) {
            REQUIRE_EQ(result[i].size(), 2);
            KData kdata = stocks[i].getKData(query);
            check_indicator(result[i][0], MA(CLOSE(), 5)(kdata).getResult(0));
            check_indicator(result[i][1], EMA(CLOSE(), 13)(kdata).getResult(0));
        }
    }
}

/** @par Test point: test the boundary conditions and the exception handling of FactorSet */
TEST_CASE("test_FactorSet_edge_cases_and_exceptions") {
    // Test the construction with an empty name
    SUBCASE("Empty name construction") {
        FactorSet fs("", KQuery::DAY);
        CHECK_EQ(fs.name(), "");
        CHECK_EQ(fs.ktype(), KQuery::DAY);
        CHECK_EQ(fs.size(), 0);
        CHECK_UNARY(fs.empty());
    }

    // Test an extremely long name
    SUBCASE("Very long name") {
        string long_name(1000, 'A');
        FactorSet fs(long_name, KQuery::DAY);
        CHECK_EQ(fs.name(), long_name);
        CHECK_EQ(fs.ktype(), KQuery::DAY);
    }

    // Test the case of many factors
    SUBCASE("Large number of factors") {
        FactorSet fs("LARGE_SET", KQuery::DAY);
        Indicator base_indicator = CLOSE();

        // Add many factors (testing the performance and the stability)
        const size_t factor_count = 100;
        for (size_t i = 0; i < factor_count; ++i) {
            string name = "FACTOR_" + std::to_string(i);
            Indicator ind = MA(base_indicator, static_cast<int>(i + 5));
            Factor factor(name, ind, KQuery::DAY);
            CHECK_NOTHROW(fs.add(factor));
        }

        CHECK_EQ(fs.size(), factor_count);

        // Test the random access
        for (size_t i = 0; i < factor_count; i += 10) {
            string name = "FACTOR_" + std::to_string(i);
            CHECK_UNARY(fs.have(name));
            const Factor& factor = fs.get(name);
            CHECK_EQ(factor.name(), name);
        }

        // Test the iterator traversal
        size_t count = 0;
        for (const auto& factor : fs) {
            CHECK_FALSE(factor.name().empty());
            count++;
        }
        CHECK_EQ(count, factor_count);
    }

    // Test adding many factors with the same name repeatedly
    SUBCASE("Massive duplicate additions") {
        FactorSet fs("DUPLICATE_TEST", KQuery::DAY);
        Indicator ma5 = MA(CLOSE(), 5);
        Factor base_factor("SAME_NAME", ma5, KQuery::DAY);

        const size_t iterations = 3;
        for (size_t i = 0; i < iterations; ++i) {
            CHECK_NOTHROW(fs.add(base_factor));
            // The size should always stay 1
            CHECK_EQ(fs.size(), 1);
        }

        CHECK_EQ(fs.size(), 1);
        CHECK_UNARY(fs.have("SAME_NAME"));
    }

    // Test the concurrent access (simulating a multi-threaded environment)
    SUBCASE("Concurrent-like access simulation") {
        FactorSet fs("CONCURRENT_TEST", KQuery::DAY);
        Indicator ma5 = MA(CLOSE(), 5);
        Indicator ma10 = MA(CLOSE(), 10);

        Factor factor1("MA5", ma5, KQuery::DAY);
        Factor factor2("MA10", ma10, KQuery::DAY);

        // Simulate a concurrent add
        fs.add(factor1);
        fs.add(factor2);

        // Simulate a concurrent read
        CHECK_UNARY(fs.have("MA5"));
        CHECK_UNARY(fs.have("MA10"));

        const Factor& f1 = fs.get("MA5");
        const Factor& f2 = fs.get("MA10");

        CHECK_EQ(f1.name(), "MA5");
        CHECK_EQ(f2.name(), "MA10");

        // Simulate a concurrent remove
        fs.remove("MA5");
        CHECK_FALSE(fs.have("MA5"));
        CHECK_EQ(fs.size(), 1);
    }
}

/** @par Test point: test the performance related functionality of FactorSet */
TEST_CASE("test_FactorSet_performance_features") {
    // Test the performance of the FactorSet with the different K-line types
    SUBCASE("Different KType performance") {
        Indicator ma5 = MA(CLOSE(), 5);

        // The daily line FactorSet
        FactorSet day_fs("DAY_PERF", KQuery::DAY);
        Factor day_factor("DAY_MA5", ma5, KQuery::DAY);
        CHECK_NOTHROW(day_fs.add(day_factor));
        CHECK_EQ(day_fs.size(), 1);

        // The weekly line FactorSet
        FactorSet week_fs("WEEK_PERF", KQuery::WEEK);
        Factor week_factor("WEEK_MA5", ma5, KQuery::WEEK);
        CHECK_NOTHROW(week_fs.add(week_factor));
        CHECK_EQ(week_fs.size(), 1);

        // The monthly line FactorSet
        FactorSet month_fs("MONTH_PERF", KQuery::MONTH);
        Factor month_factor("MONTH_MA5", ma5, KQuery::MONTH);
        CHECK_NOTHROW(month_fs.add(month_factor));
        CHECK_EQ(month_fs.size(), 1);

        // The quarterly line FactorSet
        FactorSet quarter_fs("QUARTER_PERF", KQuery::QUARTER);
        Factor quarter_factor("QUARTER_MA5", ma5, KQuery::QUARTER);
        CHECK_NOTHROW(quarter_fs.add(quarter_factor));
        CHECK_EQ(quarter_fs.size(), 1);

        // The yearly line FactorSet
        FactorSet year_fs("YEAR_PERF", KQuery::YEAR);
        Factor year_factor("YEAR_MA5", ma5, KQuery::YEAR);
        CHECK_NOTHROW(year_fs.add(year_factor));
        CHECK_EQ(year_fs.size(), 1);
    }

    // Test the performance of a complex indicator combination
    SUBCASE("Complex indicator combination performance") {
        FactorSet fs("COMPLEX_PERF", KQuery::DAY);

        // Create a complex indicator combination
        Indicator close = CLOSE();
        Indicator open = OPEN();
        Indicator high = HIGH();
        Indicator low = LOW();
        Indicator vol = VOL();

        // Add the various complex factors
        vector<std::pair<string, Indicator>> indicators = {
          {"PRICE_RANGE", high - low},
          {"PRICE_CHANGE", close - open},
          {"VOL_CHANGE", MA(vol, 5) - MA(vol, 10)},
          {"PRICE_VOL_RATIO", close / vol},
          {"COMPLEX_MA", (MA(close, 5) + MA(close, 10)) / 2}};

        for (const auto& item : indicators) {
            Factor factor(item.first, item.second, KQuery::DAY);
            CHECK_NOTHROW(fs.add(factor));
        }

        CHECK_EQ(fs.size(), indicators.size());

        // Verify that all the factors can be accessed correctly
        for (const auto& item : indicators) {
            CHECK_UNARY(fs.have(item.first));
            const Factor& factor = fs.get(item.first);
            CHECK_EQ(factor.name(), item.first);
        }
    }
}

/** @par Test point: test the memory management and the resource release of FactorSet */
TEST_CASE("test_FactorSet_memory_management") {
    // Test the memory management of many temporary objects
    SUBCASE("Temporary object memory management") {
        vector<FactorSet> factor_sets;

        // Create multiple FactorSet instances
        const size_t set_count = 50;
        for (size_t i = 0; i < set_count; ++i) {
            string name = "TEMP_SET_" + std::to_string(i);
            FactorSet fs(name, KQuery::DAY);

            // Add a few factors to every FactorSet
            for (size_t j = 0; j < 3; ++j) {
                string factor_name = "FACTOR_" + std::to_string(j);
                Indicator ind = MA(CLOSE(), static_cast<int>(j + 5));
                Factor factor(factor_name, ind, KQuery::DAY);
                fs.add(factor);
            }

            factor_sets.push_back(std::move(fs));
        }

        // Verify that all the FactorSets were created correctly
        CHECK_EQ(factor_sets.size(), set_count);
        for (size_t i = 0; i < set_count; ++i) {
            CHECK_EQ(factor_sets[i].size(), 3);
            CHECK_EQ(factor_sets[i].name(), "TEMP_SET_" + std::to_string(i));
        }

        // Clean up
        factor_sets.clear();
    }

    // Test the circular reference detection
    SUBCASE("Circular reference detection") {
        FactorSet fs1("SET1", KQuery::DAY);
        FactorSet fs2("SET2", KQuery::DAY);

        // A normal operation should not cause a circular reference
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor1("MA5_1", ma5, KQuery::DAY);
        Factor factor2("MA5_2", ma5, KQuery::DAY);

        CHECK_NOTHROW(fs1.add(factor1));
        CHECK_NOTHROW(fs2.add(factor2));

        // Verify that the two FactorSets are independent of each other
        CHECK_EQ(fs1.size(), 1);
        CHECK_EQ(fs2.size(), 1);
        CHECK_NE(&fs1, &fs2);
    }
}

/** @par Test point: test the integration of FactorSet with the other systems */
TEST_CASE("test_FactorSet_integration") {
    // Test the integration with StockManager
    SUBCASE("StockManager integration") {
        FactorSet fs("STOCK_INTEGRATION", KQuery::DAY);
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor("MA5_INTEGRATION", ma5, KQuery::DAY);

        CHECK_NOTHROW(fs.add(factor));
        CHECK_EQ(fs.size(), 1);

        // Test getAllValues (it calls StockManager)
        KQuery query(0, 10, KQuery::DAY);
        CHECK_NOTHROW(fs.getAllValues(query));
        // Note: the real result depends on the state of StockManager; only no exception is tested
        // here
    }

    // Test the integration with the Block system
    SUBCASE("Block system integration") {
        // Create a real Block (if possible)
        Block real_block("行业", "真实测试板块");

        FactorSet fs("BLOCK_INTEGRATION", KQuery::DAY, real_block);
        CHECK_EQ(fs.block().category(), "行业");
        CHECK_EQ(fs.block().name(), "真实测试板块");

        // Test the Block related operations
        Block new_block("概念", "新概念");
        fs.block(new_block);
        CHECK_EQ(fs.block().category(), "概念");
        CHECK_EQ(fs.block().name(), "新概念");
    }
}

#if HKU_SUPPORT_SERIALIZATION

/** @par Test point: test the serialization of FactorSet */
TEST_CASE("test_FactorSet_serialization") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/FactorSet.xml";

    // Create the Factor object used by the test
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    // Create the Block for the test
    Block test_block("行业", "测试板块");

    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "测试5日均线", false, Datetime::min(),
                   test_block);
    Factor factor2("MA10", ma10, KQuery::DAY, "10日均线因子", "测试10日均线", false,
                   Datetime::min(), test_block);

    // Test the basic serialization - an empty FactorSet
    SUBCASE("Empty FactorSet serialization") {
        FactorSet fs1("EMPTY_TEST", KQuery::DAY, test_block);

        {
            std::ofstream ofs(filename);
            boost::archive::xml_oarchive oa(ofs);
            oa << BOOST_SERIALIZATION_NVP(fs1);
        }

        FactorSet fs2;
        {
            std::ifstream ifs(filename);
            boost::archive::xml_iarchive ia(ifs);
            ia >> BOOST_SERIALIZATION_NVP(fs2);
        }

        CHECK_EQ(fs1.name(), fs2.name());
        CHECK_EQ(fs1.ktype(), fs2.ktype());
        CHECK_EQ(fs1.size(), fs2.size());
        CHECK_EQ(fs1.block().category(), fs2.block().category());
        CHECK_EQ(fs1.block().name(), fs2.block().name());
    }

    // Test the serialization with the factors
    SUBCASE("FactorSet with factors serialization") {
        FactorSet fs1("FACTORS_TEST", KQuery::DAY, test_block);
        fs1.add(factor1);
        fs1.add(factor2);

        {
            std::ofstream ofs(filename);
            boost::archive::xml_oarchive oa(ofs);
            oa << BOOST_SERIALIZATION_NVP(fs1);
        }

        FactorSet fs2;
        {
            std::ifstream ifs(filename);
            boost::archive::xml_iarchive ia(ifs);
            ia >> BOOST_SERIALIZATION_NVP(fs2);
        }

        CHECK_EQ(fs1.name(), fs2.name());
        CHECK_EQ(fs1.ktype(), fs2.ktype());
        CHECK_EQ(fs1.size(), fs2.size());
        CHECK_EQ(fs1.block().category(), fs2.block().category());
        CHECK_EQ(fs1.block().name(), fs2.block().name());

        // Verify that the factors were serialized correctly too
        CHECK_UNARY(fs2.have("MA5"));
        CHECK_UNARY(fs2.have("MA10"));

        Factor retrieved_ma5 = fs2.get("MA5");
        CHECK_EQ(retrieved_ma5.name(), "MA5");
        CHECK_EQ(retrieved_ma5.brief(), "5日均线因子");

        Factor retrieved_ma10 = fs2.get("MA10");
        CHECK_EQ(retrieved_ma10.name(), "MA10");
        CHECK_EQ(retrieved_ma10.brief(), "10日均线因子");
    }

    // Test the serialization of the different K-line types
    SUBCASE("Different KType serialization") {
        Indicator week_ma = MA(CLOSE(), 5);
        Factor week_factor("WEEK_MA", week_ma, KQuery::WEEK, "周线因子");

        FactorSet fs1("WEEK_TEST", KQuery::WEEK);
        fs1.add(week_factor);

        {
            std::ofstream ofs(filename);
            boost::archive::xml_oarchive oa(ofs);
            oa << BOOST_SERIALIZATION_NVP(fs1);
        }

        FactorSet fs2;
        {
            std::ifstream ifs(filename);
            boost::archive::xml_iarchive ia(ifs);
            ia >> BOOST_SERIALIZATION_NVP(fs2);
        }

        CHECK_EQ(fs1.name(), fs2.name());
        CHECK_EQ(fs1.ktype(), fs2.ktype());
        CHECK_EQ(fs1.size(), fs2.size());
        CHECK_UNARY(fs2.have("WEEK_MA"));
    }

    // Test the serialization of an empty name
    SUBCASE("Empty name serialization") {
        FactorSet fs1("", KQuery::DAY, test_block);
        fs1.add(factor1);

        {
            std::ofstream ofs(filename);
            boost::archive::xml_oarchive oa(ofs);
            oa << BOOST_SERIALIZATION_NVP(fs1);
        }

        FactorSet fs2;
        {
            std::ifstream ifs(filename);
            boost::archive::xml_iarchive ia(ifs);
            ia >> BOOST_SERIALIZATION_NVP(fs2);
        }

        CHECK_EQ(fs1.name(), fs2.name());
        CHECK_EQ(fs1.ktype(), fs2.ktype());
        CHECK_EQ(fs1.size(), fs2.size());
        CHECK_UNARY(fs2.have("MA5"));
    }

    // Test the serialization after the modification
    SUBCASE("Modified FactorSet serialization") {
        FactorSet fs1("MODIFY_TEST", KQuery::DAY, test_block);
        fs1.add(factor1);

        {
            std::ofstream ofs(filename);
            boost::archive::xml_oarchive oa(ofs);
            oa << BOOST_SERIALIZATION_NVP(fs1);
        }

        // Modify the original object
        fs1.add(factor2);
        fs1.remove("MA5");

        FactorSet fs2;
        {
            std::ifstream ifs(filename);
            boost::archive::xml_iarchive ia(ifs);
            ia >> BOOST_SERIALIZATION_NVP(fs2);
        }

        // Verify that the deserialized object has the state at the save time, unaffected by the
        // later modification
        CHECK_EQ(fs2.name(), "MODIFY_TEST");
        CHECK_EQ(fs2.size(), 1);
        CHECK_UNARY(fs2.have("MA5"));
        CHECK_FALSE(fs2.have("MA10"));
    }

    // Test the serialization with an empty Block
    SUBCASE("Empty Block serialization") {
        FactorSet fs1("NO_BLOCK_TEST", KQuery::DAY);

        {
            std::ofstream ofs(filename);
            boost::archive::xml_oarchive oa(ofs);
            oa << BOOST_SERIALIZATION_NVP(fs1);
        }

        FactorSet fs2;
        {
            std::ifstream ifs(filename);
            boost::archive::xml_iarchive ia(ifs);
            ia >> BOOST_SERIALIZATION_NVP(fs2);
        }

        CHECK_EQ(fs1.name(), fs2.name());
        CHECK_EQ(fs1.ktype(), fs2.ktype());
        CHECK_EQ(fs1.size(), fs2.size());
        CHECK_UNARY(fs2.block().isNull());
    }
}

#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
