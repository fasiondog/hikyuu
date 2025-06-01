/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-06-01
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/BARSLASTCOUNT.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_BARSLASTCOUNT test_indicator_BARSLASTCOUNT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_BARSLASTCOUNT") {
    auto k = getKData("sz000001", KQuery(-10));
    Indicator ret = BARSLASTCOUNT(CLOSE() > (OPEN()))(k);
    CHECK_EQ(ret.name(), "BARSLASTCOUNT");

    auto expect = PRICELIST(PriceList{1, 0, 1, 2, 0, 1, 0, 0, 1, 2});
    check_indicator(ret, expect);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_BARSLASTCOUNT_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BARSLASTCOUNT.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = BARSLASTCOUNT(CLOSE() > OPEN())(kdata);
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

    CHECK_EQ(x2.name(), "BARSLASTCOUNT");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    check_indicator(x2, x1);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
