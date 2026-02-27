/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MRR.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_MRR test_indicator_MRR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MRR") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator open, close, mrr;

    /** @arg n = 10 且数据大小刚好为10 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    auto c = kdata.close();
    auto m = MRR(c);
    CHECK_EQ(m.name(), "MRR");
    CHECK_EQ(m.empty(), false);
    CHECK_EQ(m.size(), kdata.size());
    CHECK_EQ(m.discard(), 0);
    std::vector<price_t> expects{0.,      0.103922, 0.103922, 0.118056, 1.35151,
                                 1.35151, 2.29046,  2.29046,  2.29046,  2.29046};
    check_indicator(m, PRICELIST(expects));

    /** @arg n = 0 时，正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    close = CLOSE(kdata);
    mrr = MRR(close, 0);
    CHECK_EQ(mrr.name(), "MRR");
    CHECK_EQ(mrr.empty(), false);
    CHECK_EQ(mrr.size(), kdata.size());
    CHECK_EQ(mrr.discard(), 0);
    check_indicator(mrr, PRICELIST(expects));

    /** @arg n = 1 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-5));
    close = CLOSE(kdata);
    mrr = MRR(close, 1);
    CHECK_EQ(mrr.name(), "MRR");
    CHECK_EQ(mrr.empty(), false);
    CHECK_EQ(mrr.size(), kdata.size());
    CHECK_EQ(mrr.discard(), 0);
    Indicator zero_ind = PRICELIST(PriceList(kdata.size(), 0.0));
    check_indicator(mrr, zero_ind);

    /** @arg 空数据测试 */
    kdata = KData();
    close = CLOSE(kdata);
    mrr = MRR(close, 0);
    CHECK_EQ(mrr.name(), "MRR");
    CHECK_EQ(mrr.size(), 0);
    CHECK_EQ(mrr.empty(), true);

    /** @arg 极端上涨序列 */
    PriceList rising_data{100.0, 110.0, 120.0, 130.0, 140.0};
    Indicator rising = PRICELIST(rising_data);
    Indicator mrr_rising = MRR(rising, 0);
    expects = {0.0, 10.0, 20.0, 30.0, 40.0};
    check_indicator(mrr_rising, PRICELIST(expects));

    mrr_rising = MRR(rising, 3);
    expects = {0.0, 10.0, 20.0, 18.181818, 16.66667};
    check_indicator(mrr_rising, PRICELIST(expects));

    /** @arg 极端下跌序列 */
    PriceList falling_data{100.0, 90.0, 80.0, 70.0, 60.0};
    Indicator falling = PRICELIST(falling_data);
    Indicator mrr_falling = MRR(falling, 0);
    Indicator zero_ind2 = PRICELIST(PriceList(falling_data.size(), 0.0));
    check_indicator(mrr_falling, zero_ind2);

    /** @arg 包含相等价格的情况 */
    PriceList equal_data{100.0, 100.0, 100.0, 100.0, 100.0};
    Indicator equal = PRICELIST(equal_data);
    Indicator mrr_equal = MRR(equal, 0);
    Indicator zero_equal = PRICELIST(PriceList(equal_data.size(), 0.0));
    check_indicator(mrr_equal, zero_equal);

    /** @arg 单一数据点 */
    PriceList single_data{100.0};
    Indicator single = PRICELIST(single_data);
    Indicator mrr_single = MRR(single, 0);
    CHECK_EQ(mrr_single.size(), 1);
    CHECK_EQ(mrr_single[0], 0.0);

    /** @arg 空数据 */
    PriceList empty_data{};
    Indicator empty = PRICELIST(empty_data);
    Indicator mrr_empty = MRR(empty, 0);
    CHECK_EQ(mrr_empty.size(), 0);
    CHECK_EQ(mrr_empty.empty(), true);

    /** @arg 增量计算 */
    kdata = stock.getKData(KQuery(-20, -10));
    m = MRR(CLOSE(), 3)(kdata);
    mrr = m(stock.getKData(-15));
    m = MRR(CLOSE(), 3)(stock.getKData(-15));
    for (size_t i = 10; i < mrr.size(); i++) {
        CHECK_EQ(mrr[i], doctest::Approx(m[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MRR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MRR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator m1 = MRR(kdata.close());
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(m1);
    }

    Indicator m2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(m2);
    }

    CHECK_EQ(m2.name(), "MRR");
    CHECK_EQ(m1.size(), m2.size());
    CHECK_EQ(m1.discard(), m2.discard());
    CHECK_EQ(m1.getResultNumber(), m2.getResultNumber());
    // 使用check_indicator验证序列化前后的一致性
    check_indicator(m1, m2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */