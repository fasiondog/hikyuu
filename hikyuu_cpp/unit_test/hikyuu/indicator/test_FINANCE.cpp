/*
 * test_FINANCE.cpp
 *
 *  Created on: 2026-10-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/FINANCE.h>

using namespace hku;

/**
 * @defgroup test_indicator_FINANCE test_indicator_FINANCE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points: an unrecognized field name fails fast with a contextual message */
TEST_CASE("test_FINANCE_unrecognized_field_name") {
    Stock stk = getStock("SZ000001");
    REQUIRE(!stk.isNull());
    KData k = stk.getKData(KQuery(-20));
    REQUIRE(k.size() > 0);

    // The factory calls setContext, which drives the calculation synchronously; the field name is
    // resolved before the records are read, so a bad name throws during the factory call itself
    /** @arg The KData factory throws on an unrecognized field name */
    CHECK_THROWS_AS(FINANCE(k, "__nonexistent_finance_field__"), std::exception);

    /** @arg The no-context factory + setContext also throws (setContext triggers calculation) */
    Indicator r = FINANCE("__nonexistent_finance_field__");
    CHECK_THROWS_AS(r.setContext(k), std::exception);
}

/** @par Test points: a valid field index on a stock without finance data returns an empty result */
TEST_CASE("test_FINANCE_no_finance_data") {
    Stock stk = getStock("SZ000001");
    REQUIRE(!stk.isNull());
    KData k = stk.getKData(KQuery(-20));
    REQUIRE(k.size() > 0);

    /** @arg A valid index (>= 0) does not throw; no finance data -> discard == size */
    Indicator r = FINANCE(k, 0);
    CHECK_EQ(r.name(), "FINANCE");
    CHECK_EQ(r.size(), k.size());
    CHECK_EQ(r.discard(), r.size());
}

/** @par Test points: a recognized field name resolves without error */
TEST_CASE("test_FINANCE_valid_field_name") {
    Stock stk = getStock("SZ000001");
    REQUIRE(!stk.isNull());
    KData k = stk.getKData(KQuery(-20));
    REQUIRE(k.size() > 0);

    auto fields = StockManager::instance().getHistoryFinanceAllFields();
    REQUIRE(fields.size() > 0);
    string name = fields[0].second;

    /** @arg A real field name resolves to a valid index; no data -> discard == size, no throw */
    Indicator r = FINANCE(k, name);
    CHECK_EQ(r.size(), k.size());
    CHECK_EQ(r.discard(), r.size());
}

/** @par Test points: the incremental path matches full calculation */
TEST_CASE("test_FINANCE_incremental_matches_full") {
    Stock stk = getStock("SZ000001");
    REQUIRE(!stk.isNull());
    REQUIRE(Indicator::enableIncrementCalculate());

    KData k_short = stk.getKData(KQueryByDate(Datetime(200701010000), Datetime(201101010000)));
    KData k_long = stk.getKData(KQueryByDate(Datetime(200701010000), Datetime(201112050000)));
    REQUIRE(k_short.size() > 0);
    REQUIRE(k_long.size() > k_short.size());

    Indicator r = FINANCE(0);
    r.setContext(k_short);
    r.setContext(k_long);

    Indicator full = FINANCE(k_long, 0);
    /** @arg size, discard and every value match the single-shot full calculation */
    CHECK_EQ(r.size(), full.size());
    CHECK_EQ(r.discard(), full.discard());
    for (size_t i = 0; i < r.size(); i++) {
        if (std::isnan(full[i])) {
            CHECK_UNARY(std::isnan(r[i]));
        } else {
            CHECK_UNARY(!std::isnan(r[i]));
            CHECK_EQ(r[i], full[i]);
        }
    }
}

/** @} */
