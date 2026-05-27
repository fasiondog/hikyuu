/*
 * test_INDEXX.cpp
 * 使用真实数据测试INDEXC在通达信异动价格公式中的正确性
 *
 * 对应通达信公式:
 *   IC:INDEXC;
 *   INDEX_RET:(IC-REF(IC,30))/REF(IC,30);
 *   异动价格30日:REF(CLOSE,30)*(3+INDEX_RET);
 *   INDEX_RET10:=(IC-REF(IC,10))/REF(IC,10);
 *   异动价格10日:REF(CLOSE,10)*(2+INDEX_RET10);
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

/** @par 检测点: INDEXC在异动价格公式中的正确性（含停牌个股） */
TEST_CASE("test_INDEXC") {
    StockManager& sm = StockManager::instance();

    auto testOne = [&](const char* code, const char* name) {
        auto stock = sm.getStock(code);
        REQUIRE(!stock.isNull());
        auto kdata = stock.getKData(KQuery(-1000, Null<int64_t>()));
        size_t n = kdata.size();
        REQUIRE_GT(n, 0);
        std::cout << code << " " << name << " K线数=" << n << std::endl;

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
            std::cout << "INDEXC异常: " << e.what() << std::endl;
            CHECK_GT(0, 0);
        } catch (...) {
            std::cout << "INDEXC未知异常" << std::endl;
            CHECK_GT(0, 0);
        }


        double v30 = yidong30[n - 1];
        double v10 = yidong10[n - 1];
        std::cout << "  异动价格30日=" << v30 << " 异动价格10日=" << v10 << std::endl;
        CHECK_UNARY(!std::isnan(v30));
        CHECK_UNARY(!std::isnan(v10));
        CHECK_GT(v30, 0);
        CHECK_GT(v10, 0);


        ic.setContext(kdata);
        // 逐条对比: INDEXC[i] 应等于 sh000001 在 kdata[i].datetime 日期的收盘价
        auto idx_stock = sm.getStock("sh000001");
        REQUIRE(!idx_stock.isNull());
        KQuery query = kdata.getQuery();
        auto secs = KQuery::getKTypeInSeconds(query.kType());
        KQuery idxQuery = KQueryByDate(
            kdata[0].datetime, kdata[n - 1].datetime + Seconds(secs),
            query.kType(), query.recoverType());
        auto idx_kdata = idx_stock.getKData(idxQuery);

        // 构建指数日期->收盘价映射
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
                    std::cout << "  差异 bar[" << i << "] date=" << dt
                              << " INDEXC=" << ic_val << " 期望=" << expected
                              << " diff=" << std::abs(ic_val - expected) << std::endl;
                }
            }
        }
        std::cout << "  INDEXC逐条对比: 差异数=" << diff_count << "/" << n
                  << " 指数缺失日数=" << null_count << std::endl;
        CHECK_EQ(diff_count, 0);
    };

    // 603629: 无停牌，指数K线数==个股K线数
    testOne("sh603629", "利通电子");
    // 603045: 有停牌（2021-09-27~2021-10-15, 2022-11-30），指数K线数!=个股K线数
    testOne("sh603045", "福达合金");
}

/** @} */
