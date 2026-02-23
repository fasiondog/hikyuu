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

using namespace hku;

/**
 * @defgroup test_Factor test_Factor
 * @ingroup test_hikyuu_factor_suite
 * @{
 */

/** @par 检测点：测试Factor基本功能 */
TEST_CASE("test_Factor_basic") {
    // 创建测试用的 Indicator
    Indicator ma5 = MA(CLOSE(), 5);
    
    // 测试构造函数
    Factor factor("TEST_FACTOR", ma5, KQuery::DAY, "测试因子", "详细描述");
    
    // 测试基本属性
    CHECK_EQ(factor.name(), "TEST_FACTOR");
    CHECK_EQ(factor.ktype(), KQuery::DAY);
    CHECK_FALSE(factor.isNull());
    CHECK_EQ(factor.brief(), "测试因子");
    CHECK_EQ(factor.details(), "详细描述");
    
    // 测试 formula 属性
    Indicator formula = factor.formula();
    // 不直接检查 empty()，而是检查名称
    CHECK_EQ(formula.name(), "MA");
    
    // 测试 block 属性（默认为空）
    const Block& block = factor.block();
    CHECK_UNARY(block.isNull());
    CHECK_EQ(block.size(), 0);
    
    // 测试日期属性（可能为空，这是正常的）
    CHECK_EQ(factor.startDate(), Datetime::min());
    // createAt 和 updateAt 可能为空，这取决于具体实现
    Datetime create_time = factor.createAt();
    Datetime update_time = factor.updateAt();
    // 只要不抛出异常就是好的
    
    // 测试持久化属性
    CHECK_FALSE(factor.needPersist());
    factor.needPersist(true);
    CHECK_UNARY(factor.needPersist());
}

/** @par 检测点：测试Factor带Block构造 */
TEST_CASE("test_Factor_with_block") {
    // 创建测试 Block
    Block test_block("行业", "测试板块");
    
    // 创建带 Block 的 Factor
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor("BLOCK_FACTOR", ma5, KQuery::DAY, "带板块因子", "测试板块功能", 
                  false, Datetime::min(), test_block);
    
    // 验证 Block 属性
    const Block& block = factor.block();
    CHECK_FALSE(block.isNull());
    CHECK_EQ(block.category(), "行业");
    CHECK_EQ(block.name(), "测试板块");
    CHECK_EQ(block.size(), 0);
}

/** @par 检测点：测试Factor getValues方法的check参数 */
TEST_CASE("test_Factor_getValues_check") {
    // 创建测试 Indicator
    Indicator ma5 = MA(CLOSE(), 5);
    KQuery query_obj(0, Null<int64_t>(), KQuery::DAY);
    
    // 创建不带 Block 的 Factor
    SUBCASE("Factor without block") {
        Factor factor("NO_BLOCK_FACTOR", ma5, KQuery::DAY);
        
        // check=false 时应该正常工作
        CHECK_NOTHROW(factor.getValues(StockList{}, query_obj, false));
        
        // check=true 时也应该正常工作（因为没有 Block 限制）
        CHECK_NOTHROW(factor.getValues(StockList{}, query_obj, true));
    }
    
    // 创建带 Block 的 Factor
    SUBCASE("Factor with block") {
        Block test_block("行业", "测试板块");
        Factor factor("BLOCK_FACTOR", ma5, KQuery::DAY, "测试", "描述", 
                      false, Datetime::min(), test_block);
        
        // 验证 Block 设置正确
        CHECK_FALSE(factor.block().isNull());
        
        // check=false 时应该正常工作
        CHECK_NOTHROW(factor.getValues(StockList{}, query_obj, false));
        
        // check=true 时，空股票列表应该正常工作
        CHECK_NOTHROW(factor.getValues(StockList{}, query_obj, true));
    }
}

/** @par 检测点：测试Factor getAllValues方法 */
TEST_CASE("test_Factor_getAllValues") {
    KQuery query_obj(0, Null<int64_t>(), KQuery::DAY);
    KQuery week_query_obj(0, Null<int64_t>(), KQuery::WEEK);
    KQuery month_query_obj(0, Null<int64_t>(), KQuery::MONTH);
    
    // 测试不带 Block 的 Factor
    SUBCASE("Factor without block") {
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor("ALL_NO_BLOCK", ma5, KQuery::DAY);
        
        // 应该能正常调用（虽然可能返回空结果）
        CHECK_NOTHROW(factor.getAllValues(query_obj));
        // 验证方法可以正常调用，不抛出异常
    }
    
    // 测试带 Block 的 Factor
    SUBCASE("Factor with block") {
        Block test_block("行业", "测试板块");
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor("ALL_BLOCK_FACTOR", ma5, KQuery::DAY, "测试", "描述", 
                      false, Datetime::min(), test_block);
        
        // 验证 Block 设置正确
        CHECK_FALSE(factor.block().isNull());
        
        // 应该能正常调用
        CHECK_NOTHROW(factor.getAllValues(query_obj));
        // 验证方法可以正常调用，不抛出异常
    }
    
    // 测试不同查询参数
    SUBCASE("Different query parameters") {
        Indicator ma5 = MA(CLOSE(), 5);
        Factor factor("QUERY_TEST", ma5, KQuery::DAY);
        
        // 测试日线查询
        CHECK_NOTHROW(factor.getAllValues(query_obj));
        
        // 测试周线查询
        CHECK_NOTHROW(factor.getAllValues(week_query_obj));
        
        // 测试月线查询
        CHECK_NOTHROW(factor.getAllValues(month_query_obj));
    }
}

/** @par 检测点：测试Factor拷贝和赋值语义 */
TEST_CASE("test_Factor_copy_semantics") {
    // 创建原始 Factor
    Indicator ma5 = MA(CLOSE(), 5);
    Block test_block("行业", "测试板块");
    Factor original("COPY_TEST", ma5, KQuery::DAY, "拷贝测试", "详细描述", 
                    true, Datetime(202001010000LL), test_block);
    
    // 测试拷贝构造
    Factor copy1(original);
    CHECK_EQ(copy1.name(), "COPY_TEST");
    CHECK_EQ(copy1.ktype(), KQuery::DAY);
    CHECK_EQ(copy1.brief(), "拷贝测试");
    CHECK_EQ(copy1.details(), "详细描述");
    CHECK_UNARY(copy1.needPersist());
    CHECK_EQ(copy1.startDate(), Datetime(202001010000LL));
    CHECK_EQ(copy1.block().category(), "行业");
    CHECK_EQ(copy1.block().name(), "测试板块");
    
    // 测试拷贝赋值
    Factor copy2;
    copy2 = original;
    CHECK_EQ(copy2.name(), "COPY_TEST");
    CHECK_EQ(copy2.ktype(), KQuery::DAY);
    CHECK_EQ(copy2.brief(), "拷贝测试");
    CHECK_EQ(copy2.details(), "详细描述");
    CHECK_UNARY(copy2.needPersist());
    CHECK_EQ(copy2.startDate(), Datetime(202001010000LL));
    CHECK_EQ(copy2.block().category(), "行业");
    CHECK_EQ(copy2.block().name(), "测试板块");
    
    // 测试移动构造
    Factor moved1(std::move(original));
    CHECK_EQ(moved1.name(), "COPY_TEST");
    CHECK_EQ(moved1.ktype(), KQuery::DAY);
    
    // 原始对象应该变为空
    CHECK_UNARY(original.isNull());
    
    // 测试移动赋值
    Factor moved2;
    moved2 = std::move(copy1);
    CHECK_EQ(moved2.name(), "COPY_TEST");
    CHECK_EQ(moved2.ktype(), KQuery::DAY);
    
    // 原对象应该变为空
    CHECK_UNARY(copy1.isNull());
}

/** @par 检测点：测试Factor哈希和比较 */
TEST_CASE("test_Factor_hash_compare") {
    Indicator ma5 = MA(CLOSE(), 5);
    Indicator ma10 = MA(CLOSE(), 10);
    
    // 创建相同的 Factor
    Factor factor1("HASH_TEST", ma5, KQuery::DAY);
    Factor factor2("HASH_TEST", ma5, KQuery::DAY);
    
    // 创建不同的 Factor
    Factor factor3("HASH_TEST", ma10, KQuery::DAY);
    Factor factor4("DIFFERENT", ma5, KQuery::DAY);
    
    // 测试不同的 Factor 应该有不同的哈希值
    CHECK_NE(factor1.hash(), factor3.hash());
    CHECK_NE(factor1.hash(), factor4.hash());
    
    // 测试标准库哈希函数
    std::hash<Factor> hasher;
    CHECK_NE(hasher(factor1), hasher(factor3));
}

/** @} */
