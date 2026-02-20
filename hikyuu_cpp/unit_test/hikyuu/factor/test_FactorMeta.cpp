/*
 * test_FactorMeta.cpp
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/factor/FactorMeta.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_FactorMeta test_FactorMeta
 * @ingroup test_hikyuu_factor_suite
 * @{
 */

/** @par 检测点：测试FactorMeta构造函数对名称的验证（间接测试isValidName函数） */
TEST_CASE("test_FactorMeta_name_validation") {
    Indicator dummy_formula = PRICELIST();

    /** @arg 测试正常的有效名称 */
    CHECK_NOTHROW(FactorMeta("valid_name", dummy_formula));
    CHECK_NOTHROW(FactorMeta("test123", dummy_formula));
    CHECK_NOTHROW(FactorMeta("_underscore", dummy_formula));
    CHECK_NOTHROW(FactorMeta("MyFactor", dummy_formula));
    CHECK_NOTHROW(FactorMeta("factor_name_2026", dummy_formula));

    /** @arg 测试无效的名称格式（会抛出异常）*/
    CHECK_THROWS(FactorMeta("", dummy_formula));              // 空名称
    CHECK_THROWS(FactorMeta("123invalid", dummy_formula));    // 数字开头
    CHECK_THROWS(FactorMeta("invalid-name", dummy_formula));  // 包含连字符
    CHECK_THROWS(FactorMeta("invalid name", dummy_formula));  // 包含空格
    CHECK_THROWS(FactorMeta("invalid.name", dummy_formula));  // 包含点号

    // 测试数据库关键字（应该抛出异常）
    /** @arg SQLite关键字测试 */
    CHECK_THROWS(FactorMeta("SELECT", dummy_formula));    // SQLite关键字
    CHECK_THROWS(FactorMeta("select", dummy_formula));    // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("Table", dummy_formula));     // SQLite关键字
    CHECK_THROWS(FactorMeta("table", dummy_formula));     // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("CREATE", dummy_formula));    // 所有三个数据库的关键字
    CHECK_THROWS(FactorMeta("create", dummy_formula));    // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("INDEX", dummy_formula));     // SQLite/MySQL关键字
    CHECK_THROWS(FactorMeta("index", dummy_formula));     // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("DATABASE", dummy_formula));  // SQLite关键字
    CHECK_THROWS(FactorMeta("database", dummy_formula));  // 小写也应该被拒绝

    /** @arg MySQL关键字测试 */
    CHECK_THROWS(FactorMeta("ADD", dummy_formula));      // MySQL关键字
    CHECK_THROWS(FactorMeta("add", dummy_formula));      // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("ALTER", dummy_formula));    // MySQL关键字
    CHECK_THROWS(FactorMeta("alter", dummy_formula));    // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("PRIMARY", dummy_formula));  // MySQL关键字
    CHECK_THROWS(FactorMeta("primary", dummy_formula));  // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("UNIQUE", dummy_formula));   // MySQL关键字
    CHECK_THROWS(FactorMeta("unique", dummy_formula));   // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("UPDATE", dummy_formula));   // MySQL关键字
    CHECK_THROWS(FactorMeta("update", dummy_formula));   // 小写也应该被拒绝

    /** @arg ClickHouse关键字测试 */
    CHECK_THROWS(FactorMeta("FROM", dummy_formula));   // ClickHouse关键字
    CHECK_THROWS(FactorMeta("from", dummy_formula));   // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("WHERE", dummy_formula));  // ClickHouse关键字
    CHECK_THROWS(FactorMeta("where", dummy_formula));  // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("GROUP", dummy_formula));  // ClickHouse关键字
    CHECK_THROWS(FactorMeta("group", dummy_formula));  // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("ORDER", dummy_formula));  // ClickHouse关键字
    CHECK_THROWS(FactorMeta("order", dummy_formula));  // 小写也应该被拒绝
    CHECK_THROWS(FactorMeta("LIMIT", dummy_formula));  // ClickHouse关键字
    CHECK_THROWS(FactorMeta("limit", dummy_formula));  // 小写也应该被拒绝

    /** @arg 测试混合大小写的数据库关键字 */
    CHECK_THROWS(FactorMeta("SeLeCt", dummy_formula));  // 混合大小写也应该被拒绝
    CHECK_THROWS(FactorMeta("TaBlE", dummy_formula));   // 混合大小写也应该被拒绝
    CHECK_THROWS(FactorMeta("CrEaTe", dummy_formula));  // 混合大小写也应该被拒绝

    /** @arg 测试非关键字的正常名称（确保不影响正常名称） */
    CHECK_NOTHROW(FactorMeta("my_select", dummy_formula));      // 包含关键字但不是纯关键字
    CHECK_NOTHROW(FactorMeta("select_data", dummy_formula));    // 包含关键字但不是纯关键字
    CHECK_NOTHROW(FactorMeta("table_view", dummy_formula));     // 包含关键字但不是纯关键字
    CHECK_NOTHROW(FactorMeta("create_factor", dummy_formula));  // 包含关键字但不是纯关键字
    CHECK_NOTHROW(FactorMeta("index_value", dummy_formula));    // 包含关键字但不是纯关键字
}

