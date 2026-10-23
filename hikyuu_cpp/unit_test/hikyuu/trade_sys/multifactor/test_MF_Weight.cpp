/*
 * test_ABS.cpp
 *
 *  Created on: 2019-4-2
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
#include <hikyuu/trade_sys/multifactor/crt/MF_EqualWeight.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_Weight.h>
#include <hikyuu/trade_sys/multifactor/buildin_scfilter.h>

using namespace hku;

/**
 * @defgroup test_MF_Weight test_MF_Weight
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MF_Weight") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-100);
    KData ref_k = ref_stk.getKData(query);
    DatetimeList ref_dates = ref_k.getDatetimeList();
    IndicatorList src_inds{MA(CLOSE()), AMA(CLOSE(), EMA(CLOSE()))};
    PriceList weights{0.2, 0.3, 0.5};

    /** @arg The input stock list contains an empty stock */
    CHECK_THROWS_AS(MF_Weight(src_inds, weights, {Null<Stock>()}, query, ref_stk), std::exception);

    /** @arg The input original factor list is empty */
    CHECK_THROWS_AS(MF_Weight(IndicatorList{}, weights, stks, query, ref_stk), std::exception);

    /** @arg The input reference security is empty */
    CHECK_THROWS_AS(MF_Weight(IndicatorList{}, weights, stks, query, Null<Stock>()),
                    std::exception);

    /** @arg The data length is insufficient */
    CHECK_THROWS_AS(MF_Weight(src_inds, weights, stks, KQuery(-1), ref_stk), std::exception);

    /** @arg The security list count is insufficient */
    CHECK_THROWS_AS(MF_Weight(src_inds, weights, {sm["sh600004"]}, KQuery(-2), ref_stk),
                    std::exception);

    /** @arg An invalid ic_n is passed */
    CHECK_THROWS_AS(MF_Weight(src_inds, weights, stks, KQuery(-2), ref_stk, 0), std::exception);

    /** @arg The factor list and the weight list have different lengths */
    CHECK_THROWS_AS(MF_Weight(src_inds, PriceList{0.1}, stks, KQuery(-2), ref_stk, 0),
                    std::exception);

    /** @arg The boundary state: 1 factor, 2 securities and the data length 2 */
    src_inds = {MA(CLOSE())};
    stks = {sm["sh600005"], sm["sh600004"]};
    query = KQuery(-2);
    ref_k = ref_stk.getKData(query);
    ref_dates = ref_k.getDatetimeList();
    auto mf = MF_Weight(src_inds, PriceList{1.0}, stks, query, ref_stk);
    mf->setParam<bool>("save_all_factors", true);
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
    auto ic1 = mf->getIC(1);
    auto ic2 = IC(MA(CLOSE()), stks, 1)(ref_k);
    CHECK_UNARY(ic1.equal(ic2));

    CHECK_UNARY(mf->getScores(Datetime(20111204)).empty());

    query = KQuery(-30);
    ref_k = ref_stk.getKData(query);
    ref_dates = ref_k.getDatetimeList();
    mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    auto cross = mf->getScores(Datetime(20111205));
    CHECK_EQ(cross.size(), 2);
    CHECK_EQ(cross[0].stock, sm["sh600004"]);
    CHECK_EQ(cross[0].value, doctest::Approx(7.143).epsilon(0.001));
    CHECK_EQ(cross[1].stock, sm["sh600005"]);
    CHECK_EQ(cross[1].value, doctest::Approx(3.337).epsilon(0.001));

    cross = mf->getScores(Datetime(20111206));
    CHECK_EQ(cross.size(), 2);
    CHECK_EQ(cross[0].stock, sm["sh600004"]);
    CHECK_EQ(cross[0].value, doctest::Approx(7.1186).epsilon(0.001));
    CHECK_EQ(cross[1].stock, sm["sh600005"]);
    CHECK_EQ(cross[1].value, doctest::Approx(3.3209).epsilon(0.001));

    /** @arg 3 factors, 4 securities, the data length 20 and the comparison return of 3 days */
    int ndays = 3;
    src_inds = {MA(ROCR(CLOSE(), ndays)), AMA(ROCR(CLOSE(), ndays)), EMA(ROCR(CLOSE(), ndays))};
    stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    query = KQuery(-20);
    ref_k = ref_stk.getKData(query);
    ref_dates = ref_k.getDatetimeList();
    mf = MF_Weight(src_inds, weights, stks, query, ref_stk, ndays);
    mf->setParam<bool>("save_all_factors", true);
    CHECK_EQ(mf->name(), "MF_Weight");
    CHECK_THROWS_AS(mf->getFactor(sm["sh600000"]), std::exception);

    auto stk = sm["sh600004"];
    ind1 = MA(ROCR(CLOSE(stk.getKData(query)), ndays));
    ind2 = AMA(ROCR(CLOSE(stk.getKData(query)), ndays));
    auto ind3 = EMA(ROCR(CLOSE(stk.getKData(query)), ndays));
    auto ind4 = mf->getFactor(stk);
    // the warm-up days where every factor is still nan are Null now (not a fake 0.0), so the
    // discard reflects the warm-up instead of collapsing to 0
    CHECK_EQ(ind4.discard(), 3);
    for (size_t i = 0; i < ind4.discard(); i++) {
        CHECK_UNARY(std::isnan(ind4[i]));
    }
    for (size_t i = std::max(ind4.discard(), ind1.discard()), len = ref_dates.size(); i < len;
         i++) {
        CHECK_EQ(ind4[i],
                 doctest::Approx(ind1[i] * weights[0] + ind2[i] * weights[1] + ind3[i] * weights[2])
                   .epsilon(0.0001));
    }
}

