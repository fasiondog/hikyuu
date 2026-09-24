/*
 * test_CODELIKE_NAMELIKE.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CODELIKE.h>
#include <hikyuu/indicator/crt/NAMELIKE.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_CODELIKE_NAMELIKE test_CODELIKE_NAMELIKE
 * @ingroup test_hikyuu_indicator
 * @{
 */

/** @par Test points */
TEST_CASE("test_CODELIKE") {
    StockManager& sm = StockManager::instance();

    /** @arg The prefix match test - without a wildcard */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // The code of sh000001 is 000001, starting with "000", so it should match
        Indicator result = CODELIKE(kdata, "000");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }

    /** @arg The prefix match test - no match */
    {
        Stock stock = sm.getStock("sh600000");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // The code of sh600000 is 600000, not starting with "000", so it should not match
        Indicator result = CODELIKE(kdata, "000");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 0.0);
        }
    }

    /** @arg The wildcard ? exact match test */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // 000001 matches ?????? (6 question marks)
        Indicator result = CODELIKE(kdata, "??????");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }

    /** @arg The Shenzhen stock test */
    {
        Stock stock = sm.getStock("sz000001");
        if (!stock.isNull()) {
            KData kdata = stock.getKData(KQuery(-10));
            CHECK_EQ(10, kdata.size());
            // The code of sz000001 is 000001, starting with "000"
            Indicator result = CODELIKE(kdata, "000");
            CHECK_EQ(result.size(), kdata.size());
            for (size_t i = 0; i < result.size(); ++i) {
                CHECK_EQ(result[i], 1.0);
            }
        }
    }
}

/** @par Test points */
TEST_CASE("test_NAMELIKE") {
    StockManager& sm = StockManager::instance();

    /** @arg The prefix match test */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // The name of sh000001 is "上证指数", starting with "上证", so it should match
        Indicator result = NAMELIKE(kdata, "上证");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }

    /** @arg The no-match test */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // The name of sh000001 is "上证指数", not starting with "深证"
        Indicator result = NAMELIKE(kdata, "深证");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 0.0);
        }
    }

    /** @arg The wildcard test */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // An exact match with a wildcard
        Indicator result = NAMELIKE(kdata, "*指数");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }
}

/** @par Test points */
TEST_CASE("test_CODELIKE_NAMELIKE_wildcard") {
    StockManager& sm = StockManager::instance();

    /** @arg The wildcard * matches any sequence */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());

        // * matches any sequence (an exact match)
        Indicator result = CODELIKE(kdata, "*");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }

        result = NAMELIKE(kdata, "*");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }

    /** @arg The wildcard combination test */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());

        // Test the combination of * and ? (an exact match)
        // 000001 matches "0*1" (starts with 0 and ends with 1)
        Indicator result = CODELIKE(kdata, "0*1");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }

        // 000001 matches "0????1" (starts with 0, 4 arbitrary characters, ends with 1)
        result = CODELIKE(kdata, "0????1");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }
}

/** @} */
