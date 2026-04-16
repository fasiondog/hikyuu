/*
 * test_STKTYPE.cpp
 *
 *  Created on: 2026-04-17
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/STKTYPE.h>

using namespace hku;

/**
 * @defgroup test_indicator_STKTYPE test_indicator_STKTYPE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_STKTYPE") {
    KData k;
    Indicator stktype;

    /** @arg 查询A股股票的类型 */
    Stock stk = getStock("SZ000001");
    REQUIRE(!stk.isNull());
    k = stk.getKData(KQuery(-10));
    REQUIRE(k.size() > 0);

    stktype = STKTYPE(k);
    CHECK_EQ(stktype.name(), "STKTYPE");
    CHECK_EQ(stktype.size(), k.size());
    CHECK_EQ(stktype.discard(), 0);

    // A股的类型为 STOCKTYPE_A (1)
    for (size_t i = 0; i < k.size(); ++i) {
        CHECK_EQ(stktype[i], doctest::Approx(STOCKTYPE_A));
    }

    /** @arg 查询指数的类型 */
    stk = getStock("sh000001");
    REQUIRE(!stk.isNull());
    k = stk.getKData(KQuery(-10));
    REQUIRE(k.size() > 0);

    stktype = STKTYPE(k);
    CHECK_EQ(stktype.size(), k.size());
    CHECK_EQ(stktype.discard(), 0);

    // 指数的类型为 STOCKTYPE_INDEX (2)
    for (size_t i = 0; i < k.size(); ++i) {
        CHECK_EQ(stktype[i], doctest::Approx(STOCKTYPE_INDEX));
    }

    /** @arg 使用默认上下文（无参数版本） */
    stk = getStock("SZ000001");
    REQUIRE(!stk.isNull());
    k = stk.getKData(KQuery(-10));
    REQUIRE(k.size() > 0);

    stktype = STKTYPE();
    stktype.setContext(k);
    CHECK_EQ(stktype.size(), k.size());
    CHECK_EQ(stktype.discard(), 0);

    // A股的类型为 STOCKTYPE_A (1)
    for (size_t i = 0; i < k.size(); ++i) {
        CHECK_EQ(stktype[i], doctest::Approx(STOCKTYPE_A));
    }

    /** @arg 空K线数据 */
    KData empty_k;
    stktype = STKTYPE(empty_k);
    CHECK_EQ(stktype.size(), 0);
    CHECK_EQ(stktype.discard(), 0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_STKTYPE_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/STKTYPE.xml";

    KData k = getStock("SH600000").getKData(KQuery(-10));
    Indicator x1 = STKTYPE(k);
    x1.setContext(k);

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

    CHECK_EQ(x2.name(), "STKTYPE");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