/** @par Test points */
TEST_CASE("test_MF_Weight_clone") {
    StockManager& sm = StockManager::instance();
    int ndays = 3;
    IndicatorList src_inds = {MA(ROCR(CLOSE(), ndays)), AMA(ROCR(CLOSE(), ndays)),
                              EMA(ROCR(CLOSE(), ndays))};
    StockList stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    KQuery query = KQuery(-20);
    Stock ref_stk = sm["sh000001"];
    PriceList weights{0.2, 0.3, 0.5};

    auto mf1 = MF_Weight(src_inds, weights, stks, query, ref_stk, ndays);
    mf1->setParam<bool>("save_all_factors", true);
    auto ic1 = mf1->getIC();
    auto scores1 = mf1->getScores(mf1->getDatetimeList().back());

    /** @arg The cloned WeightMultiFactor carries the private weights, so the calculated results
     * are the same as the original (ISS-023: the lost m_weights made the clone read out of
     * bounds) */
    auto mf2 = mf1->clone();
    CHECK_EQ(mf2->name(), mf1->name());
    auto ic2 = mf2->getIC();
    CHECK_EQ(ic1.size(), ic2.size());
    CHECK_EQ(ic1.discard(), ic2.discard());
    CHECK_UNARY(ic1.equal(ic2));

    auto scores2 = mf2->getScores(mf1->getDatetimeList().back());
    REQUIRE_EQ(scores1.size(), scores2.size());
    for (size_t i = 0, len = scores1.size(); i < len; i++) {
        CHECK_EQ(scores1[i].stock, scores2[i].stock);
        // a warm-up day with every factor missing scores as nan; Approx(nan) never matches, so
        // compare nan positions explicitly
        if (std::isnan(scores1[i].value)) {
            CHECK_UNARY(std::isnan(scores2[i].value));
        } else {
            CHECK(scores1[i].value == doctest::Approx(scores2[i].value).epsilon(0.0001));
        }
    }

    for (const auto& stk : stks) {
        CHECK_UNARY(mf1->getFactor(stk).equal(mf2->getFactor(stk)));
    }
}

/** @par Test points */
TEST_CASE("test_MF_Weight_empty_weights") {
    StockManager& sm = StockManager::instance();
    StockList stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"]};
    KQuery query = KQuery(-20);
    IndicatorList src_inds = {MA(CLOSE()), EMA(CLOSE())};

    /** @arg the no-arg factory carries no weights; calculating a multi-factor set must throw
     * instead of indexing the empty weight list out of bounds */
    auto mf = MF_Weight();
    mf->setQuery(query);
    mf->setStockList(stks);
    mf->setRefFactorSet(FactorSet(src_inds, query.kType()));
    CHECK_THROWS_AS(mf->calculate(), std::exception);
}

