/*
 * test_AllocateFunds.cpp
 *
 *  Created on: 2018-2-11
 *      Author: fasiondog
 */


#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_trade_sys_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>

using namespace hku;

/**
 * @defgroup test_AllocateFunds test_AllocateFunds
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_AllocateFunds) {
    StockManager& sm = StockManager::instance();

    SEPtr se = SE_Fixed();
    AFPtr af = AF_EqualWeight();
    TMPtr tm = crtTM(Datetime(200101010000L), 100000);
    TMPtr subtm = crtTM(Datetime(200101010000L), 0);
    SYSPtr sys = SYS_Simple(subtm->clone());

    SystemList se_list;
    SystemList hold_list;
    SystemList ac_list;
    SystemWeightList sw_list;

    /** @arg 测试setTM */
    BOOST_CHECK(!af->getTM());
    af->setTM(tm);
    BOOST_CHECK(af->getTM());

    /** @arg 测试clone */
    AFPtr af2 = af->clone();
    BOOST_CHECK(af2.get() != af.get());
    BOOST_CHECK(af2->name() == af->name());
    BOOST_CHECK(af2->getTM() == af->getTM());

    /** @arg 出入的se_list、hold_list均为空  */
    BOOST_CHECK(se_list.size() == 0);
    BOOST_CHECK(hold_list.size() == 0);
    sw_list = af->_allocateWeight(Datetime(201802100000L), se_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    BOOST_CHECK(sw_list.size() == 0);
    BOOST_CHECK(ac_list.size() == 0);

    /** @arg 最大持仓系统数小于0 */
    af->setParam<int>("max_sys_num", -1);
    se->clear();
    tm->reset();
    sys->setTM(subtm->clone());
    se->addStock(sm["sh600000"], sys);
    se_list = se->getSelectedSystemList(Datetime(201802100000L));
    BOOST_CHECK(se_list.size() == 1);

    sw_list = af->_allocateWeight(Datetime(201802100000L), se_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, se_list);
    BOOST_CHECK(sw_list.size() == 1);
    BOOST_CHECK(sw_list[0].getSYS() == se_list[0]);
    BOOST_CHECK(ac_list.size() == 0);
    BOOST_CHECK(tm->currentCash() == 100000);


    /** @arg 最大持仓系统数为0 */
    af->setParam<int>("max_sys_num", 0);
    tm->reset();
    se->clear();
    sys->setTM(subtm->clone());
    se->addStock(sm["sh600000"], sys);
    se_list = se->getSelectedSystemList(Datetime(201802100000L));
    BOOST_CHECK(se_list.size() == 1);

    sw_list = af->_allocateWeight(Datetime(201802100000L), se_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    BOOST_CHECK(sw_list.size() == 1);
    BOOST_CHECK(ac_list.size() == 0);
    BOOST_CHECK(tm->currentCash() == 100000);
}

/** @} */





