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

/** @par Test points */
TEST_CASE("test_Block_getStockList_null_safe") {
    /** @arg The default-constructed (Null) Block must not dereference a null m_data */
    Block blk;
    CHECK(blk.isNull());

    StockList all = blk.getStockList();
    CHECK(all.empty());

    StockList filtered = blk.getStockList([](const Stock&) { return true; });
    CHECK(filtered.empty());
}

/** @par Test points */
TEST_CASE("test_Block_add_list_all_of") {
    StockManager& sm = StockManager::instance();

    /** @arg A middle failure is not overwritten by later successes (StockList) */
    Block blk;
    CHECK_UNARY_FALSE(blk.add(StockList{sm["sh000001"], Stock(), sm["sz000002"]}));
    CHECK_EQ(blk.size(), 2u);

    /** @arg All succeed (StockList) */
    Block blk2;
    CHECK_UNARY(blk2.add(StockList{sm["sh000001"], sm["sz000002"]}));
    CHECK_EQ(blk2.size(), 2u);

    /** @arg Empty list returns true */
    CHECK_UNARY(blk2.add(StockList{}));

    /** @arg A middle failure is not overwritten by later successes (StringList) */
    Block blk3;
    CHECK_UNARY_FALSE(blk3.add(StringList{"sh000001", "invalid_code", "sz000002"}));
    CHECK_EQ(blk3.size(), 2u);

    /** @arg All succeed (StringList) */
    Block blk4;
    CHECK_UNARY(blk4.add(StringList{"sh000001", "sz000002"}));

    /** @arg Adding an already contained stock is an idempotent success */
    CHECK_UNARY(blk.add(sm["sh000001"]));
    CHECK_EQ(blk.size(), 2u);
    CHECK_UNARY(blk.add("sh000001"));
    CHECK_EQ(blk.size(), 2u);

    /** @arg An invalid code still fails */
    CHECK_UNARY_FALSE(blk.add("invalid_code"));

    /** @arg Duplicates within one list are also idempotent successes */
    Block blk5;
    CHECK_UNARY(blk5.add(StringList{"sh000001", "sh000001", "sz000002"}));
    CHECK_EQ(blk5.size(), 2u);
}

/** @par Test points */
TEST_CASE("test_Block_eq_and_hash_order_independent") {
    StockManager& sm = StockManager::instance();

    Stock a = sm["sh000001"];
    Stock b = sm["sz000002"];
    Stock c = sm["sh000002"];

    /** @arg Two blocks with the same stocks added in different order are equal
     *      (equality must not depend on unordered_map iteration order) */
    Block blk1;
    blk1.add(a);
    blk1.add(b);
    Block blk2;
    blk2.add(b);  // reversed insertion order
    blk2.add(a);
    CHECK(blk1 == blk2);
    CHECK_UNARY_FALSE(blk1 != blk2);

    /** @arg strongHash is order-independent for the same stock set */
    CHECK_EQ(blk1.strongHash(), blk2.strongHash());

    /** @arg A block with a different stock set is not equal and hashes differently */
    Block blk3;
    blk3.add(a);
    blk3.add(c);
    CHECK_UNARY_FALSE(blk1 == blk3);
    CHECK(blk1.strongHash() != blk3.strongHash());

    /** @arg A block with an extra stock is not equal and hashes differently */
    Block blk4;
    blk4.add(a);
    blk4.add(b);
    blk4.add(c);
    CHECK_UNARY_FALSE(blk1 == blk4);
    CHECK(blk1.strongHash() != blk4.strongHash());
}

/** @} */
