/*
 * test_Block.cpp
 *
 *  Created on: 2015年2月8日
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <iostream>
#include "hikyuu/utilities/Log.h"
#include <hikyuu/Block.h>
#include <hikyuu/StockManager.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_Block test_hikyuu_Block
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_Block_1") {
    Block blk;
    /** @arg 默认构造函数,category、name为"" */
    CHECK(blk.category() == "");
    CHECK(blk.name() == "");

    /** @arg 使用category、name构造 */
    Block blk2("test", "1");
    CHECK(blk2.category() == "test");
    CHECK(blk2.name() == "1");

    /** @arg 不等判断 */
    CHECK((blk != blk2));

    /** @arg 空Block比较 */
    CHECK_UNARY(blk == Null<Block>());
    CHECK_UNARY(blk == Block());

    /** @arg 赋值后比较是否相等 */
    Block blk3 = blk2;
    CHECK((blk2 == blk3));

    /** @arg category、name相同但指向不同，判定不相等 */
    Block blk4("test", "1");
    CHECK((blk2 != blk4));
}

/** @par 检测点 */
TEST_CASE("test_Block_2") {
    StockManager& sm = StockManager::instance();
    Block blk;

    /** @arg block为空，empty=true / size=0 */
    CHECK(blk.empty());
    CHECK(blk.size() == 0);

    /** @arg 直接加入Stock实例 */
    blk.add(sm["sh000001"]);
    CHECK(!blk.empty());
    CHECK(blk.size() == 1);
    blk.add(sm["sz000002"]);
    CHECK(!blk.empty());
    CHECK(blk.size() == 2);

    CHECK(blk.have("sh000001"));
    CHECK(blk.have("sz000002"));
    CHECK(!blk.have("sz000001"));
    CHECK(blk.get("sh000001") == sm["sh000001"]);
    CHECK(blk.get("sz000002") == sm["sz000002"]);

    /*for (auto iter = blk.begin(); iter != blk.end(); ++iter) {
        std::cout << *iter << std::endl;
    }*/

    /** @arg 已Stock标识加入 */
    blk.add("sh000002");
    CHECK(!blk.empty());
    CHECK(blk.size() == 3);
    CHECK(blk.have("sh000002"));
    CHECK(blk.get("sh000002") == sm["sh000002"]);

    /** @arg 直接删除Stock实例 */
    blk.remove(sm["sh000001"]);
    CHECK(!blk.empty());
    CHECK(blk.size() == 2);
    CHECK(blk.have("sh000002"));
    CHECK(blk.have("sz000002"));
    CHECK(!blk.have("sh000001"));
    // CHECK((blk.get("sh000001") == Null<Stock>()));

    /** @arg 使用Stock标识进行删除 */
    blk.remove("sz000002");
    CHECK(!blk.empty());
    CHECK(blk.size() == 1);
    CHECK(blk.have("sh000002"));
    CHECK(!blk.have("sz000002"));
    CHECK(!blk.have("sh000001"));
    // CHECK((blk.get("sz000002") == Null<Stock>()));

    /** @arg 仅剩一条记录时执行删除操作 */
    blk.remove("sh000002");
    CHECK(blk.empty());
    CHECK(blk.size() == 0);
    CHECK(!blk.have("sh000002"));
    CHECK(!blk.have("sz000002"));
    CHECK(!blk.have("sh000001"));
    // CHECK((blk.get("sh000002") == Null<Stock>()));

    /** @arg 为空时，执行删除操作 */
    blk.remove("sh000002");
    CHECK(blk.empty());
    CHECK(blk.size() == 0);

    /** @arg 删除不存在的Stock */
    blk.add("sh000002");
    CHECK(!blk.have("sz000001"));
    blk.remove("sz000001");
    CHECK(blk.have("sh000002"));
    CHECK(!blk.empty());
    CHECK(blk.size() == 1);

    /** @arg 测试clear */
    CHECK(!blk.empty());
    blk.clear();
    CHECK(blk.empty());
}

/** @} */
