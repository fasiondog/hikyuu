/*
 * test_FactorSet.cpp
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/factor/FactorSet.h>
#include <hikyuu/factor/Factor.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/StockManager.h>
#include <algorithm>
#include <utility>  // for std::pair
#include <string>   // for std::to_string

using namespace hku;

/**
 * @defgroup test_FactorSet test_FactorSet
 * @ingroup test_hikyuu_factor_suite
 * @{
 */

/** @par 检测点：测试FactorSet基本功能 */
TEST_CASE("test_FactorSet_basic") {
    // 创建测试用的 Factor 对象
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "测试5日均线");
    Factor factor2("MA10", ma10, KQuery::DAY, "10日均线因子", "测试10日均线");

    // 测试构造函数和基本属性
    FactorSet fs("TestFactorSet");
    CHECK_EQ(fs.name(), "TESTFACTORSET");

    // 测试空状态
    CHECK_EQ(fs.size(), 0);
    CHECK_UNARY(fs.empty());

    // 添加因子
    fs.add(factor1);
    fs.add(factor2);

    CHECK_EQ(fs.size(), 2);
    CHECK_FALSE(fs.empty());

    // 测试因子查询
    CHECK_UNARY(fs.have("MA5"));
    CHECK_UNARY(fs.have("MA10"));
    CHECK_FALSE(fs.have("NONEXIST"));

    // 测试获取因子
    Factor retrieved1 = fs.get("MA5");
    CHECK_EQ(retrieved1.name(), "MA5");
    CHECK_EQ(retrieved1.brief(), "5日均线因子");

    Factor retrieved2 = fs.get("MA10");
    CHECK_EQ(retrieved2.name(), "MA10");
    CHECK_EQ(retrieved2.brief(), "10日均线因子");

    // 测试不存在的因子
    CHECK_THROWS(fs.get("NONEXIST"));

    // 测试移除因子
    fs.remove("MA5");
    CHECK_EQ(fs.size(), 1);
    CHECK_FALSE(fs.have("MA5"));
    CHECK_UNARY(fs.have("MA10"));

    // 测试清空
    fs.clear();
    CHECK_EQ(fs.size(), 0);
    CHECK_UNARY(fs.empty());
    CHECK_FALSE(fs.have("MA10"));
}

/** @par 检测点：测试FactorSet ktype 匹配检查 */
TEST_CASE("test_FactorSet_ktype_check") {
    // 创建不同周期的因子
    Indicator day_ma = MA(CLOSE(), 5);
    Indicator week_ma = MA(CLOSE(), 10);
    Indicator month_ma = MA(CLOSE(), 20);

    Factor day_factor("DAY_MA", day_ma, KQuery::DAY, "日线因子");
    Factor week_factor("WEEK_MA", week_ma, KQuery::WEEK, "周线因子");
    Factor month_factor("MONTH_MA", month_ma, KQuery::MONTH, "月线因子");

    // 测试日线 FactorSet
    SUBCASE("DAY FactorSet") {
        FactorSet day_fs("DayFactors", KQuery::DAY);

        // 应该能正常添加日线因子
        CHECK_NOTHROW(day_fs.add(day_factor));
        CHECK_EQ(day_fs.size(), 1);

        // 添加周线因子应该抛出异常
        CHECK_THROWS(day_fs.add(week_factor));
        CHECK_EQ(day_fs.size(), 1);  // 大小应该不变

        // 添加月线因子也应该抛出异常
        CHECK_THROWS(day_fs.add(month_factor));
        CHECK_EQ(day_fs.size(), 1);  // 大小应该不变
    }

    // 测试周线 FactorSet
    SUBCASE("WEEK FactorSet") {
        FactorSet week_fs("WeekFactors", KQuery::WEEK);

        // 应该能正常添加周线因子
        CHECK_NOTHROW(week_fs.add(week_factor));
        CHECK_EQ(week_fs.size(), 1);

        // 添加日线因子应该抛出异常
        CHECK_THROWS(week_fs.add(day_factor));
        CHECK_EQ(week_fs.size(), 1);  // 大小应该不变

        // 添加月线因子也应该抛出异常
        CHECK_THROWS(week_fs.add(month_factor));
        CHECK_EQ(week_fs.size(), 1);  // 大小应该不变
    }

    // 测试月线 FactorSet
    SUBCASE("MONTH FactorSet") {
        FactorSet month_fs("MonthFactors", KQuery::MONTH);

        // 应该能正常添加月线因子
        CHECK_NOTHROW(month_fs.add(month_factor));
        CHECK_EQ(month_fs.size(), 1);

        // 添加日线因子应该抛出异常
        CHECK_THROWS(month_fs.add(day_factor));
        CHECK_EQ(month_fs.size(), 1);  // 大小应该不变

        // 添加周线因子也应该抛出异常
        CHECK_THROWS(month_fs.add(week_factor));
        CHECK_EQ(month_fs.size(), 1);  // 大小应该不变
    }

    // 测试默认构造的 FactorSet (DAY)
    SUBCASE("Default FactorSet") {
        FactorSet default_fs("DefaultFactors");
        // 默认应该是 DAY 类型
        CHECK_NOTHROW(default_fs.add(day_factor));
        CHECK_EQ(default_fs.size(), 1);

        CHECK_THROWS(default_fs.add(week_factor));
        CHECK_EQ(default_fs.size(), 1);
    }
}

/** @par 检测点：测试FactorSet同名因子覆盖 */
TEST_CASE("test_FactorSet_duplicate_name") {
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma5_new = MA(CLOSE(), 5);  // 同名但不同的指标

    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "原始版本");
    Factor factor2("MA5", ma5_new, KQuery::DAY, "5日均线因子", "更新版本");

    FactorSet fs("TestFactorSet");

    // 添加第一个因子
    fs.add(factor1);
    CHECK_EQ(fs.size(), 1);

    // 获取并验证第一个因子
    Factor retrieved1 = fs.get("MA5");
    CHECK_EQ(retrieved1.brief(), "5日均线因子");
    CHECK_EQ(retrieved1.details(), "原始版本");

    // 添加同名因子，应该覆盖原有的
    fs.add(factor2);
    CHECK_EQ(fs.size(), 1);  // 大小不变，因为是覆盖而不是新增

    // 验证被覆盖后的因子
    Factor retrieved2 = fs.get("MA5");
    CHECK_EQ(retrieved2.brief(), "5日均线因子");
    CHECK_EQ(retrieved2.details(), "更新版本");  // 应该是新的值

    // 验证迭代顺序（应该只有一个元素）
    size_t count = 0;
    for (const auto& factor : fs) {
        CHECK_EQ(factor.name(), "MA5");
        CHECK_EQ(factor.details(), "更新版本");
        count++;
    }
    CHECK_EQ(count, 1);
}

/** @par 检测点：测试FactorSet浅拷贝语义 */
TEST_CASE("test_FactorSet_shallow_copy") {
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    Factor factor1("MA5", ma5, KQuery::DAY);
    Factor factor2("MA10", ma10, KQuery::DAY);

    // 创建原始 FactorSet
    FactorSet fs1("Original");
    fs1.add(factor1);
    fs1.add(factor2);
    CHECK_EQ(fs1.size(), 2);

    // 测试拷贝构造（浅拷贝）
    FactorSet fs2(fs1);
    CHECK_EQ(fs2.name(), "ORIGINAL");
    CHECK_EQ(fs2.size(), 2);
    CHECK_UNARY(fs2.have("MA5"));
    CHECK_UNARY(fs2.have("MA10"));

    // 由于是浅拷贝，修改拷贝会影响原对象
    fs2.remove("MA5");

    // 验证原对象也被修改了（浅拷贝特性）
    CHECK_EQ(fs1.size(), 1);       // 原对象大小也被修改
    CHECK_FALSE(fs1.have("MA5"));  // 原对象因子被移除

    // 测试拷贝赋值（浅拷贝）
    FactorSet fs3("Target");
    fs3 = fs1;
    CHECK_EQ(fs3.name(), "ORIGINAL");
    CHECK_EQ(fs3.size(), 1);

    // 测试移动语义 - 在浅拷贝情况下，移动构造实际上是拷贝构造
    FactorSet fs4(std::move(fs2));
    // fs4 应该反映最新的状态，因为所有对象共享同一份数据
    CHECK_EQ(fs4.name(), "ORIGINAL");
    CHECK_EQ(fs4.size(), 1);

    FactorSet fs5("Target2");
    fs5 = std::move(fs3);
    CHECK_EQ(fs5.name(), "ORIGINAL");
    CHECK_EQ(fs5.size(), 1);

    // 测试自赋值（浅拷贝情况下自赋值应该是安全的）
    fs1 = fs1;
    CHECK_EQ(fs1.size(), 1);
}

/** @par 检测点：测试FactorSet迭代器功能 */
TEST_CASE("test_FactorSet_iterator") {
    // 创建测试用的 Factor 对象
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "测试5日均线");
    Factor factor2("MA10", ma10, KQuery::DAY, "10日均线因子", "测试10日均线");

    // 创建 FactorSet 并添加因子
    FactorSet fs("TestFactorSet");
    fs.add(factor1);
    fs.add(factor2);

    CHECK_EQ(fs.size(), 2);
    CHECK_FALSE(fs.empty());

    // 测试基于范围的for循环 - 迭代器应直接返回Factor引用
    size_t count = 0;
    for (const Factor& factor : fs) {
        CHECK_UNARY_FALSE(factor.isNull());
        CHECK_UNARY_FALSE(factor.name().empty());
        CHECK_UNARY(factor.name() == "MA5" || factor.name() == "MA10");
        count++;
    }
    CHECK_EQ(count, 2);

    // 测试显式迭代器 - 解引用应直接返回Factor引用
    auto it = fs.begin();
    auto end = fs.end();
    count = 0;
    while (it != end) {
        const Factor& factor = *it;  // 直接解引用得到Factor引用
        CHECK_UNARY_FALSE(factor.isNull());
        CHECK_UNARY(factor.name() == "MA5" || factor.name() == "MA10");
        count++;
        ++it;
    }
    CHECK_EQ(count, 2);

    // 测试迭代器箭头操作符
    for (auto it = fs.begin(); it != fs.end(); ++it) {
        const Factor* factor_ptr = it.operator->();  // 箭头操作符应返回Factor指针
        CHECK_UNARY_FALSE(factor_ptr->isNull());
        CHECK_UNARY_FALSE(factor_ptr->name().empty());
    }

    // 测试 const_iterator 和 iterator 的等价性
    const FactorSet& const_fs = fs;
    count = 0;
    for (auto it = const_fs.begin(); it != const_fs.end(); ++it) {
        const Factor& factor = *it;
        CHECK_UNARY_FALSE(factor.isNull());
        count++;
    }
    CHECK_EQ(count, 2);

    // 测试 cbegin/cend
    count = 0;
    for (auto it = fs.cbegin(); it != fs.cend(); ++it) {
        const Factor& factor = *it;
        CHECK_UNARY_FALSE(factor.isNull());
        count++;
    }
    CHECK_EQ(count, 2);

    // 测试迭代器比较操作
    auto it1 = fs.begin();
    auto it2 = fs.begin();
    CHECK_UNARY(it1 == it2);
    ++it1;
    CHECK_UNARY(it1 != it2);

    // 测试空集合的迭代器
    FactorSet empty_fs;
    CHECK_UNARY(empty_fs.begin() == empty_fs.end());
    CHECK_UNARY(empty_fs.cbegin() == empty_fs.cend());

    // 测试基于迭代器的因子收集
    vector<Factor> factors;
    for (const Factor& factor : fs) {
        factors.push_back(factor);
    }
    CHECK_EQ(factors.size(), 2);
    // 验证收集到的因子
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

/** @par 检测点：测试FactorSet Block属性功能 */
TEST_CASE("test_FactorSet_block") {
    // 测试默认构造函数的 Block
    FactorSet fs1;
    CHECK_UNARY(fs1.block().isNull());
    CHECK_EQ(fs1.block().size(), 0);

    // 测试带参数的构造函数（使用默认 Block）
    FactorSet fs2("TestSet", KQuery::DAY);
    CHECK_EQ(fs2.name(), "TESTSET");
    CHECK_EQ(fs2.ktype(), KQuery::DAY);
    CHECK_UNARY(fs2.block().isNull());
    CHECK_EQ(fs2.block().size(), 0);

    // 创建测试用的 Block
    Block test_block("行业", "测试板块");

    // 测试带显式 Block 参数的构造函数
    FactorSet fs3("TestWithBlock", KQuery::WEEK, test_block);
    CHECK_EQ(fs3.name(), "TESTWITHBLOCK");
    CHECK_EQ(fs3.ktype(), KQuery::WEEK);
    CHECK_FALSE(fs3.block().isNull());
    CHECK_EQ(fs3.block().category(), "行业");
    CHECK_EQ(fs3.block().name(), "测试板块");
    CHECK_EQ(fs3.block().size(), 0);  // 新创建的 Block 应该是空的

    // 测试 Block setter/getter
    Block new_block("概念", "新概念板块");
    fs1.block(new_block);
    CHECK_FALSE(fs1.block().isNull());
    CHECK_EQ(fs1.block().category(), "概念");
    CHECK_EQ(fs1.block().name(), "新概念板块");

    // 测试拷贝语义下的 Block 共享
    FactorSet fs4(fs3);
    CHECK_EQ(fs4.block().category(), "行业");
    CHECK_EQ(fs4.block().name(), "测试板块");

    // 修改拷贝的 Block 应该影响原对象（浅拷贝）
    Block modified_block = fs4.block();
    modified_block.category("地域");
    modified_block.name("修改后的板块");
    fs4.block(modified_block);

    CHECK_EQ(fs3.block().category(), "地域");
    CHECK_EQ(fs3.block().name(), "修改后的板块");
}

/** @par 检测点：测试FactorSet block匹配检查 */
TEST_CASE("test_FactorSet_block_check") {
    // 创建测试用的 Blocks
    Block block1("行业", "科技板块");
    Block block2("行业", "金融板块");
    Block block3("概念", "新能源概念");

    // 创建测试因子（注意Block参数在最后）
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor1("MA5_B1", ma5, KQuery::DAY, "MA5因子", "block1测试", false, Datetime::min(),
                   block1);
    Factor factor2("MA5_B2", ma5, KQuery::DAY, "MA5因子", "block2测试", false, Datetime::min(),
                   block2);
    Factor factor3("MA5_B3", ma5, KQuery::DAY, "MA5因子", "block3测试", false, Datetime::min(),
                   block3);

    // 测试使用 block1 构造的 FactorSet
    SUBCASE("FactorSet with block1") {
        FactorSet fs("TestFS", KQuery::DAY, block1);

        // 应该能正常添加匹配 block1 的因子
        CHECK_NOTHROW(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);

        // 添加不匹配 block1 的因子应该抛出异常
        CHECK_THROWS(fs.add(factor2));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变

        CHECK_THROWS(fs.add(factor3));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变
    }

    // 测试使用 block2 构造的 FactorSet
    SUBCASE("FactorSet with block2") {
        FactorSet fs("TestFS", KQuery::DAY, block2);

        // 应该能正常添加匹配 block2 的因子
        CHECK_NOTHROW(fs.add(factor2));
        CHECK_EQ(fs.size(), 1);

        // 添加不匹配 block2 的因子应该抛出异常
        CHECK_THROWS(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变

        CHECK_THROWS(fs.add(factor3));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变
    }

    // 测试使用 block3 构造的 FactorSet
    SUBCASE("FactorSet with block3") {
        FactorSet fs("TestFS", KQuery::DAY, block3);

        // 应该能正常添加匹配 block3 的因子
        CHECK_NOTHROW(fs.add(factor3));
        CHECK_EQ(fs.size(), 1);

        // 添加不匹配 block3 的因子应该抛出异常
        CHECK_THROWS(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变

        CHECK_THROWS(fs.add(factor2));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变
    }

    // 测试默认构造的 FactorSet（空 Block）
    SUBCASE("FactorSet with default empty block") {
        FactorSet fs("TestFS");

        // 创建一个空 Block 的因子
        Block empty_block;
        Factor factor_empty("EMPTY", ma5, KQuery::DAY, "空Block因子", "", false, Datetime::min(),
                            empty_block);

        // 应该能正常添加空 Block 的因子
        CHECK_NOTHROW(fs.add(factor_empty));
        CHECK_EQ(fs.size(), 1);

        // 添加非空 Block 的因子应该抛出异常
        CHECK_THROWS(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);

        CHECK_THROWS(fs.add(factor2));
        CHECK_EQ(fs.size(), 1);

        CHECK_THROWS(fs.add(factor3));
        CHECK_EQ(fs.size(), 1);
    }

    // 测试运行时修改 Block 的情况
    SUBCASE("FactorSet with runtime block modification") {
        FactorSet fs("TestFS", KQuery::DAY, block1);

        // 先添加一个匹配的因子
        CHECK_NOTHROW(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);

        // 修改 FactorSet 的 Block
        fs.block(block2);

        // 现在添加原来匹配但现在不匹配的因子应该失败
        CHECK_THROWS(fs.add(factor1));
        CHECK_EQ(fs.size(), 1);

        // 添加新的匹配因子应该成功
        CHECK_NOTHROW(fs.add(factor2));
        CHECK_EQ(fs.size(), 2);
    }
}

/** @par 检测点：测试FactorSet因子顺序保持功能 */
TEST_CASE("test_FactorSet_order_preservation") {
    // 创建测试用的 Indicator
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);
    Indicator ma20 = MA(CLOSE(), 20);

    // 创建 Factor 对象
    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子");
    Factor factor2("MA10", ma10, KQuery::DAY, "10日均线因子");
    Factor factor3("MA20", ma20, KQuery::DAY, "20日均线因子");

    // 创建 FactorSet 并按特定顺序添加因子
    FactorSet fs("ORDER_TEST", KQuery::DAY);
    fs.add(factor1);  // 第1个添加
    fs.add(factor2);  // 第2个添加
    fs.add(factor3);  // 第3个添加

    CHECK_EQ(fs.size(), 3);

    // 验证迭代顺序与添加顺序一致
    vector<string> expected_order{"MA5", "MA10", "MA20"};
    size_t index = 0;
    for (const auto& factor : fs) {
        CHECK_EQ(factor.name(), expected_order[index]);
        index++;
    }
    CHECK_EQ(index, 3);

    // 测试重复添加相同名称的因子
    SUBCASE("Duplicate factor name handling") {
        Factor duplicate_factor("MA5", ma5, KQuery::DAY, "重复的5日均线");
        fs.add(duplicate_factor);
        // 大小应该保持不变
        CHECK_EQ(fs.size(), 3);
        // 顺序应该保持不变
        index = 0;
        for (const auto& factor : fs) {
            CHECK_EQ(factor.name(), expected_order[index]);
            index++;
        }
        CHECK_EQ(index, 3);
    }

    // 测试删除最后一个因子
    SUBCASE("Remove last factor") {
        fs.remove("MA20");  // 删除最后一个因子
        CHECK_EQ(fs.size(), 2);

        vector<string> expected_after_removal{"MA5", "MA10"};
        index = 0;
        for (const auto& factor : fs) {
            CHECK_EQ(factor.name(), expected_after_removal[index]);
            index++;
        }
        CHECK_EQ(index, 2);

        // 验证剩余因子仍可正常访问
        CHECK_UNARY(fs.have("MA5"));
        CHECK_UNARY(fs.have("MA10"));
        CHECK_FALSE(fs.have("MA20"));
    }

    // 测试删除中间因子（关键测试点）
    SUBCASE("Remove middle factor") {
        fs.remove("MA10");  // 删除中间的因子
        CHECK_EQ(fs.size(), 2);

        // 验证顺序：第一个和第三个因子应该保持，第二个被移除
        vector<string> expected_after_removal{"MA5", "MA20"};
        index = 0;
        for (const auto& factor : fs) {
            CHECK_EQ(factor.name(), expected_after_removal[index]);
            index++;
        }
        CHECK_EQ(index, 2);

        // 验证剩余因子仍可正常访问
        CHECK_UNARY(fs.have("MA5"));
        CHECK_UNARY(fs.have("MA20"));
        CHECK_FALSE(fs.have("MA10"));

        // 验证可以通过名称获取因子
        Factor retrieved_factor = fs.get("MA20");
        CHECK_EQ(retrieved_factor.name(), "MA20");
    }

    // 测试删除所有因子
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

/** @par 检测点：测试FactorSet getValues功能 */
TEST_CASE("test_FactorSet_getValues") {
    // 创建测试用的 Indicator
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    // 创建 Factor 对象
    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子");
    Factor factor2("MA10", ma10, KQuery::DAY, "10日均线因子");

    // 创建 FactorSet
    FactorSet fs("TEST_SET", KQuery::DAY);
    fs.add(factor1);
    fs.add(factor2);

    CHECK_EQ(fs.size(), 2);

    // 创建测试股票列表
    StockList stocks;
    // 由于没有初始化 StockManager，这里使用空列表进行测试
    // 实际使用时应该包含有效的股票对象

    KQuery query(0, Null<int64_t>(), KQuery::DAY);

    // 测试 getValues 基本功能
    SUBCASE("Basic getValues functionality") {
        vector<IndicatorList> results = fs.getValues(stocks, query, false);
        // 结果数量应该等于股票数量
        CHECK_EQ(results.size(), stocks.size());
        // 每个股票的结果应该包含所有因子的计算结果
        for (const auto& stock_results : results) {
            CHECK_EQ(stock_results.size(), fs.size());
        }
    }

    // 测试 check 参数功能
    SUBCASE("Check parameter functionality") {
        // 不进行 block 检查时应该正常工作
        CHECK_NOTHROW(fs.getValues(stocks, query, false));

        // 当前 FactorSet 没有设置 block，所以 check=true 也应该正常工作
        CHECK_NOTHROW(fs.getValues(stocks, query, true));
    }

    // 测试空 FactorSet 情况
    SUBCASE("Empty FactorSet") {
        FactorSet empty_fs("EMPTY", KQuery::DAY);
        vector<IndicatorList> results = empty_fs.getValues(stocks, query, false);
        CHECK_EQ(results.size(), stocks.size());
        // 空 FactorSet 的结果应该是空的 IndicatorList
        for (const auto& stock_results : results) {
            CHECK_EQ(stock_results.size(), 0);
        }
    }
}

/** @par 检测点：测试FactorSet边界条件和异常处理 */
TEST_CASE("test_FactorSet_edge_cases_and_exceptions") {
    // 测试空名称构造
    SUBCASE("Empty name construction") {
        FactorSet fs("", KQuery::DAY);
        CHECK_EQ(fs.name(), "");
        CHECK_EQ(fs.ktype(), KQuery::DAY);
        CHECK_EQ(fs.size(), 0);
        CHECK_UNARY(fs.empty());
    }

    // 测试极端长名称
    SUBCASE("Very long name") {
        string long_name(1000, 'A');
        FactorSet fs(long_name, KQuery::DAY);
        CHECK_EQ(fs.name(), long_name);
        CHECK_EQ(fs.ktype(), KQuery::DAY);
    }

    // 测试大量因子的情况
    SUBCASE("Large number of factors") {
        FactorSet fs("LARGE_SET", KQuery::DAY);
        Indicator base_indicator = CLOSE();

        // 添加大量因子（测试性能和稳定性）
        const size_t factor_count = 100;
        for (size_t i = 0; i < factor_count; ++i) {
            string name = "FACTOR_" + std::to_string(i);
            Indicator ind = MA(base_indicator, static_cast<int>(i + 5));
            Factor factor(name, ind, KQuery::DAY);
            CHECK_NOTHROW(fs.add(factor));
        }

        CHECK_EQ(fs.size(), factor_count);

        // 测试随机访问
        for (size_t i = 0; i < factor_count; i += 10) {
            string name = "FACTOR_" + std::to_string(i);
            CHECK_UNARY(fs.have(name));
            const Factor& factor = fs.get(name);
            CHECK_EQ(factor.name(), name);
        }

        // 测试迭代器遍历
        size_t count = 0;
        for (const auto& factor : fs) {
            CHECK_FALSE(factor.name().empty());
            count++;
        }
        CHECK_EQ(count, factor_count);
    }

    // 测试重复添加大量同名因子
    SUBCASE("Massive duplicate additions") {
        FactorSet fs("DUPLICATE_TEST", KQuery::DAY);
        Indicator ma5 = MA(CLOSE(), 5);
        Factor base_factor("SAME_NAME", ma5, KQuery::DAY);

        const size_t iterations = 1000;
        for (size_t i = 0; i < iterations; ++i) {
            CHECK_NOTHROW(fs.add(base_factor));
            // 大小应该始终保持为1
            CHECK_EQ(fs.size(), 1);
        }

        CHECK_EQ(fs.size(), 1);
        CHECK_UNARY(fs.have("SAME_NAME"));
    }

    // 测试并发访问（模拟多线程环境）
    SUBCASE("Concurrent-like access simulation") {
        FactorSet fs("CONCURRENT_TEST", KQuery::DAY);
        Indicator ma5 = MA(CLOSE(), 5);
        Indicator ma10 = MA(CLOSE(), 10);

        Factor factor1("MA5", ma5, KQuery::DAY);
        Factor factor2("MA10", ma10, KQuery::DAY);

        // 模拟并发添加
        fs.add(factor1);
        fs.add(factor2);

        // 模拟并发读取
        CHECK_UNARY(fs.have("MA5"));
        CHECK_UNARY(fs.have("MA10"));

        const Factor& f1 = fs.get("MA5");
        const Factor& f2 = fs.get("MA10");

        CHECK_EQ(f1.name(), "MA5");
        CHECK_EQ(f2.name(), "MA10");

        // 模拟并发删除
        fs.remove("MA5");
        CHECK_FALSE(fs.have("MA5"));
        CHECK_EQ(fs.size(), 1);
    }
}

/** @par 检测点：测试FactorSet性能相关功能 */
TEST_CASE("test_FactorSet_performance_features") {
    // 测试不同K线类型的FactorSet性能
    SUBCASE("Different KType performance") {
        Indicator ma5 = MA(CLOSE(), 5);

        // 日线FactorSet
        FactorSet day_fs("DAY_PERF", KQuery::DAY);
        Factor day_factor("DAY_MA5", ma5, KQuery::DAY);
        CHECK_NOTHROW(day_fs.add(day_factor));
        CHECK_EQ(day_fs.size(), 1);

        // 周线FactorSet
        FactorSet week_fs("WEEK_PERF", KQuery::WEEK);
        Factor week_factor("WEEK_MA5", ma5, KQuery::WEEK);
        CHECK_NOTHROW(week_fs.add(week_factor));
        CHECK_EQ(week_fs.size(), 1);

        // 月线FactorSet
        FactorSet month_fs("MONTH_PERF", KQuery::MONTH);
        Factor month_factor("MONTH_MA5", ma5, KQuery::MONTH);
        CHECK_NOTHROW(month_fs.add(month_factor));
        CHECK_EQ(month_fs.size(), 1);

        // 季线FactorSet
        FactorSet quarter_fs("QUARTER_PERF", KQuery::QUARTER);
        Factor quarter_factor("QUARTER_MA5", ma5, KQuery::QUARTER);
        CHECK_NOTHROW(quarter_fs.add(quarter_factor));
        CHECK_EQ(quarter_fs.size(), 1);

        // 年线FactorSet
        FactorSet year_fs("YEAR_PERF", KQuery::YEAR);
        Factor year_factor("YEAR_MA5", ma5, KQuery::YEAR);
        CHECK_NOTHROW(year_fs.add(year_factor));
        CHECK_EQ(year_fs.size(), 1);
    }

    // 测试复杂指标组合的性能
    SUBCASE("Complex indicator combination performance") {
        FactorSet fs("COMPLEX_PERF", KQuery::DAY);

        // 创建复杂的指标组合
        Indicator close = CLOSE();
        Indicator open = OPEN();
        Indicator high = HIGH();
        Indicator low = LOW();
        Indicator vol = VOL();

        // 添加各种复杂因子
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

        // 验证所有因子都能正确访问
        for (const auto& item : indicators) {
            CHECK_UNARY(fs.have(item.first));
            const Factor& factor = fs.get(item.first);
            CHECK_EQ(factor.name(), item.first);
        }
    }
}

/** @par 检测点：测试FactorSet内存管理和资源释放 */
TEST_CASE("test_FactorSet_memory_management") {
    // 测试大量临时对象的内存管理
    SUBCASE("Temporary object memory management") {
        vector<FactorSet> factor_sets;

        // 创建多个FactorSet实例
        const size_t set_count = 50;
        for (size_t i = 0; i < set_count; ++i) {
            string name = "TEMP_SET_" + std::to_string(i);
            FactorSet fs(name, KQuery::DAY);

            // 为每个FactorSet添加几个因子
            for (size_t j = 0; j < 3; ++j) {
                string factor_name = "FACTOR_" + std::to_string(j);
                Indicator ind = MA(CLOSE(), static_cast<int>(j + 5));
                Factor factor(factor_name, ind, KQuery::DAY);
                fs.add(factor);
            }

            factor_sets.push_back(std::move(fs));
        }

        // 验证所有FactorSet都正确创建
        CHECK_EQ(factor_sets.size(), set_count);
        for (size_t i = 0; i < set_count; ++i) {
            CHECK_EQ(factor_sets[i].size(), 3);
            CHECK_EQ(factor_sets[i].name(), "TEMP_SET_" + std::to_string(i));
        }

        // 清理
        factor_sets.clear();
    }

    // 测试循环引用检测
    SUBCASE("Circular reference detection") {
        FactorSet fs1("SET1", KQuery::DAY);
        FactorSet fs2("SET2", KQuery::DAY);

        // 正常的操作不应该导致循环引用
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor1("MA5_1", ma5, KQuery::DAY);
        Factor factor2("MA5_2", ma5, KQuery::DAY);

        CHECK_NOTHROW(fs1.add(factor1));
        CHECK_NOTHROW(fs2.add(factor2));

        // 验证两个FactorSet互相独立
        CHECK_EQ(fs1.size(), 1);
        CHECK_EQ(fs2.size(), 1);
        CHECK_NE(&fs1, &fs2);
    }
}

/** @par 检测点：测试FactorSet与其他系统的集成 */
TEST_CASE("test_FactorSet_integration") {
    // 测试与StockManager的集成
    SUBCASE("StockManager integration") {
        FactorSet fs("STOCK_INTEGRATION", KQuery::DAY);
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor("MA5_INTEGRATION", ma5, KQuery::DAY);

        CHECK_NOTHROW(fs.add(factor));
        CHECK_EQ(fs.size(), 1);

        // 测试getAllValues（会调用StockManager）
        KQuery query(0, 10, KQuery::DAY);
        CHECK_NOTHROW(fs.getAllValues(query));
        // 注意：实际结果取决于StockManager的状态，这里只测试不抛异常
    }

    // 测试与Block系统的集成
    SUBCASE("Block system integration") {
        // 创建一个真实的Block（如果可能的话）
        Block real_block("行业", "真实测试板块");

        FactorSet fs("BLOCK_INTEGRATION", KQuery::DAY, real_block);
        CHECK_EQ(fs.block().category(), "行业");
        CHECK_EQ(fs.block().name(), "真实测试板块");

        // 测试Block相关操作
        Block new_block("概念", "新概念");
        fs.block(new_block);
        CHECK_EQ(fs.block().category(), "概念");
        CHECK_EQ(fs.block().name(), "新概念");
    }
}

#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点：测试FactorSet序列化功能 */
TEST_CASE("test_FactorSet_serialization") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/FactorSet.xml";

    // 创建测试用的 Factor 对象
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    // 创建测试用的 Block
    Block test_block("行业", "测试板块");

    Factor factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "测试5日均线", false, Datetime::min(),
                   test_block);
    Factor factor2("MA10", ma10, KQuery::DAY, "10日均线因子", "测试10日均线", false,
                   Datetime::min(), test_block);

    // 测试基本序列化 - 空FactorSet
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

    // 测试带因子的序列化
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

        // 验证因子也被正确序列化
        CHECK_UNARY(fs2.have("MA5"));
        CHECK_UNARY(fs2.have("MA10"));

        Factor retrieved_ma5 = fs2.get("MA5");
        CHECK_EQ(retrieved_ma5.name(), "MA5");
        CHECK_EQ(retrieved_ma5.brief(), "5日均线因子");

        Factor retrieved_ma10 = fs2.get("MA10");
        CHECK_EQ(retrieved_ma10.name(), "MA10");
        CHECK_EQ(retrieved_ma10.brief(), "10日均线因子");
    }

    // 测试不同K线类型的序列化
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

    // 测试空名称的序列化
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

    // 测试修改后的序列化
    SUBCASE("Modified FactorSet serialization") {
        FactorSet fs1("MODIFY_TEST", KQuery::DAY, test_block);
        fs1.add(factor1);

        {
            std::ofstream ofs(filename);
            boost::archive::xml_oarchive oa(ofs);
            oa << BOOST_SERIALIZATION_NVP(fs1);
        }

        // 修改原始对象
        fs1.add(factor2);
        fs1.remove("MA5");

        FactorSet fs2;
        {
            std::ifstream ifs(filename);
            boost::archive::xml_iarchive ia(ifs);
            ia >> BOOST_SERIALIZATION_NVP(fs2);
        }

        // 验证反序列化的对象是保存时的状态，不受后续修改影响
        CHECK_EQ(fs2.name(), "MODIFY_TEST");
        CHECK_EQ(fs2.size(), 1);
        CHECK_UNARY(fs2.have("MA5"));
        CHECK_FALSE(fs2.have("MA10"));
    }

    // 测试Block为空的序列化
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