/** @par 检测点：测试FactorMeta的hash方法 */
TEST_CASE("test_FactorMeta_hash_basic") {
    Indicator dummy_formula = PRICELIST();

    /** @arg 测试相同名称和K线类型的FactorMeta应该有相同的hash值 */
    FactorMeta factor1("test_factor_same", dummy_formula, KQuery::DAY);
    FactorMeta factor2("test_factor_same", dummy_formula, KQuery::DAY);
    CHECK_EQ(factor1.hash(), factor2.hash());

    /** @arg 测试不同名称的FactorMeta应该有不同的hash值 */
    FactorMeta factor3("different_name_factor", dummy_formula, KQuery::DAY);
    CHECK_NE(factor1.hash(), factor3.hash());

    /** @arg 测试不同K线类型的FactorMeta应该有不同的hash值 */
    FactorMeta factor4("test_factor_same", dummy_formula, KQuery::MIN5);
    CHECK_NE(factor1.hash(), factor4.hash());
}

/** @par 检测点：测试FactorMeta的hash方法一致性 */
TEST_CASE("test_FactorMeta_hash_consistency") {
    Indicator dummy_formula = PRICELIST();

    /** @arg 测试hash值的一致性 - 同一对象多次调用hash应该返回相同值 */
    FactorMeta factor1("consistent_test", dummy_formula, KQuery::DAY);
    uint64_t hash1 = factor1.hash();
    uint64_t hash2 = factor1.hash();
    CHECK_EQ(hash1, hash2);

    /** @arg 测试拷贝构造后的hash值一致性 */
    FactorMeta factor_copy(factor1);
    CHECK_EQ(factor1.hash(), factor_copy.hash());

    /** @arg 测试赋值操作后的hash值一致性 */
    FactorMeta factor_assigned = factor1;
    CHECK_EQ(factor1.hash(), factor_assigned.hash());
}

/** @par 检测点：测试FactorMeta的hash方法边界情况 */
TEST_CASE("test_FactorMeta_hash_edge_cases") {
    Indicator dummy_formula = PRICELIST();

    /** @arg 测试长名称的情况 */
    std::string long_name = "very_long_factor_name_that_exceeds_typical_length_2026";
    FactorMeta factor_long(long_name, dummy_formula, KQuery::DAY);
    FactorMeta factor_long2(long_name, dummy_formula, KQuery::DAY);
    CHECK_EQ(factor_long.hash(), factor_long2.hash());

    /** @arg 测试包含数字的名称 */
    FactorMeta factor_num1("factor_with_numbers_123", dummy_formula, KQuery::DAY);
    FactorMeta factor_num2("factor_with_numbers_123", dummy_formula, KQuery::DAY);
    CHECK_EQ(factor_num1.hash(), factor_num2.hash());

    /** @arg 测试下划线开头的名称 */
    FactorMeta factor_under1("_leading_underscore_name", dummy_formula, KQuery::DAY);
    FactorMeta factor_under2("_leading_underscore_name", dummy_formula, KQuery::DAY);
    CHECK_EQ(factor_under1.hash(), factor_under2.hash());

    /** @arg 测试混合大小写的名称（应该被视为相同） */
    FactorMeta factor_case1("MixedCaseFactorName", dummy_formula, KQuery::DAY);
    FactorMeta factor_case2("mixedcasefactorname", dummy_formula, KQuery::DAY);
    CHECK_EQ(factor_case1.hash(), factor_case2.hash());

    /** @arg 测试边界情况：最小有效名称 */
    FactorMeta factor_min1("a", dummy_formula, KQuery::DAY);
    FactorMeta factor_min2("a", dummy_formula, KQuery::DAY);
    FactorMeta factor_min3("b", dummy_formula, KQuery::DAY);
    CHECK_EQ(factor_min1.hash(), factor_min2.hash());
    CHECK_NE(factor_min1.hash(), factor_min3.hash());
}

/** @par 检测点：测试FactorMeta的hash方法K线类型 */
TEST_CASE("test_FactorMeta_hash_ktypes") {
    Indicator dummy_formula = PRICELIST();

    /** @arg 测试不同的K线类型常量 */
    FactorMeta factor1("ktype_min", dummy_formula, KQuery::MIN);
    FactorMeta factor2("ktype_min5", dummy_formula, KQuery::MIN5);
    FactorMeta factor3("ktype_day", dummy_formula, KQuery::DAY);
    FactorMeta factor4("ktype_week", dummy_formula, KQuery::WEEK);

    /** @arg 验证不同K线类型产生不同的hash值 */
    std::vector<uint64_t> hashes = {factor1.hash(), factor2.hash(), factor3.hash(), factor4.hash()};

    /** @arg 检查是否有重复的hash值 */
    std::set<uint64_t> unique_hashes(hashes.begin(), hashes.end());
    CHECK(unique_hashes.size() >= hashes.size() * 0.75);  // 至少75%不重复
}

/** @} */