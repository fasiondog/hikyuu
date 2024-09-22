/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-16
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/trade_sys/system/crt/SYS_WalkForward.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/NOT.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/signal/crt/SG_Bool.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>

using namespace hku;

/**
 * @defgroup test_SYS_WalkForward test_SYS_WalkForward
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

static SYSPtr create_test_sys(int fast_n, int slow_n) {
    auto ind = MA(CLOSE(), fast_n) > MA(CLOSE(), slow_n);
    auto sg = SG_Bool(ind, NOT(ind));
    auto tm = crtTM();
    auto sys = SYS_Simple(tm, MM_Nothing(), EnvironmentPtr(), ConditionPtr(), sg);
    sys->name(fmt::format("test_sys_{}_{}", fast_n, slow_n));
    return sys;
}

/** @par 检测点 */
TEST_CASE("test_SYS_WalkForword") {
    Stock stk = getStock("sz000001");
    KQuery query = KQueryByIndex(-50);
    TMPtr tm = crtTM();

    /** @arg 候选系统列表为空 */
    CHECK_THROWS(SYS_WalkForward(SystemList(), tm));

    /** @arg 未指定tm */
    SystemList candidate_sys_list{create_test_sys(3, 5)};
    CHECK_THROWS(SYS_WalkForward(candidate_sys_list));

    /** @arg 执行时未指定证券标的 */
    auto sys = SYS_WalkForward(SystemList{create_test_sys(3, 5)}, tm, 30, 20);
    CHECK_THROWS(sys->run(query));

    /** @arg 只有一个候选系统 */
    sys = SYS_WalkForward(SystemList{create_test_sys(3, 5)}, tm, 30, 20);
    CHECK_EQ(sys->name(), "SYS_WalkForward");
    query = KQueryByIndex(-125);
    // sys->setParam<bool>("trace", true);
    sys->run(stk, query);

    auto delay_request = sys->getBuyTradeRequest();
    CHECK_UNARY(delay_request.valid);
    CHECK_EQ(delay_request.business, BUSINESS_BUY);
    CHECK_EQ(delay_request.datetime, Datetime(20111205));

    tm = sys->getTM();
    CHECK_EQ(tm->currentCash(), 99328.0);

    auto tr_list1 = tm->getTradeList();
    auto tr_list2 = sys->getTradeRecordList();
    CHECK_EQ(tr_list1.size(), tr_list2.size() + 1);
    for (size_t i = 0, total = tr_list2.size(); i < total; i++) {
        CHECK_EQ(tr_list1[i + 1], tr_list2[i]);
    }

    /** @arg 多个后续系统 */
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    SystemList sys_list;
    for (const auto& param : params) {
        sys_list.emplace_back(create_test_sys(param.first, param.second));
    }
    tm->reset();
    REQUIRE(tm->getTradeList().size() == 1);
    sys = SYS_WalkForward(sys_list, tm, 30, 20);
    query = KQueryByIndex(-125);
    // sys->setParam<bool>("trace", true);
    sys->run(stk, query);

    delay_request = sys->getBuyTradeRequest();
    CHECK_UNARY(delay_request.valid);
    CHECK_EQ(delay_request.business, BUSINESS_BUY);
    CHECK_EQ(delay_request.datetime, Datetime(20111205));

    tm = sys->getTM();
    CHECK_EQ(tm->currentCash(), 98286.0);

    tr_list1 = tm->getTradeList();
    tr_list2 = sys->getTradeRecordList();
    CHECK_EQ(tr_list1.size(), tr_list2.size() + 1);
    for (size_t i = 0, total = tr_list2.size(); i < total; i++) {
        CHECK_EQ(tr_list1[i + 1], tr_list2[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SYS_WalkForword_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SYS_WalkForward.xml";

    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    SystemList sys_list;
    for (const auto& param : params) {
        sys_list.emplace_back(create_test_sys(param.first, param.second));
    }
    TMPtr tm = crtTM();
    Stock stk = getStock("sz000001");
    auto sys1 = SYS_WalkForward(sys_list, tm, 30, 20);
    auto query = KQueryByIndex(-125);
    sys1->run(stk, query);
    auto tr_list1 = sys1->getTradeRecordList();
    sys1->reset();

    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(sys1);
    }

    SYSPtr sys2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(sys2);
    }

    CHECK_EQ(sys1->name(), sys2->name());

    sys2->run(stk, query);
    auto tr_list2 = sys2->getTradeRecordList();
    CHECK_EQ(tr_list1.size(), tr_list2.size());
    for (size_t i = 0, total = tr_list2.size(); i < total; i++) {
        CHECK_EQ(tr_list1[i], tr_list2[i]);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */