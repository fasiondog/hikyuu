/*
 * test_FactorSet.cpp
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/factor/FactorSet.h>
#include <hikyuu/factor/FactorMeta.h>
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
    // 创建测试用的 FactorMeta 对象
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);
    
    FactorMeta factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "测试5日均线");
    FactorMeta factor2("MA10", ma10, KQuery::DAY, "10日均线因子", "测试10日均线");
    
    // 测试构造函数和基本属性
    FactorSet fs("TestFactorSet");
    CHECK_EQ(fs.name(), "TestFactorSet");
    
    fs.name("NewName");
    CHECK_EQ(fs.name(), "NewName");
    
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
    FactorMeta retrieved1 = fs.getFactor("MA5");
    CHECK_EQ(retrieved1.name(), "MA5");
    CHECK_EQ(retrieved1.brief(), "5日均线因子");
    
    FactorMeta retrieved2 = fs.getFactor("MA10");
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
    
    FactorMeta day_factor("DAY_MA", day_ma, KQuery::DAY, "日线因子");
    FactorMeta week_factor("WEEK_MA", week_ma, KQuery::WEEK, "周线因子");
    FactorMeta month_factor("MONTH_MA", month_ma, KQuery::MONTH, "月线因子");
    
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
    
    FactorMeta factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "原始版本");
    FactorMeta factor2("MA5", ma5_new, KQuery::DAY, "5日均线因子", "更新版本");
    
    FactorSet fs("TestFactorSet");
    
    // 添加第一个因子
    fs.addFactor(factor1);
    CHECK_EQ(fs.size(), 1);
    
    // 获取并验证第一个因子
    FactorMeta retrieved1 = fs.getFactor("MA5");
    CHECK_EQ(retrieved1.brief(), "5日均线因子");
    CHECK_EQ(retrieved1.details(), "原始版本");
    
    // 添加同名因子，应该覆盖原有的
    fs.addFactor(factor2);
    CHECK_EQ(fs.size(), 1);  // 大小不变，因为是覆盖而不是新增
    
    // 验证被覆盖后的因子
    FactorMeta retrieved2 = fs.getFactor("MA5");
    CHECK_EQ(retrieved2.brief(), "5日均线因子");
    CHECK_EQ(retrieved2.details(), "更新版本");  // 应该是新的值
}

/** @par 检测点：测试FactorSet浅拷贝语义 */
TEST_CASE("test_FactorSet_shallow_copy") {
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);
    
    FactorMeta factor1("MA5", ma5, KQuery::DAY);
    FactorMeta factor2("MA10", ma10, KQuery::DAY);
    
    // 创建原始 FactorSet
    FactorSet fs1("Original");
    fs1.addFactor(factor1);
    fs1.addFactor(factor2);
    CHECK_EQ(fs1.size(), 2);
    
    // 测试拷贝构造（浅拷贝）
    FactorSet fs2(fs1);
    CHECK_EQ(fs2.name(), "Original");
    CHECK_EQ(fs2.size(), 2);
    CHECK_UNARY(fs2.hasFactor("MA5"));
    CHECK_UNARY(fs2.hasFactor("MA10"));
    
    // 由于是浅拷贝，修改拷贝会影响原对象
    fs2.name("ModifiedCopy");
    fs2.removeFactor("MA5");
    
    // 验证原对象也被修改了（浅拷贝特性）
    CHECK_EQ(fs1.name(), "ModifiedCopy");  // 原对象名称被修改
    CHECK_EQ(fs1.size(), 1);               // 原对象大小也被修改
    CHECK_FALSE(fs1.hasFactor("MA5"));     // 原对象因子被移除
    
    // 测试拷贝赋值（浅拷贝）
    FactorSet fs3("Target");
    fs3 = fs1;
    CHECK_EQ(fs3.name(), "ModifiedCopy");
    CHECK_EQ(fs3.size(), 1);
    
    // 修改fs3也会影响fs1（因为它们共享同一份数据）
    fs3.name("AnotherModification");
    CHECK_EQ(fs1.name(), "AnotherModification");
    
    // 测试移动语义 - 在浅拷贝情况下，移动构造实际上是拷贝构造
    FactorSet fs4(std::move(fs2));
    // fs4 应该反映最新的状态，因为所有对象共享同一份数据
    CHECK_EQ(fs4.name(), "AnotherModification");
    CHECK_EQ(fs4.size(), 1);
    
    FactorSet fs5("Target2");
    fs5 = std::move(fs3);
    CHECK_EQ(fs5.name(), "AnotherModification");
    CHECK_EQ(fs5.size(), 1);
    
    // 测试自赋值（浅拷贝情况下自赋值应该是安全的）
    fs1 = fs1;
    CHECK_EQ(fs1.size(), 1);
}

/** @par 检测点：测试FactorSet迭代器功能 */
TEST_CASE("test_FactorSet_iterator") {
    // 创建测试用的 FactorMeta 对象
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);
    
    FactorMeta factor1("MA5", ma5, KQuery::DAY, "5日均线因子", "测试5日均线");
    FactorMeta factor2("MA10", ma10, KQuery::DAY, "10日均线因子", "测试10日均线");
    
    // 创建 FactorSet 并添加因子
    FactorSet fs("TestFactorSet");
    fs.addFactor(factor1);
    fs.addFactor(factor2);
    
    CHECK_EQ(fs.size(), 2);
    CHECK_FALSE(fs.empty());
    
    // 测试基于范围的for循环
    size_t count = 0;
    for (const auto& pair : fs) {
        const string& name = pair.first;
        const FactorMeta& factor = pair.second;
        CHECK_UNARY(name == "MA5" || name == "MA10");
        CHECK_UNARY(factor.name() == name);
        count++;
    }
    CHECK_EQ(count, 2);
    
    // 测试显式迭代器
    auto it = fs.begin();
    auto end = fs.end();
    count = 0;
    while (it != end) {
        const auto& pair = *it;
        CHECK_UNARY(pair.first == "MA5" || pair.first == "MA10");
        count++;
        ++it;
    }
    CHECK_EQ(count, 2);
    
    // 测试 const_iterator 和 iterator 的等价性
    const FactorSet& const_fs = fs;
    count = 0;
    for (auto it = const_fs.begin(); it != const_fs.end(); ++it) {
        count++;
    }
    CHECK_EQ(count, 2);
    
    // 测试 cbegin/cend
    count = 0;
    for (auto it = fs.cbegin(); it != fs.cend(); ++it) {
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
    
    // 测试基于迭代器的算法
    vector<string> names;
    for (const auto& pair : fs) {
        names.push_back(pair.first);
    }
    CHECK_EQ(names.size(), 2);
    CHECK_UNARY(std::find(names.begin(), names.end(), "MA5") != names.end());
    CHECK_UNARY(std::find(names.begin(), names.end(), "MA10") != names.end());
}

/** @} */
