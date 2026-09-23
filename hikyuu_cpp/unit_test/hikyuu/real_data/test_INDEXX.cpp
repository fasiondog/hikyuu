/*
 * test_INDEXX.cpp
 * Test the correctness of INDEXC in the TDX unusual price formula with real data
 *
 * The corresponding TDX formula:
 *   IC:INDEXC;
 *   INDEX_RET:(IC-REF(IC,30))/REF(IC,30);
 *   unusual price 30 days: REF(CLOSE,30)*(3+INDEX_RET);
 *   INDEX_RET10:=(IC-REF(IC,10))/REF(IC,10);
 *   unusual price 10 days: REF(CLOSE,10)*(2+INDEX_RET10);
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/build_in.h>
#include <cmath>
#include <iostream>

using namespace hku;

/**
 * @defgroup test_INDEXC_yidong_real test_INDEXC_yidong_real
 * @ingroup test_hikyuu_real_data
 * @{
 */

/** @par Test point: the correctness of INDEXC in the unusual price formula (suspended stocks) */
TEST_CASE("test_INDEXC") {
    StockManager& sm = StockManager::instance();

    auto testOne = [&](const char* code, const char* name) {
        auto stock = sm.getStock(code);
        REQUIRE(!stock.isNull());
        auto kdata = stock.getKData(KQuery(-1000, Null<int64_t>()));
        size_t n = kdata.size();
        REQUIRE_GT(n, 0);
        std::cout << code << " " << name << " K-line count=" << n << std::endl;

        auto ic = INDEXC();
        auto c = CLOSE();

        auto ref_c30 = REF(30)(c);
        auto ref_ic30 = REF(30)(ic);
        auto index_ret = (ic - ref_ic30) / ref_ic30;
        auto yidong30 = ref_c30 * (CVAL(3.0) + index_ret);

        auto ref_c10 = REF(10)(c);
        auto ref_ic10 = REF(10)(ic);
        auto index_ret10 = (ic - ref_ic10) / ref_ic10;
        auto yidong10 = ref_c10 * (CVAL(2.0) + index_ret10);

        try {
            yidong30.setContext(kdata);
            yidong10.setContext(kdata);
        } catch (const std::exception& e) {
            std::cout << "INDEXC exception: " << e.what() << std::endl;
            CHECK_GT(0, 0);
        } catch (...) {
            std::cout << "INDEXC unknown exception" << std::endl;
            CHECK_GT(0, 0);
        }

        double v30 = yidong30[n - 1];
        double v10 = yidong10[n - 1];
        std::cout << "  unusual price 30 days=" << v30 << " 10 days=" << v10 << std::endl;
        CHECK_UNARY(!std::isnan(v30));
        CHECK_UNARY(!std::isnan(v10));
        CHECK_GT(v30, 0);
        CHECK_GT(v10, 0);

        ic.setContext(kdata);
        // Compare bar by bar: INDEXC[i] should equal the close of sh000001 on kdata[i].datetime
        auto idx_stock = sm.getStock("sh000001");
        REQUIRE(!idx_stock.isNull());
        KQuery query = kdata.getQuery();
        auto secs = KQuery::getKTypeInSeconds(query.kType());
        KQuery idxQuery = KQueryByDate(kdata[0].datetime, kdata[n - 1].datetime + Seconds(secs),
                                       query.kType(), query.recoverType());
        auto idx_kdata = idx_stock.getKData(idxQuery);

        // Build the index date -> close price mapping
        std::unordered_map<hku::Datetime, double> idx_close_map;
        for (size_t j = 0; j < idx_kdata.size(); ++j) {
            idx_close_map[idx_kdata[j].datetime] = idx_kdata[j].closePrice;
        }

        size_t diff_count = 0;
        size_t null_count = 0;
        for (size_t i = 0; i < n; ++i) {
            double ic_val = ic[i];
            auto dt = kdata[i].datetime;
            auto it = idx_close_map.find(dt);
            if (it == idx_close_map.end()) {
                null_count++;
                continue;
            }
            double expected = it->second;
            if (std::abs(ic_val - expected) > 0.0001) {
                diff_count++;
                if (diff_count <= 5) {
                    std::cout << "  difference bar[" << i << "] date=" << dt << " INDEXC=" << ic_val
                              << " expected=" << expected << " diff=" << std::abs(ic_val - expected)
                              << std::endl;
                }
            }
        }
        std::cout << "  INDEXC bar by bar: the difference count=" << diff_count << "/" << n
                  << " the missing index day count=" << null_count << std::endl;
        CHECK_EQ(diff_count, 0);
    };

    // 603629: no suspension, the index K-line count == the stock K-line count
    testOne("sh603629", "利通电子");
    // 603045: suspended (2021-09-27~2021-10-15, 2022-11-30), the two K-line counts differ
    testOne("sh603045", "福达合金");
}

/** @} */
