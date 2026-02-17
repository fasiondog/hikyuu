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
    
    // 测试正常的有效名称
    CHECK_NOTHROW(FactorMeta("valid_name", dummy_formula));
    CHECK_NOTHROW(FactorMeta("test123", dummy_formula));
    CHECK_NOTHROW(FactorMeta("_underscore", dummy_formula));
    CHECK_NOTHROW(FactorMeta("MyFactor", dummy_formula));
    CHECK_NOTHROW(FactorMeta("factor_name_2026", dummy_formula));
    
    // 测试无效的名称格式（会抛出异常）
    CHECK_THROWS_AS(FactorMeta("", dummy_formula), hku::exception);           // 空名称
    CHECK_THROWS_AS(FactorMeta("123invalid", dummy_formula), hku::exception); // 数字开头
    CHECK_THROWS_AS(FactorMeta("invalid-name", dummy_formula), hku::exception); // 包含连字符
    CHECK_THROWS_AS(FactorMeta("invalid name", dummy_formula), hku::exception); // 包含空格
    CHECK_THROWS_AS(FactorMeta("invalid.name", dummy_formula), hku::exception); // 包含点号
    
    // 测试数据库关键字（应该抛出异常）
    // SQLite关键字测试
    CHECK_THROWS_AS(FactorMeta("SELECT", dummy_formula), hku::exception);     // SQLite关键字
    CHECK_THROWS_AS(FactorMeta("select", dummy_formula), hku::exception);     // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("Table", dummy_formula), hku::exception);      // SQLite关键字
    CHECK_THROWS_AS(FactorMeta("table", dummy_formula), hku::exception);      // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("CREATE", dummy_formula), hku::exception);     // 所有三个数据库的关键字
    CHECK_THROWS_AS(FactorMeta("create", dummy_formula), hku::exception);     // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("INDEX", dummy_formula), hku::exception);      // SQLite/MySQL关键字
    CHECK_THROWS_AS(FactorMeta("index", dummy_formula), hku::exception);      // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("DATABASE", dummy_formula), hku::exception);   // SQLite关键字
    CHECK_THROWS_AS(FactorMeta("database", dummy_formula), hku::exception);   // 小写也应该被拒绝
    
    // MySQL关键字测试
    CHECK_THROWS_AS(FactorMeta("ADD", dummy_formula), hku::exception);        // MySQL关键字
    CHECK_THROWS_AS(FactorMeta("add", dummy_formula), hku::exception);        // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("ALTER", dummy_formula), hku::exception);      // MySQL关键字
    CHECK_THROWS_AS(FactorMeta("alter", dummy_formula), hku::exception);      // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("PRIMARY", dummy_formula), hku::exception);    // MySQL关键字
    CHECK_THROWS_AS(FactorMeta("primary", dummy_formula), hku::exception);    // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("UNIQUE", dummy_formula), hku::exception);     // MySQL关键字
    CHECK_THROWS_AS(FactorMeta("unique", dummy_formula), hku::exception);     // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("UPDATE", dummy_formula), hku::exception);     // MySQL关键字
    CHECK_THROWS_AS(FactorMeta("update", dummy_formula), hku::exception);     // 小写也应该被拒绝
    
    // ClickHouse关键字测试
    CHECK_THROWS_AS(FactorMeta("FROM", dummy_formula), hku::exception);       // ClickHouse关键字
    CHECK_THROWS_AS(FactorMeta("from", dummy_formula), hku::exception);       // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("WHERE", dummy_formula), hku::exception);      // ClickHouse关键字
    CHECK_THROWS_AS(FactorMeta("where", dummy_formula), hku::exception);      // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("GROUP", dummy_formula), hku::exception);      // ClickHouse关键字
    CHECK_THROWS_AS(FactorMeta("group", dummy_formula), hku::exception);      // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("ORDER", dummy_formula), hku::exception);      // ClickHouse关键字
    CHECK_THROWS_AS(FactorMeta("order", dummy_formula), hku::exception);      // 小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("LIMIT", dummy_formula), hku::exception);      // ClickHouse关键字
    CHECK_THROWS_AS(FactorMeta("limit", dummy_formula), hku::exception);      // 小写也应该被拒绝
    
    // 测试混合大小写的数据库关键字
    CHECK_THROWS_AS(FactorMeta("SeLeCt", dummy_formula), hku::exception);     // 混合大小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("TaBlE", dummy_formula), hku::exception);      // 混合大小写也应该被拒绝
    CHECK_THROWS_AS(FactorMeta("CrEaTe", dummy_formula), hku::exception);     // 混合大小写也应该被拒绝
    
    // 测试非关键字的正常名称（确保不影响正常名称）
    CHECK_NOTHROW(FactorMeta("my_select", dummy_formula));        // 包含关键字但不是纯关键字
    CHECK_NOTHROW(FactorMeta("select_data", dummy_formula));      // 包含关键字但不是纯关键字
    CHECK_NOTHROW(FactorMeta("table_view", dummy_formula));       // 包含关键字但不是纯关键字
    CHECK_NOTHROW(FactorMeta("create_factor", dummy_formula));    // 包含关键字但不是纯关键字
    CHECK_NOTHROW(FactorMeta("index_value", dummy_formula));      // 包含关键字但不是纯关键字
}

/** @} */