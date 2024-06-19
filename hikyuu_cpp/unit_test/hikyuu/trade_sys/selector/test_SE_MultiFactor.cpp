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
#include <hikyuu/trade_sys/signal/crt/SG_Cycle.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/EMA.h>

using namespace hku;

/**
 * @defgroup test_SE_MultiFactor test_SE_MultiFactor
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SE_MultiFactor") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQueryByDate(Datetime(20110712), Datetime(20111206));
    IndicatorList src_inds{MA(CLOSE())};

    auto sys = SYS_Simple(crtTM(), MM_Nothing());
    sys->setSG(SG_Cycle());
    sys->setParam<bool>("buy_delay", false);

    /** @arg 测试试图修改参数值为非法值 */
    auto ret = SE_MultiFactor(src_inds);
    CHECK_THROWS(ret->setParam<int>("ic_n", 0));
    CHECK_THROWS(ret->setParam<int>("ic_rolling_n", 0));
    CHECK_THROWS(ret->setParam<string>("mode", "MF"));

    /** @arg src_inds 为空，其余为默认参数 */
    CHECK_THROWS(SE_MultiFactor(IndicatorList{}));

    /** @arg 默认参数 */
    ret = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_ICIRWeight");
    ret->addStockList(stks, sys);
    auto proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    auto sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 4);

#if 0
    ret = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_EqualWeight");
    ret->addStockList(stks, sys);
    proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 4);

    ret = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk, "MF_ICWeight");
    ret->addStockList(stks, sys);
    proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
    sw_list = ret->getSelected(Datetime(20110712));
    CHECK_EQ(sw_list.size(), 4);
#endif

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

/** @par 检测点 */
TEST_CASE("test_SE_MultiFactor_export") {
    StockManager& sm = StockManager::instance();
    string filename(fmt::format("{}/SE_MultiFactor.xml", sm.tmpdir()));

    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQueryByDate(Datetime(20110712), Datetime(20111206));
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
    CHECK_EQ(sw_list.size(), 4);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */