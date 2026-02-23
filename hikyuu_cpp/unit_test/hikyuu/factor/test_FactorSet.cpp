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
#include <algorithm>

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
    fs.addFactor(factor1);
    fs.addFactor(factor2);

    CHECK_EQ(fs.size(), 2);
    CHECK_FALSE(fs.empty());

    // 测试因子查询
    CHECK_UNARY(fs.hasFactor("MA5"));
    CHECK_UNARY(fs.hasFactor("MA10"));
    CHECK_FALSE(fs.hasFactor("NONEXIST"));

    // 测试获取因子
    Factor retrieved1 = fs.getFactor("MA5");
    CHECK_EQ(retrieved1.name(), "MA5");
    CHECK_EQ(retrieved1.brief(), "5日均线因子");

    Factor retrieved2 = fs.getFactor("MA10");
    CHECK_EQ(retrieved2.name(), "MA10");
    CHECK_EQ(retrieved2.brief(), "10日均线因子");

    // 测试不存在的因子
    CHECK_THROWS(fs.getFactor("NONEXIST"));

    // 测试移除因子
    fs.removeFactor("MA5");
    CHECK_EQ(fs.size(), 1);
    CHECK_FALSE(fs.hasFactor("MA5"));
    CHECK_UNARY(fs.hasFactor("MA10"));

    // 测试清空
    fs.clear();
    CHECK_EQ(fs.size(), 0);
    CHECK_UNARY(fs.empty());
    CHECK_FALSE(fs.hasFactor("MA10"));
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
        CHECK_NOTHROW(day_fs.addFactor(day_factor));
        CHECK_EQ(day_fs.size(), 1);

        // 添加周线因子应该抛出异常
        CHECK_THROWS(day_fs.addFactor(week_factor));
        CHECK_EQ(day_fs.size(), 1);  // 大小应该不变

        // 添加月线因子也应该抛出异常
        CHECK_THROWS(day_fs.addFactor(month_factor));
        CHECK_EQ(day_fs.size(), 1);  // 大小应该不变
    }

    // 测试周线 FactorSet
    SUBCASE("WEEK FactorSet") {
        FactorSet week_fs("WeekFactors", KQuery::WEEK);

        // 应该能正常添加周线因子
        CHECK_NOTHROW(week_fs.addFactor(week_factor));
        CHECK_EQ(week_fs.size(), 1);

        // 添加日线因子应该抛出异常
        CHECK_THROWS(week_fs.addFactor(day_factor));
        CHECK_EQ(week_fs.size(), 1);  // 大小应该不变

        // 添加月线因子也应该抛出异常
        CHECK_THROWS(week_fs.addFactor(month_factor));
        CHECK_EQ(week_fs.size(), 1);  // 大小应该不变
    }

    // 测试月线 FactorSet
    SUBCASE("MONTH FactorSet") {
        FactorSet month_fs("MonthFactors", KQuery::MONTH);

        // 应该能正常添加月线因子
        CHECK_NOTHROW(month_fs.addFactor(month_factor));
        CHECK_EQ(month_fs.size(), 1);

        // 添加日线因子应该抛出异常
        CHECK_THROWS(month_fs.addFactor(day_factor));
        CHECK_EQ(month_fs.size(), 1);  // 大小应该不变

        // 添加周线因子也应该抛出异常
        CHECK_THROWS(month_fs.addFactor(week_factor));
        CHECK_EQ(month_fs.size(), 1);  // 大小应该不变
    }

    // 测试默认构造的 FactorSet (DAY)
    SUBCASE("Default FactorSet") {
        FactorSet default_fs("DefaultFactors");
        // 默认应该是 DAY 类型
        CHECK_NOTHROW(default_fs.addFactor(day_factor));
        CHECK_EQ(default_fs.size(), 1);

        CHECK_THROWS(default_fs.addFactor(week_factor));
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
    fs.addFactor(factor1);
    CHECK_EQ(fs.size(), 1);

    // 获取并验证第一个因子
    Factor retrieved1 = fs.getFactor("MA5");
    CHECK_EQ(retrieved1.brief(), "5日均线因子");
    CHECK_EQ(retrieved1.details(), "原始版本");

    // 添加同名因子，应该覆盖原有的
    fs.addFactor(factor2);
    CHECK_EQ(fs.size(), 1);  // 大小不变，因为是覆盖而不是新增

    // 验证被覆盖后的因子
    Factor retrieved2 = fs.getFactor("MA5");
    CHECK_EQ(retrieved2.brief(), "5日均线因子");
    CHECK_EQ(retrieved2.details(), "更新版本");  // 应该是新的值
}

/** @par 检测点：测试FactorSet浅拷贝语义 */
TEST_CASE("test_FactorSet_shallow_copy") {
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);

    Factor factor1("MA5", ma5, KQuery::DAY);
    Factor factor2("MA10", ma10, KQuery::DAY);

    // 创建原始 FactorSet
    FactorSet fs1("Original");
    fs1.addFactor(factor1);
    fs1.addFactor(factor2);
    CHECK_EQ(fs1.size(), 2);

    // 测试拷贝构造（浅拷贝）
    FactorSet fs2(fs1);
    CHECK_EQ(fs2.name(), "ORIGINAL");
    CHECK_EQ(fs2.size(), 2);
    CHECK_UNARY(fs2.hasFactor("MA5"));
    CHECK_UNARY(fs2.hasFactor("MA10"));

    // 由于是浅拷贝，修改拷贝会影响原对象
    fs2.removeFactor("MA5");

    // 验证原对象也被修改了（浅拷贝特性）
    CHECK_EQ(fs1.size(), 1);            // 原对象大小也被修改
    CHECK_FALSE(fs1.hasFactor("MA5"));  // 原对象因子被移除

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
    fs.addFactor(factor1);
    fs.addFactor(factor2);

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
    Factor factor1("MA5_B1", ma5, KQuery::DAY, "MA5因子", "block1测试", false, Datetime::min(), block1);
    Factor factor2("MA5_B2", ma5, KQuery::DAY, "MA5因子", "block2测试", false, Datetime::min(), block2);
    Factor factor3("MA5_B3", ma5, KQuery::DAY, "MA5因子", "block3测试", false, Datetime::min(), block3);
    
    // 测试使用 block1 构造的 FactorSet
    SUBCASE("FactorSet with block1") {
        FactorSet fs("TestFS", KQuery::DAY, block1);
        
        // 应该能正常添加匹配 block1 的因子
        CHECK_NOTHROW(fs.addFactor(factor1));
        CHECK_EQ(fs.size(), 1);
        
        // 添加不匹配 block1 的因子应该抛出异常
        CHECK_THROWS(fs.addFactor(factor2));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变
        
        CHECK_THROWS(fs.addFactor(factor3));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变
    }
    
    // 测试使用 block2 构造的 FactorSet
    SUBCASE("FactorSet with block2") {
        FactorSet fs("TestFS", KQuery::DAY, block2);
        
        // 应该能正常添加匹配 block2 的因子
        CHECK_NOTHROW(fs.addFactor(factor2));
        CHECK_EQ(fs.size(), 1);
        
        // 添加不匹配 block2 的因子应该抛出异常
        CHECK_THROWS(fs.addFactor(factor1));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变
        
        CHECK_THROWS(fs.addFactor(factor3));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变
    }
    
    // 测试使用 block3 构造的 FactorSet
    SUBCASE("FactorSet with block3") {
        FactorSet fs("TestFS", KQuery::DAY, block3);
        
        // 应该能正常添加匹配 block3 的因子
        CHECK_NOTHROW(fs.addFactor(factor3));
        CHECK_EQ(fs.size(), 1);
        
        // 添加不匹配 block3 的因子应该抛出异常
        CHECK_THROWS(fs.addFactor(factor1));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变
        
        CHECK_THROWS(fs.addFactor(factor2));
        CHECK_EQ(fs.size(), 1);  // 大小应该不变
    }
    
    // 测试默认构造的 FactorSet（空 Block）
    SUBCASE("FactorSet with default empty block") {
        FactorSet fs("TestFS");
        
        // 创建一个空 Block 的因子
        Block empty_block;
        Factor factor_empty("EMPTY", ma5, KQuery::DAY, "空Block因子", "", false, Datetime::min(), empty_block);
        
        // 应该能正常添加空 Block 的因子
        CHECK_NOTHROW(fs.addFactor(factor_empty));
        CHECK_EQ(fs.size(), 1);
        
        // 添加非空 Block 的因子应该抛出异常
        CHECK_THROWS(fs.addFactor(factor1));
        CHECK_EQ(fs.size(), 1);
        
        CHECK_THROWS(fs.addFactor(factor2));
        CHECK_EQ(fs.size(), 1);
        
        CHECK_THROWS(fs.addFactor(factor3));
        CHECK_EQ(fs.size(), 1);
    }
    
    // 测试运行时修改 Block 的情况
    SUBCASE("FactorSet with runtime block modification") {
        FactorSet fs("TestFS", KQuery::DAY, block1);
        
        // 先添加一个匹配的因子
        CHECK_NOTHROW(fs.addFactor(factor1));
        CHECK_EQ(fs.size(), 1);
        
        // 修改 FactorSet 的 Block
        fs.block(block2);
        
        // 现在添加原来匹配但现在不匹配的因子应该失败
        CHECK_THROWS(fs.addFactor(factor1));
        CHECK_EQ(fs.size(), 1);
        
        // 添加新的匹配因子应该成功
        CHECK_NOTHROW(fs.addFactor(factor2));
        CHECK_EQ(fs.size(), 2);
    }
}

/** @} */
