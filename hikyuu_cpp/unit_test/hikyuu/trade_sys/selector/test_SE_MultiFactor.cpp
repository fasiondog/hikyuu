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
#include <hikyuu/trade_sys/selector/imp/MultiFactorSelector.h>
#include <hikyuu/trade_sys/selector/imp/MultiFactorSelector2.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cycle.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_EqualWeight.h>

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
    auto se1 = make_shared<MultiFactorSelector>();
    SelectorPtr se2;
    CHECK_NOTHROW(se2 = se1->clone());
    CHECK_UNARY(se2);

    auto se3 = make_shared<MultiFactorSelector2>();
    SelectorPtr se4;
    CHECK_NOTHROW(se4 = se3->clone());
    CHECK_UNARY(se4);
}

/** @} */