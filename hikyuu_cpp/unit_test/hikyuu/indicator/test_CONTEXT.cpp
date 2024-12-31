/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-29
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CONTEXT.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/ALIGN.h>

using namespace hku;

/**
 * @defgroup test_indicator_CONTEXT test_indicator_CONTEXT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

static void check_output(const Indicator& result, const Indicator& expect) {
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = 0, len = result.discard(); i < len; i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = 0, total = result.size(); i < total; i++) {
        if (expect[i] == Null<price_t>()) {
            CHECK_EQ(result[i], Null<price_t>());
        } else {
            CHECK_EQ(result[i], expect[i]);
        }
    }
}

/** @par 检测点 */
TEST_CASE("test_CONTEXT") {
    auto stk1 = getStock("sz000001");
    auto stk2 = getStock("sz000002");
    auto stk3 = getStock("sh000001");
    KQuery q1 = KQuery(Datetime(201110250000));  // -30
    KQuery q2 = KQuery(Datetime(201111020000));  // -25
    KQuery q3 = KQuery(Datetime(201111250000));  // -8
    auto k1 = stk1.getKData(q1);
    auto k2 = stk2.getKData(q2);
    auto k3 = stk3.getKData(q3);
    price_t nan = Null<price_t>();

    /** @arg 空对象 */
    auto ctx = hku::CONTEXT();
    CHECK_UNARY(ctx.empty());
    CHECK_EQ(CONTEXT_K(ctx), Null<KData>());
    CHECK_EQ(ctx.getContext(), Null<KData>());
    CHECK_EQ(ctx.name(), "CONTEXT");
    ctx = ctx(k1);
    CHECK_UNARY(ctx.empty());
    CHECK_EQ(ctx.name(), "CONTEXT");

    /** @arg 公式原型（无数据序列） */
    ctx = hku::CONTEXT(MA(CLOSE()));
    CHECK_UNARY(ctx.empty());
    CHECK_EQ(CONTEXT_K(ctx), Null<KData>());
    CHECK_EQ(ctx.getContext(), Null<KData>());
    CHECK_EQ(ctx.name(), "CONTEXT");

    /** @arg 时间无关序列 */
    Indicator a = PRICELIST(PriceList{nan, nan, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 2);
    ctx = hku::CONTEXT(a);
    CHECK_EQ(CONTEXT_K(ctx), Null<KData>());
    check_output(ctx, a);

    auto result = ctx(k1);
    CHECK_EQ(CONTEXT_K(result), Null<KData>());
    CHECK_EQ(result.getContext(), k1);
    check_output(result, CVAL(0.)(k1) + a);

    result = result(k1);
    CHECK_EQ(CONTEXT_K(result), Null<KData>());
    CHECK_EQ(result.getContext(), k1);
    check_output(result, CVAL(0.)(k1) + a);

    result = result(k2);
    CHECK_EQ(CONTEXT_K(result), Null<KData>());
    CHECK_EQ(result.getContext(), k2);
    check_output(result, CVAL(0.)(k2) + a);

    result = result(k1);
    CHECK_EQ(CONTEXT_K(result), Null<KData>());
    CHECK_EQ(result.getContext(), k1);
    check_output(result, CVAL(0.)(k1) + a);

    result = result(k3);
    CHECK_EQ(CONTEXT_K(result), Null<KData>());
    CHECK_EQ(result.getContext(), k3);
    check_output(result, PRICELIST(PriceList{3, 4, 5, 6, 7, 8, 9, 10}));

    result = ctx(k3);
    CHECK_EQ(CONTEXT_K(result), Null<KData>());
    CHECK_EQ(result.getContext(), k3);
    check_output(result, PRICELIST(PriceList{3, 4, 5, 6, 7, 8, 9, 10}));

    /** @arg 无上下文的，时间序列 */
    auto dates = getStock("sz000001").getDatetimeList(KQuery(-12));
    a = PRICELIST(PriceList{nan, nan, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, dates, 2);
    ctx = hku::CONTEXT(a);
    CHECK_EQ(CONTEXT_K(ctx), Null<KData>());
    CHECK_EQ(ctx.getContext(), Null<KData>());
    check_output(ctx, a);

    result = ctx(k1);
    CHECK_EQ(CONTEXT_K(result), Null<KData>());
    CHECK_EQ(result.getContext(), k1);
    CHECK_EQ(result.size(), k1.size());
    CHECK_EQ(result.discard(), 20);
    check_output(result, CVAL(0.)(k1) + a);

    a = PRICELIST(PriceList{nan, nan, 1, 2, 3, 4, 5, nan, 7, 8, 9, 10}, dates, 2);
    ctx = hku::CONTEXT(a);
    CHECK_EQ(CONTEXT_K(ctx), Null<KData>());
    CHECK_EQ(ctx.getContext(), Null<KData>());
    check_output(ctx, a);

    result = ctx(k1);
    CHECK_EQ(CONTEXT_K(result), Null<KData>());
    CHECK_EQ(result.getContext(), k1);
    check_output(result, CVAL(0.)(k1) + a);

    /** @arg 带上下文的时间序列 */
    a = k2.close();
    ctx = hku::CONTEXT(a);
    CHECK_EQ(CONTEXT_K(ctx), k2);
    CHECK_EQ(ctx.getContext(), Null<KData>());
    check_output(ctx, a);

    result = ctx(a);
    CHECK_EQ(CONTEXT_K(result), k2);
    CHECK_EQ(result.getContext(), Null<KData>());
    check_output(result, a);

    result = ctx(k1);
    CHECK_EQ(CONTEXT_K(result), k2);
    CHECK_EQ(result.getContext(), k1);
    CHECK_EQ(result.size(), k1.size());
    check_output(result, ALIGN(stk2.getKData(q1).close(), k1));

    a = k1.close();
    result = hku::CONTEXT(a)(k3);
    CHECK_EQ(CONTEXT_K(result), k1);
    CHECK_EQ(result.getContext(), k3);
    CHECK_EQ(result.size(), k3.size());
    check_output(result, ALIGN(stk1.getKData(q3).close(), k3));

    /** @arg 复杂公式 */
    result = (CLOSE(k1) + hku::CONTEXT(MA(k3.close(), 2)))(k2);
    CHECK_EQ(result.size(), k2.size());
    auto expect = k2.close() + MA(stk3.getKData(q2).close(), 2);
    check_output(result, expect);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_CONTEXT_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/CONTEXT.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = hku::CONTEXT(MA(CLOSE(kdata), 3));
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

    CHECK_UNARY(x1.size() == x2.size());
    CHECK_UNARY(x1.discard() == x2.discard());
    CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
