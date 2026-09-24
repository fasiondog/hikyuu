/*
 * test_Block.cpp
 *
 *  Created on: 2015-2-8
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

/** @par Test points */
TEST_CASE("test_Block_1") {
    Block blk;
    /** @arg The default constructor with category and name being "" */
    CHECK(blk.category() == "");
    CHECK(blk.name() == "");

    /** @arg Constructed with category and name */
    Block blk2("test", "1");
    CHECK(blk2.category() == "test");
    CHECK(blk2.name() == "1");

    /** @arg The inequality judgment */
    CHECK((blk != blk2));

    /** @arg The comparison with an empty Block */
    CHECK_UNARY(blk == Null<Block>());
    CHECK_UNARY(blk == Block());

    /** @arg Compare the equality after the assignment */
    Block blk3 = blk2;
    CHECK((blk2 == blk3));
}

/** @par Test points */
TEST_CASE("test_Block_2") {
    StockManager& sm = StockManager::instance();
    Block blk;

    /** @arg The block is empty: empty=true / size=0 */
    CHECK(blk.empty());
    CHECK(blk.size() == 0);

    /** @arg Add a Stock instance directly */
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

    /** @arg Add by the Stock identifier */
    blk.add("sh000002");
    CHECK(!blk.empty());
    CHECK(blk.size() == 3);
    CHECK(blk.have("sh000002"));
    CHECK(blk.get("sh000002") == sm["sh000002"]);

    /** @arg Remove a Stock instance directly */
    blk.remove(sm["sh000001"]);
    CHECK(!blk.empty());
    CHECK(blk.size() == 2);
    CHECK(blk.have("sh000002"));
    CHECK(blk.have("sz000002"));
    CHECK(!blk.have("sh000001"));
    // CHECK((blk.get("sh000001") == Null<Stock>()));

    /** @arg Remove by the Stock identifier */
    blk.remove("sz000002");
    CHECK(!blk.empty());
    CHECK(blk.size() == 1);
    CHECK(blk.have("sh000002"));
    CHECK(!blk.have("sz000002"));
    CHECK(!blk.have("sh000001"));
    // CHECK((blk.get("sz000002") == Null<Stock>()));

    /** @arg Remove when only one record is left */
    blk.remove("sh000002");
    CHECK(blk.empty());
    CHECK(blk.size() == 0);
    CHECK(!blk.have("sh000002"));
    CHECK(!blk.have("sz000002"));
    CHECK(!blk.have("sh000001"));
    // CHECK((blk.get("sh000002") == Null<Stock>()));

    /** @arg Remove when it is empty */
    blk.remove("sh000002");
    CHECK(blk.empty());
    CHECK(blk.size() == 0);

    /** @arg Remove a Stock that does not exist */
    blk.add("sh000002");
    CHECK(!blk.have("sz000001"));
    blk.remove("sz000001");
    CHECK(blk.have("sh000002"));
    CHECK(!blk.empty());
    CHECK(blk.size() == 1);

    /** @arg Test clear */
    CHECK(!blk.empty());
    blk.clear();
    CHECK(blk.empty());
}

/** @} */
