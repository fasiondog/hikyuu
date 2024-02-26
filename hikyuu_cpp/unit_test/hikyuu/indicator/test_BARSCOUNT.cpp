/*
 * test_BARSCOUNT.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/BARSLAST.h>
#include <hikyuu/indicator/crt/BARSCOUNT.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_BARSCOUNT test_indicator_BARSCOUNT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_BARSCOUNT") {
    Indicator result;

    /** @arg 输入指标，未关联上下文 */
    PriceList a;
    a.push_back(1);
    a.push_back(0);
    a.push_back(0);
    a.push_back(1);

    Indicator data = PRICELIST(a, 2);

    result = BARSCOUNT(data);
    CHECK_EQ(result.name(), "BARSCOUNT");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_EQ(result[2], 1);
    CHECK_EQ(result[3], 2);

    /** @arg 输入指标为日线 CLOSE */
    Stock stk = getStock("sz000001");
    KData k = stk.getKData(KQuery(-100));
    result = BARSCOUNT(CLOSE(k));
    CHECK_EQ(result.name(), "BARSCOUNT");
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 4838);
    CHECK_EQ(result[99], 4937);

    /** @arg 输入指标为分钟线 CLOSE */
    k = stk.getKData(KQueryByDate(Datetime(200209020000), Datetime(200209050000), KQuery::MIN));
    result = BARSCOUNT(CLOSE(k));
    CHECK_EQ(result.name(), "BARSCOUNT");
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 1);
    CHECK_EQ(result[1], 2);
    CHECK_EQ(result[477], 239);
    CHECK_EQ(result[478], 240);
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_BARSCOUNT_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));  //, Null<int64_t>(), KQuery::MIN));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_BARSCOUNT_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = BARSCOUNT();
            Indicator result = ind(c);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_BARSCOUNT_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BARSCOUNT.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = BARSCOUNT(CLOSE(kdata));
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    Indicator x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    CHECK_EQ(x2.name(), "BARSCOUNT");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
