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
    CHECK_THROWS_AS(fs.getFactor("NONEXIST"), hku::exception);
    
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