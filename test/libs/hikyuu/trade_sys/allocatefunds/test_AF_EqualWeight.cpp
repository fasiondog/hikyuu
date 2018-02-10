/*
 * test_Signal.cpp
 *
 *  Created on: 2013-3-11
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
BOOST_AUTO_TEST_CASE( test_AF_EqualWeight) {
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
    sw_list = af->allocateWeight(se_list, hold_list);
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

    sw_list = af->allocateWeight(se_list, hold_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, se_list);
    BOOST_CHECK(sw_list.size() == 0);
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

    sw_list = af->allocateWeight(se_list, hold_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    BOOST_CHECK(sw_list.size() == 0);
    BOOST_CHECK(ac_list.size() == 0);
    BOOST_CHECK(tm->currentCash() == 100000);

    /** @arg 当前选中系统数等于最大持仓系统数，已持仓系统数为0，不调整已持仓系统 */
    af->setParam<bool>("adjust_hold_sys", false);
    af->setParam<int>("max_sys_num", 1);
    tm->reset();
    se->clear();
    sys->setTM(subtm->clone());
    se->addStock(sm["sh600000"], sys);
    se_list = se->getSelectedSystemList(Datetime(201802100000L));
    BOOST_CHECK(se_list.size() == 1);
    BOOST_CHECK(hold_list.size() == 0);
    BOOST_CHECK(se_list[0]->getTM()->currentCash() == 0);

    sw_list = af->allocateWeight(se_list, hold_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    BOOST_CHECK(sw_list.size() == 1);
    BOOST_CHECK(sw_list[0].sys->getStock() == sm["sh600000"]);
    BOOST_CHECK(sw_list[0].weight == 100);
    BOOST_CHECK(se_list[0]->getTM()->currentCash() == 100000);
    BOOST_CHECK(tm->currentCash() == 0);
    BOOST_CHECK(ac_list.size() == 1);
    BOOST_CHECK(ac_list[0] == sw_list[0].sys);

    /** @arg 当前选中系统数大于最大持仓系统数，已持仓系统数为0，不调整已持仓系统 */
    std::cout << "***********************" <<std::endl;
    af->setParam<bool>("adjust_hold_sys", false);
    af->setParam<int>("max_sys_num", 1);
    tm->reset();
    se->clear();
    sys->setTM(subtm->clone());
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sh600004"], sys);
    se_list = se->getSelectedSystemList(Datetime(201802100000L));
    BOOST_CHECK(se_list.size() > af->getParam<int>("max_sys_num"));
    BOOST_CHECK(hold_list.size() == 0);
    BOOST_CHECK(se_list[0]->getTM()->currentCash() == 0);
    BOOST_CHECK(se_list[1]->getTM()->currentCash() == 0);
    BOOST_CHECK(tm->currentCash() == 100000);

    sw_list = af->allocateWeight(se_list, hold_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    BOOST_CHECK(sw_list.size() == 1);
    BOOST_CHECK(sw_list[0].sys->getStock() == sm["sh600000"]);
    BOOST_CHECK(sw_list[0].weight == 50);
    BOOST_CHECK(se_list[0]->getTM()->currentCash() == 100000);
    BOOST_CHECK(tm->currentCash() == 0);
    BOOST_CHECK(ac_list.size() == 1);
    BOOST_CHECK(ac_list[0] == sw_list[0].sys);

    BOOST_CHECK(tm == af->getTM());
    std::cout << tm->currentCash() << std::endl;
    std::cout << "sw_list:===============>" << std::endl;
    for (auto iter = sw_list.begin(); iter != sw_list.end(); ++iter) {
        std::cout << *iter << std::endl;
    }
    std::cout << "ac_list:===============>" << std::endl;
    for (auto iter = ac_list.begin(); iter != ac_list.end(); ++iter) {
        std::cout << (*iter)->getTM()->currentCash() << std::endl;
    }

    /** @arg  */
    /*se_list.push_back(sm["sh600000"]);
    se_list.push_back(sm["sh600004"]);
    se_list.push_back(sm["sh600005"]);
    se_list.push_back(sm["sz000001"]);
    se_list.push_back(sm["sz000002"]);
    se_list.push_back(sm["sz000004"]);
    se_list.push_back(sm["sz000005"]);*/


}

/** @} */





