/*
 * test_ABS.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/indicator/crt/IC.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_sys/factor/crt/MF_EqualWeight.h>

using namespace hku;

/**
 * @defgroup test_MF_EqualWeight test_MF_EqualWeight
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MF_EqualWeight") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-100);
    KData ref_k = ref_stk.getKData(query);
    DatetimeList ref_dates = ref_k.getDatetimeList();
    IndicatorList src_inds{MA(CLOSE()), AMA(CLOSE(), EMA(CLOSE()))};

    /** @arg 输入的股票列表中含有空股票 */
    CHECK_THROWS_AS(MF_EqualWeight(src_inds, {Null<Stock>()}, query, ref_stk), std::exception);

    /** @arg 输入的原始因子列表为空 */
    CHECK_THROWS_AS(MF_EqualWeight({}, stks, query, ref_stk), std::exception);

    /** @arg 输入的参考证券为空 */
    CHECK_THROWS_AS(MF_EqualWeight({}, stks, query, Null<Stock>()), std::exception);

    /** @arg 数据长度不足 */
    CHECK_THROWS_AS(MF_EqualWeight(src_inds, stks, KQuery(-1), ref_stk), std::exception);

    /** @arg 证券列表数量不足 */
    CHECK_THROWS_AS(MF_EqualWeight(src_inds, {sm["sh600004"]}, KQuery(-2), ref_stk),
                    std::exception);

    /** @arg 临界状态, 原始因子数量为1, 数据长度为2 */
    auto mf = MF_EqualWeight({MA(CLOSE())}, {sm["sh600004"], sm["sh600005"]}, KQuery(-2), ref_stk);
    CHECK_EQ(mf->name(), "MF_EqualWeight");
    CHECK_THROWS_AS(mf->get(sm["sz000001"]), std::exception);
    auto dates = ref_stk.getKData(KQuery(-2)).getDatetimeList();
    CHECK_EQ(mf->getDatetimeList(), dates);
    auto ind1 = mf->get(sm["sh600004"]);
    auto ind2 = MA(CLOSE(sm["sh600004"].getKData(KQuery(-2))));
    CHECK_UNARY(ind1.equal(ind2));
    ind1 = mf->get(sm["sh600005"]);
    ind2 = MA(CLOSE(sm["sh600005"].getKData(KQuery(-2))));
    CHECK_UNARY(ind1.equal(ind2));
    auto ic1 = mf->getIC(1);
    auto ic2 = IC(MA(CLOSE()), {sm["sh600004"], sm["sh600005"]}, KQuery(-2), 1, ref_stk);
    CHECK_UNARY(ic1.equal(ic2));

    // HKU_INFO("{}", mf->getDatetimeList());

    // CHECK_EQ(mf->name(), "MF_EqualWeight");
    // CHECK(mf->get(sm["sh600004"]).empty());
    // MFPtr mf = MF_EqualWeight(src_inds, stks,
    // query, ref_stk); mf->calculate();
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_MF_EqualWeight_benchmark") {
    // Stock stock = getStock("sh000001");
    // KData kdata = stock.getKData(KQuery(0));
    // Indicator c = kdata.close();
    // int cycle = 1000;  // 测试循环次数

    // {
    //     BENCHMARK_TIME_MSG(test_ABS_benchmark, cycle, fmt::format("data len: {}", c.size()));
    //     SPEND_TIME_CONTROL(false);
    //     for (int i = 0; i < cycle; i++) {
    //         Indicator ind = ABS();
    //         Indicator result = ind(c);
    //     }
    // }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MF_EqualWeight_export") {
    // StockManager& sm = StockManager::instance();
    // string filename(sm.tmpdir());
    // filename += "/ABS.xml";

    // Stock stock = sm.getStock("sh000001");
    // KData kdata = stock.getKData(KQuery(-20));
    // Indicator x1 = ABS(CLOSE(kdata));
    // {
    //     std::ofstream ofs(filename);
    //     boost::archive::xml_oarchive oa(ofs);
    //     oa << BOOST_SERIALIZATION_NVP(x1);
    // }

    // Indicator x2;
    // {
    //     std::ifstream ifs(filename);
    //     boost::archive::xml_iarchive ia(ifs);
    //     ia >> BOOST_SERIALIZATION_NVP(x2);
    // }

    // CHECK_UNARY(x1.size() == x2.size());
    // CHECK_UNARY(x1.discard() == x2.discard());
    // CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    // for (size_t i = 0; i < x1.size(); ++i) {
    //     CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    // }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
