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

/** @par 检测点 */
TEST_CASE("test_CODELIKE") {
    StockManager& sm = StockManager::instance();

    /** @arg 开头匹配测试 - 无通配符 */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // sh000001的代码是000001，以"000"开头，应该匹配成功
        Indicator result = CODELIKE(kdata, "000");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }

    /** @arg 开头匹配测试 - 不匹配 */
    {
        Stock stock = sm.getStock("sh600000");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // sh600000的代码是600000，不以"000"开头，应该匹配失败
        Indicator result = CODELIKE(kdata, "000");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 0.0);
        }
    }

    /** @arg 通配符?完全匹配测试 */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // 000001匹配??????（6个?）
        Indicator result = CODELIKE(kdata, "??????");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }

    /** @arg 深圳股票测试 */
    {
        Stock stock = sm.getStock("sz000001");
        if (!stock.isNull()) {
            KData kdata = stock.getKData(KQuery(-10));
            CHECK_EQ(10, kdata.size());
            // sz000001的代码是000001，以"000"开头
            Indicator result = CODELIKE(kdata, "000");
            CHECK_EQ(result.size(), kdata.size());
            for (size_t i = 0; i < result.size(); ++i) {
                CHECK_EQ(result[i], 1.0);
            }
        }
    }
}

/** @par 检测点 */
TEST_CASE("test_NAMELIKE") {
    StockManager& sm = StockManager::instance();

    /** @arg 开头匹配测试 */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // sh000001的名称是"上证指数"，以"上证"开头，应该匹配成功
        Indicator result = NAMELIKE(kdata, "上证");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }

    /** @arg 不匹配测试 */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // sh000001的名称是"上证指数"，不以"深证"开头
        Indicator result = NAMELIKE(kdata, "深证");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 0.0);
        }
    }

    /** @arg 通配符测试 */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());
        // 使用通配符进行完全匹配
        Indicator result = NAMELIKE(kdata, "*指数");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }
}

/** @par 检测点 */
TEST_CASE("test_CODELIKE_NAMELIKE_wildcard") {
    StockManager& sm = StockManager::instance();

    /** @arg 通配符*匹配任意序列 */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());

        // *可以匹配任意序列（完全匹配）
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

    /** @arg 通配符组合测试 */
    {
        Stock stock = sm.getStock("sh000001");
        KData kdata = stock.getKData(KQuery(-10));
        CHECK_EQ(10, kdata.size());

        // 测试*和?的组合（完全匹配）
        // 000001匹配"0*1"（以0开头，以1结尾）
        Indicator result = CODELIKE(kdata, "0*1");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }

        // 000001匹配"0????1"（以0开头，中间4个任意字符，以1结尾）
        result = CODELIKE(kdata, "0????1");
        CHECK_EQ(result.size(), kdata.size());
        for (size_t i = 0; i < result.size(); ++i) {
            CHECK_EQ(result[i], 1.0);
        }
    }
}

/** @} */
