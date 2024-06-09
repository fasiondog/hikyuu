/*
 * test_Signal.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_SE_Fixed test_SE_Fixed
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SE_Fixed") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SEPtr se = SE_Fixed();

    /** @arg 试图加入一个不存在的stock */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg 试图加入一个空的系统策略原型 */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    // /** @arg 试图加入一个缺少MM | SG的系统策略原型 */
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);
    sys->setSG(sg);
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], sys), std::exception);

    sys->setSG(SGPtr());
    sys->setMM(mm);
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], sys), std::exception);

    // 目前必须有PF指定实际执行的子系统，下面代码无法执行
    // /** @arg getSelectedSystemList */
    sys->setSG(sg);
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);

    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());

    /** @arg reset */
    se->reset();
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 0);

    /** @arg 克隆操作 */
    proto_sys_list = se->getProtoSystemList();
    se->calculate(proto_sys_list, KQuery(-20));
    SEPtr se2;
    se2 = se->clone();
    CHECK_NE(se2.get(), se.get());
    result = se2->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SE_Fixed_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SE_FIXED.xml";

    TMPtr tm = crtTM(Datetime(20010101), 100000);
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);
    SYSPtr sys = SYS_Simple();
    sys->setTM(tm);
    sys->setSG(sg);
    sys->setMM(mm);
    StockList stkList;
    stkList.push_back(sm["sh600000"]);
    stkList.push_back(sm["sz000001"]);

    SEPtr se1 = SE_Fixed(stkList, sys);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(se1);
    }

    SEPtr se2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(se2);
    }

    CHECK_EQ(se1->name(), se2->name());
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