/** @par Test points */
TEST_CASE("test_MF_Weight_scores_range_overflow") {
    StockManager& sm = StockManager::instance();
    IndicatorList src_inds = {MA(CLOSE())};
    StockList stks = {sm["sh600004"], sm["sh600005"]};
    KQuery query = KQuery(-30);
    Stock ref_stk = sm["sh000001"];
    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    auto dates = mf->getDatetimeList();
    auto cross = mf->getScores(dates.back());
    REQUIRE_GT(cross.size(), (size_t)0);

    /** @arg a start beyond the cross-section size returns empty instead of a reserve underflow */
    auto ret = mf->getScores(dates.back(), 1000);
    CHECK_UNARY(ret.empty());

    /** @arg a filtered range with a start beyond the cross-section size returns empty */
    auto ret2 =
      mf->getScores(dates.back(), 1000, Null<size_t>(), [](const ScoreRecord&) { return true; });
    CHECK_UNARY(ret2.empty());
}

/** @par Test points */
TEST_CASE("test_MF_Weight_scores_scores_filter") {
    StockManager& sm = StockManager::instance();
    IndicatorList src_inds = {MA(CLOSE())};
    StockList stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    KQuery query = KQuery(-30);
    Stock ref_stk = sm["sh000001"];
    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    Datetime date = mf->getDatetimeList().back();
    auto cross = mf->getScores(date);
    REQUIRE_GT(cross.size(), (size_t)0);

    ScoreRecordList expected;
    for (const auto& sc : cross) {
        if (!std::isnan(sc.value)) {
            expected.emplace_back(sc);
        }
    }

    /** @arg a full-range filter (end is Null) keeps exactly the records the predicate keeps */
    auto full = mf->getScores(date, 0, Null<size_t>(), SCFilter_IgnoreNan());
    REQUIRE_EQ(full.size(), expected.size());
    for (size_t i = 0; i < full.size(); i++) {
        CHECK_EQ(full[i].stock, expected[i].stock);
    }

    /** @arg an explicit end equal to the cross-section size takes the same full-range path */
    auto full2 = mf->getScores(date, 0, cross.size(), SCFilter_IgnoreNan());
    REQUIRE_EQ(full2.size(), expected.size());
    for (size_t i = 0; i < full2.size(); i++) {
        CHECK_EQ(full2[i].stock, expected[i].stock);
    }

    /** @arg a partial range filters only the requested slice */
    auto part = mf->getScores(date, 1, cross.size(), SCFilter_IgnoreNan());
    ScoreRecordList expected_part;
    for (size_t i = 1; i < cross.size(); i++) {
        if (!std::isnan(cross[i].value)) {
            expected_part.emplace_back(cross[i]);
        }
    }
    REQUIRE_EQ(part.size(), expected_part.size());
    for (size_t i = 0; i < part.size(); i++) {
        CHECK_EQ(part[i].stock, expected_part[i].stock);
    }
}

/** @par Test points */
TEST_CASE("test_MF_Weight_get_all_factors_requires_save_all_factors") {
    StockManager& sm = StockManager::instance();
    IndicatorList src_inds = {MA(CLOSE())};
    StockList stks = {sm["sh600004"], sm["sh600005"]};
    KQuery query = KQuery(-30);
    Stock ref_stk = sm["sh000001"];
    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);

    /** @arg save_all_factors defaults to false: both factor getters raise instead of one raising
     * and the other silently returning an empty list */
    CHECK_UNARY(!mf->getParam<bool>("save_all_factors"));
    CHECK_THROWS_AS(mf->getAllFactors(), std::exception);
    CHECK_THROWS_AS(mf->getFactor(stks[0]), std::exception);

    /** @arg enabled before the calculation, both getters agree on the same factor list */
    mf->setParam<bool>("save_all_factors", true);
    const auto& all = mf->getAllFactors();
    REQUIRE_EQ(all.size(), stks.size());
    CHECK_UNARY(mf->getFactor(stks[0]).equal(all[0]));
    CHECK_UNARY(mf->getFactor(stks[1]).equal(all[1]));
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

    int cycle = 10;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_MF_Weight_benchmark, cycle,
                           fmt::format("data len: {}", ref_k.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            auto mf = MF_Weight(src_inds, PriceList{0.2, 0.3, 0.5}, stks, query, ref_stk);
            mf->setParam<bool>("save_all_factors", true);
            auto ic = mf->getIC();
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION
/** @par Test points */
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
    mf1->setParam<bool>("save_all_factors", true);
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