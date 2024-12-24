/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator_talib/ta_crt.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/CVAL.h>

using namespace hku;

/**
 * @defgroup test_indicator_TA_STOCHF test_indicator_TA_STOCHF
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_STOCHF") {
    KData kdata = getKData("sz000001", KQuery(-10));

    /** @arg 非法参数 */
    CHECK_THROWS(TA_STOCHF(kdata, 0, 3, 0));
    CHECK_THROWS(TA_STOCHF(kdata, 100001, 3, 0));
    CHECK_THROWS(TA_STOCHF(kdata, 5, 0, 0));
    CHECK_THROWS(TA_STOCHF(kdata, 5, 100001, 0));
    CHECK_THROWS(TA_STOCHF(kdata, 5, 3, -1));
    CHECK_THROWS(TA_STOCHF(kdata, 5, 3, 9));

    /** @arg KData 为空 */
    Indicator result = TA_STOCHF(KData());
    CHECK_EQ(result.name(), "TA_STOCHF");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.getResultNumber(), 2);

    /** @arg KData 长度小于默认参数抛弃数量 */
    result = TA_STOCHF(getKData("sh000001", KQuery(-1)));
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.getResultNumber(), 2);

    /** @arg 正常情况 */
    result = TA_STOCHF(kdata);
    CHECK_EQ(result.name(), "TA_STOCHF");
    CHECK_EQ(result.discard(), 6);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 2);
    CHECK_UNARY(std::isnan(result[5]));
    CHECK_EQ(result[6], doctest::Approx(5.660377).epsilon(0.00001));
    CHECK_EQ(result.get(6, 1), doctest::Approx(35.826186).epsilon(0.00001));
    CHECK_EQ(result[9], doctest::Approx(59.493670).epsilon(0.00001));
    CHECK_EQ(result.get(9, 1), doctest::Approx(56.540084).epsilon(0.00001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_STOCHF_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_STOCHF.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_STOCHF(kdata);
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

    CHECK_EQ(x1.name(), x2.name());
    CHECK_UNARY(x1.size() == x2.size());
    CHECK_UNARY(x1.discard() == x2.discard());
    CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    for (size_t r = 0, total = x1.getResultNumber(); r < total; ++r) {
        for (size_t i = x1.discard(); i < x1.size(); ++i) {
            CHECK_EQ(x1.get(i, r), doctest::Approx(x2.get(i, r)).epsilon(0.00001));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
