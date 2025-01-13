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
#include <hikyuu/indicator/crt/ROCR.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_Weight.h>

using namespace hku;

/**
 * @defgroup test_MF_Weight test_MF_Weight
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MF_Weight") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-100);
    KData ref_k = ref_stk.getKData(query);
    DatetimeList ref_dates = ref_k.getDatetimeList();
    IndicatorList src_inds{MA(CLOSE()), AMA(CLOSE(), EMA(CLOSE()))};
    PriceList weights{0.2, 0.3, 0.5};

    /** @arg 输入的股票列表中含有空股票 */
    CHECK_THROWS_AS(MF_Weight(src_inds, weights, {Null<Stock>()}, query, ref_stk), std::exception);

    /** @arg 输入的原始因子列表为空 */
    CHECK_THROWS_AS(MF_Weight({}, weights, stks, query, ref_stk), std::exception);

    /** @arg 输入的参考证券为空 */
    CHECK_THROWS_AS(MF_Weight({}, weights, stks, query, Null<Stock>()), std::exception);

    /** @arg 数据长度不足 */
    CHECK_THROWS_AS(MF_Weight(src_inds, weights, stks, KQuery(-1), ref_stk), std::exception);

    /** @arg 证券列表数量不足 */
    CHECK_THROWS_AS(MF_Weight(src_inds, weights, {sm["sh600004"]}, KQuery(-2), ref_stk),
                    std::exception);

    /** @arg 输入非法 ic_n */
    CHECK_THROWS_AS(MF_Weight(src_inds, weights, stks, KQuery(-2), ref_stk, 0), std::exception);

    /** @arg 因子列表和权重列表长度不一致 */
    CHECK_THROWS_AS(MF_Weight(src_inds, PriceList{0.1}, stks, KQuery(-2), ref_stk, 0),
                    std::exception);

    /** @arg 临界状态, 原始因子数量为1, 证券数量2, 数据长度为2 */
    src_inds = {MA(CLOSE())};
    stks = {sm["sh600005"], sm["sh600004"]};
    query = KQuery(-2);
    ref_k = ref_stk.getKData(query);
    ref_dates = ref_k.getDatetimeList();
    auto mf = MF_Weight(src_inds, PriceList{1.0}, stks, query, ref_stk);
    CHECK_EQ(mf->name(), "MF_Weight");
    CHECK_THROWS_AS(mf->getFactor(sm["sz000001"]), std::exception);
    CHECK_EQ(mf->getDatetimeList(), ref_dates);

    const auto& all_factors = mf->getAllFactors();
    CHECK_EQ(all_factors.size(), 2);
    auto ind1 = mf->getFactor(sm["sh600004"]);
    auto ind2 = MA(CLOSE(sm["sh600004"].getKData(query)));
    CHECK_UNARY(ind1.equal(ind2));
    CHECK_UNARY(all_factors[1].equal(ind2));
    ind1 = mf->getFactor(sm["sh600005"]);
    ind2 = MA(CLOSE(sm["sh600005"].getKData(query)));
    CHECK_UNARY(ind1.equal(ind2));
    CHECK_UNARY(all_factors[0].equal(ind2));
    auto ic1 = mf->getIC();
    auto ic2 = IC(MA(CLOSE()), stks, query, ref_stk, 1);
    CHECK_UNARY(ic1.equal(ic2));

    CHECK_UNARY(mf->getScores(Datetime(20111204)).empty());
    auto cross = mf->getScores(Datetime(20111205));
    CHECK_EQ(cross.size(), 2);
    CHECK_EQ(cross[0].stock, sm["sh600004"]);
    CHECK_EQ(cross[0].value, doctest::Approx(6.85));
    CHECK_EQ(cross[1].stock, sm["sh600005"]);
    CHECK_EQ(cross[1].value, doctest::Approx(3.13));

    cross = mf->getScores(Datetime(20111206));
    CHECK_EQ(cross.size(), 2);
    CHECK_EQ(cross[0].stock, sm["sh600004"]);
    CHECK_EQ(cross[0].value, doctest::Approx(6.855));
    CHECK_EQ(cross[1].stock, sm["sh600005"]);
    CHECK_EQ(cross[1].value, doctest::Approx(3.14));
    // HKU_INFO("\n{}", mf->getAllCross());

    /** @arg 原始因子数量为3, 证券数量4, 数据长度为20, 指定比较收益率 3 日 */
    int ndays = 3;
    src_inds = {MA(ROCR(CLOSE(), ndays)), AMA(ROCR(CLOSE(), ndays)), EMA(ROCR(CLOSE(), ndays))};
    stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    query = KQuery(-20);
    ref_k = ref_stk.getKData(query);
    ref_dates = ref_k.getDatetimeList();
    mf = MF_Weight(src_inds, weights, stks, query, ref_stk, ndays);
    CHECK_EQ(mf->name(), "MF_Weight");
    CHECK_THROWS_AS(mf->getFactor(sm["sh600000"]), std::exception);

    auto stk = sm["sh600004"];
    ind1 = MA(ROCR(CLOSE(stk.getKData(query)), ndays));
    ind2 = AMA(ROCR(CLOSE(stk.getKData(query)), ndays));
    auto ind3 = EMA(ROCR(CLOSE(stk.getKData(query)), ndays));
    auto ind4 = mf->getFactor(stk);
    CHECK_EQ(ind4.discard(), 3);
    for (size_t i = 0; i < ind4.discard(); i++) {
        CHECK_UNARY(std::isnan(ind4[i]));
    }
    for (size_t i = ind4.discard(), len = ref_dates.size(); i < len; i++) {
        CHECK_EQ(ind4[i],
                 doctest::Approx(ind1[i] * weights[0] + ind2[i] * weights[1] + ind3[i] * weights[2])
                   .epsilon(0.0001));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_MF_Weight_benchmark") {
    StockManager& sm = StockManager::instance();
    int ndays = 3;
    IndicatorList src_inds = {MA(ROCR(CLOSE(), ndays)), AMA(ROCR(CLOSE(), ndays)),
                              EMA(ROCR(CLOSE(), ndays))};
    StockList stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    KQuery query = KQuery(0);
    Stock ref_stk = sm["sh000001"];
    auto ref_k = ref_stk.getKData(query);
    auto ref_dates = ref_k.getDatetimeList();

    int cycle = 10;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_MF_Weight_benchmark, cycle,
                           fmt::format("data len: {}", ref_k.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            auto mf = MF_Weight(src_inds, PriceList{0.2, 0.3, 0.5}, stks, query, ref_stk);
            auto ic = mf->getIC();
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION
/** @par 检测点 */
TEST_CASE("test_MF_Weight_export") {
    StockManager& sm = StockManager::instance();
    int ndays = 3;
    IndicatorList src_inds = {MA(ROCR(CLOSE(), ndays)), AMA(ROCR(CLOSE(), ndays)),
                              EMA(ROCR(CLOSE(), ndays))};
    StockList stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    KQuery query = KQuery(0);
    Stock ref_stk = sm["sh000001"];
    auto ref_k = ref_stk.getKData(query);
    auto ref_dates = ref_k.getDatetimeList();

    string filename(sm.tmpdir());
    filename += "/MF_Weight.xml";

    auto mf1 = MF_Weight(src_inds, PriceList{0.1, 0.3, 0.6}, stks, query, ref_stk);
    auto ic1 = mf1->getIC();
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(mf1);
    }

    MFPtr mf2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(mf2);
    }

    CHECK_EQ(mf2->name(), mf1->name());
    auto ic2 = mf2->getIC();
    CHECK_EQ(ic1.size(), ic2.size());
    CHECK_EQ(ic1.discard(), ic2.discard());
    CHECK_UNARY(ic1.equal(ic2));
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */