/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-06-09
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_MultiFactor.h>
#include <hikyuu/trade_sys/selector/crt/SE_MultiFactor2.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cycle.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_EqualWeight.h>
#include <set>

using namespace hku;

/**
 * @defgroup test_SE_MultiFactor test_SE_MultiFactor
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SE_MultiFactor") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111206));
    IndicatorList src_inds{MA(CLOSE())};

    auto sys = SYS_Simple(crtTM(), MM_Nothing());
    sys->setSG(SG_Cycle());
    sys->setParam<bool>("buy_delay", false);

    /** @arg Test trying to change a parameter into an illegal value */
    auto ret = SE_MultiFactor(src_inds);
    CHECK_THROWS(ret->setParam<int>("ic_n", 0));
    CHECK_THROWS(ret->setParam<int>("ic_rolling_n", 0));
    CHECK_THROWS(ret->setParam<string>("mode", "MF"));
    /** @arg mf_recover_type out of the valid range (v1 and v2) */
    CHECK_THROWS(ret->setParam<int>("mf_recover_type", 999));
    CHECK_THROWS(ret->setParam<int>("mf_recover_type", -1));
    /** @arg mf_recover_type on the valid boundaries is accepted */
    CHECK_NOTHROW(ret->setParam<int>("mf_recover_type", KQuery::NO_RECOVER));
    CHECK_NOTHROW(ret->setParam<int>("mf_recover_type", KQuery::INVALID_RECOVER_TYPE));

    auto ret2 = SE_MultiFactor2(src_inds);
    CHECK_THROWS(ret2->setParam<int>("mf_recover_type", 999));
    CHECK_THROWS(ret2->setParam<int>("mf_recover_type", -1));

    /** @arg src_inds is empty and the others are the default parameters */
    CHECK_THROWS(SE_MultiFactor(IndicatorList{}));

    /** @arg The default parameters */
    ret = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_ICIRWeight");
    ret->addStockList(stks, sys);
    auto proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    auto sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 3);

    ret = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_EqualWeight");
    ret->addStockList(stks, sys);
    proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 3);

    ret = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_ICWeight");
    ret->addStockList(stks, sys);
    proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 3);

    /** @arg topn = 2 */
    ret = SE_MultiFactor(src_inds, 2, 5, 120, ref_stk);
    ret->addStockList(stks, sys);
    proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 2);

    // for (const auto& sw : sw_list) {
    //     HKU_INFO("{} {}", sw.sys->name(), sw.weight);
    // }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SE_MultiFactor_export") {
    StockManager& sm = StockManager::instance();
    string filename(fmt::format("{}/SE_MultiFactor.xml", sm.tmpdir()));

    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111206));
    IndicatorList src_inds{MA(CLOSE())};

    auto sys = SYS_Simple(crtTM(), MM_Nothing());
    sys->setSG(SG_Cycle());
    sys->setParam<bool>("buy_delay", false);

    auto x1 = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk);
    x1->addStockList(stks, sys);

    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    SelectorPtr x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    auto proto_list = x2->getProtoSystemList();
    x2->calculate(proto_list, query);
    auto sw_list = x2->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 3);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @par Test points */
TEST_CASE("test_SE_MultiFactor_recalculate") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    IndicatorList src_inds{MA(CLOSE())};

    auto sys = SYS_Simple(crtTM(), MM_Nothing());
    sys->setSG(SG_Cycle());
    sys->setParam<bool>("buy_delay", false);

    auto se = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_EqualWeight");
    se->addStockList(stks, sys);
    auto proto_list = se->getProtoSystemList();

    KQuery query1 = KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111206));
    KQuery query2 = KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111207));

    /** @arg The first calculation runs normally */
    CHECK_NOTHROW(se->calculate(proto_list, query1));

    /** @arg The recalculation with a changed query no longer throws the out_of_range of the
     * undeclared parameter */
    CHECK_NOTHROW(se->calculate(proto_list, query2));
    auto sw_list = se->getSelected(Datetime(20111206));
    CHECK_UNARY(!sw_list.empty());
}

/** @par Test points */
TEST_CASE("test_SE_MultiFactor_custom_mf") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    IndicatorList src_inds{MA(CLOSE())};

    auto sys = SYS_Simple(crtTM(), MM_Nothing());
    sys->setSG(SG_Cycle());
    sys->setParam<bool>("buy_delay", false);

    KQuery mf_query = KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111206), KQuery::DAY);
    auto mf = MF_EqualWeight(src_inds, stks, mf_query, ref_stk);
    CHECK_EQ(mf->getQuery().recoverType(), KQuery::NO_RECOVER);

    auto se = SE_MultiFactor(mf, 2);
    se->addStockList(stks, sys);
    auto proto_list = se->getProtoSystemList();

    KQuery query =
      KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111206), KQuery::DAY, KQuery::FORWARD);

    /** @arg keep_mf_recover_type=true (set before the first calculation): the mf keeps its own
     * recover type (NO_RECOVER) even if the query uses FORWARD, and the calculation no longer
     * throws the out_of_range of the undeclared parameter */
    se->setParam<bool>("keep_mf_recover_type", true);
    CHECK_NOTHROW(se->calculate(proto_list, query));
    CHECK_EQ(mf->getQuery().recoverType(), KQuery::NO_RECOVER);

    /** @arg keep_mf_recover_type=false: the recover type of the mf follows the query */
    se->setParam<bool>("keep_mf_recover_type", false);
    KQuery query2 =
      KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111207), KQuery::DAY, KQuery::FORWARD);
    CHECK_NOTHROW(se->calculate(proto_list, query2));
    CHECK_EQ(mf->getQuery().recoverType(), KQuery::FORWARD);
}

/** @par Test points */
TEST_CASE("test_SE_MultiFactor_clone_without_mf") {
    /** @arg Cloning before the MF is set does not crash and keeps the empty mf (ISS-089) */
    auto se1 = SE_MultiFactor();
    SelectorPtr se2;
    CHECK_NOTHROW(se2 = se1->clone());
    CHECK_UNARY(se2);

    auto se3 = SE_MultiFactor2();
    SelectorPtr se4;
    CHECK_NOTHROW(se4 = se3->clone());
    CHECK_UNARY(se4);
}

/** @par Check point: a second calculation of the same selector replaces the mapping from the
 *  instruments to the running systems instead of keeping it (the systems of the earlier run must
 *  not survive into the next one) */
TEST_CASE("test_SE_MultiFactor_recalculate_mapping") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    IndicatorList src_inds{MA(CLOSE())};

    auto sys = SYS_Simple(crtTM(), MM_Nothing());
    sys->setSG(SG_Cycle());
    sys->setParam<bool>("buy_delay", false);

    auto se = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_EqualWeight");
    se->addStockList(stks, sys);
    auto proto_list = se->getProtoSystemList();

    KQuery query1 = KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111206));
    se->calculate(proto_list, query1);
    auto sel1 = se->getSelected(Datetime(20110712));
    REQUIRE_UNARY(!sel1.empty());

    // The second run passes a different real system list (the same instruments, new instances) and
    // a different query
    SystemList real2;
    for (const auto& proto : proto_list) {
        real2.emplace_back(proto->clone());
    }
    std::set<System*> new_set;
    for (const auto& s : real2) {
        new_set.insert(s.get());
    }

    KQuery query2 = KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111207));
    se->calculate(real2, query2);

    auto sel2 = se->getSelected(Datetime(20111206));
    REQUIRE_UNARY(!sel2.empty());
    for (const auto& sw : sel2) {
        /** @arg The systems selected after the recalculation are the new running instances */
        CHECK_EQ(new_set.count(sw.sys.get()), 1);
    }
}

/** @par Check point: a cloned MF selector must not carry over the mapping of the original running
 *  systems; after the recalculation it selects its own instances */
TEST_CASE("test_SE_MultiFactor2_clone_mapping") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    IndicatorList src_inds{MA(CLOSE())};

    auto sys = SYS_Simple(crtTM(), MM_Nothing());
    sys->setSG(SG_Cycle());
    sys->setParam<bool>("buy_delay", false);

    auto se = SE_MultiFactor2(src_inds, 5, 120, ref_stk, true, "MF_EqualWeight");
    se->addStockList(stks, sys);
    auto proto_list = se->getProtoSystemList();

    KQuery query = KQueryByDate(Datetime(20110712) - Days(30), Datetime(20111206));
    se->calculate(proto_list, query);
    auto sel1 = se->getSelected(Datetime(20110712));
    REQUIRE_UNARY(!sel1.empty());
    REQUIRE_UNARY(sel1[0].sys);

    std::set<System*> orig_set;
    for (const auto& s : proto_list) {
        orig_set.insert(s.get());
    }

    auto cloned = se->clone();
    REQUIRE_UNARY(cloned);
    cloned->calculate(cloned->getProtoSystemList(), query);
    auto sel2 = cloned->getSelected(Datetime(20110712));
    REQUIRE_UNARY(!sel2.empty());

    /** @arg The clone selects its own running instances, not the ones of the original selector */
    for (const auto& sw : sel2) {
        REQUIRE_UNARY(sw.sys);
        CHECK_EQ(orig_set.count(sw.sys.get()), 0);
    }
}

/** @par Test points */
TEST_CASE("test_SE_MultiFactor_getSelected_without_mf") {
    /** @arg getSelected on a selector without an MF throws instead of dereferencing a null MF */
    auto se = SE_MultiFactor();
    CHECK_THROWS_AS(se->getSelected(Datetime(202001010000L)), std::exception);

    /** @arg The same applies to SE_MultiFactor2 */
    auto se2 = SE_MultiFactor2();
    CHECK_THROWS_AS(se2->getSelected(Datetime(202001010000L)), std::exception);
}

/** @} */