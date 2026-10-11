/*
 * test_KRecord.cpp
 *
 *  Created on: 2026-10-11
 */

#include "doctest/doctest.h"
#include <hikyuu/KRecord.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_KRecord test_hikyuu_KRecord
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_KRecord_operator_eq_amount_tolerance") {
    KRecord base(Datetime(201101010000), 10.0, 13.3, 9.7, 12.01, 1000000.0, 800000.0);

    /** @arg identical records are equal */
    KRecord same = base;
    CHECK(base == same);
    CHECK_FALSE(base != same);

    /** @arg large-magnitude amount/count differing by a tiny absolute amount are equal */
    KRecord approx = base;
    approx.transAmount = 1000000.0 + 0.5;
    approx.transCount = 800000.0 + 0.5;
    CHECK(base == approx);

    /** @arg clearly different large-magnitude amount/count are not equal */
    KRecord differ = base;
    differ.transAmount = 2000000.0;
    CHECK(base != differ);
}

/** @} */
