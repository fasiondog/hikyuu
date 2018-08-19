/*
 * test_Block.cpp
 *
 *  Created on: 2015年2月8日
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_utilities
    #include <boost/test/unit_test.hpp>
#endif

#include <iostream>
#include <hikyuu/Log.h>
#include <hikyuu/Block.h>
#include <hikyuu/StockManager.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_Block test_hikyuu_Block
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Block_1 ) {
    Block blk;
    /** @arg 默认构造函数,category、name为"" */
    BOOST_CHECK(blk.category() == "");
    BOOST_CHECK(blk.name() == "");

    /** @arg 使用category、name构造 */
    Block blk2("test", "1");
    BOOST_CHECK(blk2.category() == "test");
    BOOST_CHECK(blk2.name() == "1");

    /** @arg 不等判断 */
    BOOST_CHECK(blk != blk2);

    /** @arg 空Block比较 */
    BOOST_CHECK(blk == Null<Block>());
    BOOST_CHECK(blk == Block());

    /** @arg 赋值后比较是否相等 */
    Block blk3 = blk2;
    BOOST_CHECK(blk2 == blk3);

    /** @arg category、name相同但指向不同，判定不相等 */
    Block blk4("test", "1");
    BOOST_CHECK(blk2 != blk4);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Block_2 ) {
    StockManager& sm = StockManager::instance();
    Block blk;

    /** @arg block为空，empty=true / size=0 */
    BOOST_CHECK(blk.empty());
    BOOST_CHECK(blk.size() == 0);

    /** @arg 直接加入Stock实例 */
    blk.add(sm["sh000001"]);
    BOOST_CHECK(!blk.empty());
    BOOST_CHECK(blk.size() == 1);
    blk.add(sm["sz000002"]);
    BOOST_CHECK(!blk.empty());
    BOOST_CHECK(blk.size() == 2);

    BOOST_CHECK(blk.have("sh000001"));
    BOOST_CHECK(blk.have("sz000002"));
    BOOST_CHECK(!blk.have("sz000001"));
    BOOST_CHECK(blk.get("sh000001") == sm["sh000001"]);
    BOOST_CHECK(blk.get("sz000002") == sm["sz000002"]);

    /*for (auto iter = blk.begin(); iter != blk.end(); ++iter) {
        std::cout << *iter << std::endl;
    }*/

    /** @arg 已Stock标识加入 */
    blk.add("sh000002");
    BOOST_CHECK(!blk.empty());
    BOOST_CHECK(blk.size() == 3);
    BOOST_CHECK(blk.have("sh000002"));
    BOOST_CHECK(blk.get("sh000002") == sm["sh000002"]);

    /** @arg 直接删除Stock实例 */
    blk.remove(sm["sh000001"]);
    BOOST_CHECK(!blk.empty());
    BOOST_CHECK(blk.size() == 2);
    BOOST_CHECK(blk.have("sh000002"));
    BOOST_CHECK(blk.have("sz000002"));
    BOOST_CHECK(!blk.have("sh000001"));
    BOOST_CHECK(blk.get("sh000001") == Null<Stock>());

    /** @arg 使用Stock标识进行删除 */
    blk.remove("sz000002");
    BOOST_CHECK(!blk.empty());
    BOOST_CHECK(blk.size() == 1);
    BOOST_CHECK(blk.have("sh000002"));
    BOOST_CHECK(!blk.have("sz000002"));
    BOOST_CHECK(!blk.have("sh000001"));
    BOOST_CHECK(blk.get("sz000002") == Null<Stock>());

    /** @arg 仅剩一条记录时执行删除操作 */
    blk.remove("sh000002");
    BOOST_CHECK(blk.empty());
    BOOST_CHECK(blk.size() == 0);
    BOOST_CHECK(!blk.have("sh000002"));
    BOOST_CHECK(!blk.have("sz000002"));
    BOOST_CHECK(!blk.have("sh000001"));
    BOOST_CHECK(blk.get("sh000002") == Null<Stock>());

    /** @arg 为空时，执行删除操作 */
    blk.remove("sh000002");
    BOOST_CHECK(blk.empty());
    BOOST_CHECK(blk.size() == 0);

    /** @arg 删除不存在的Stock */
    blk.add("sh000002");
    BOOST_CHECK(!blk.have("sz000001"));
    blk.remove("sz000001");
    BOOST_CHECK(blk.have("sh000002"));
    BOOST_CHECK(!blk.empty());
    BOOST_CHECK(blk.size() == 1);

    /** @arg 测试clear */
    BOOST_CHECK(!blk.empty());
    blk.clear();
    BOOST_CHECK(blk.empty());
}

/** @} */



