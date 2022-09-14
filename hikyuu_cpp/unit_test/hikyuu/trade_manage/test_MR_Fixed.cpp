/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-09-01
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/MR_Fixed.h>

#include <hikyuu/config.h>
#if HKU_SUPPORT_SERIALIZATION
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_MR_FixedA test_MR_FixedA
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MR_Fixed") {
    Stock stk = getStock("sz000001");
    Datetime d(200001010000);

    /** @arg ratio 小于 0 */
    CHECK_THROWS(MR_Fixed(-0.1));

    /** @arg ratio 等于 0 */
    CHECK_THROWS(MR_Fixed(0.0));

    /** @arg ratio 等于 1.1 */
    CHECK_THROWS(MR_Fixed(1.1));

    /** @arg ratio 等于 1.0 */
    auto mr = MR_Fixed(1.0);
    CHECK_EQ(mr->getMarginRatio(d, stk), 1.0);

    /** @arg ratio 等于 0.6 */
    mr = MR_Fixed(0.6);
    CHECK_EQ(mr->getMarginRatio(d, stk), 0.6);
}

#if HKU_SUPPORT_SERIALIZATION
/** @par 检测点 */
TEST_CASE("test_MR_Fixed_export") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");

    string filename(sm.tmpdir());
    filename += "/MR_Fixed.xml";

    auto func1 = MR_Fixed(0.6);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(func1);
    }

    MRPtr func2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(func2);
    }

    CHECK_EQ(func2->name(), "MR_Fixed");
    CHECK_EQ(func1->getMarginRatio(Datetime(202201010000), stk),
             func2->getMarginRatio(Datetime(202201010000), stk));
}
#endif

/** @} */