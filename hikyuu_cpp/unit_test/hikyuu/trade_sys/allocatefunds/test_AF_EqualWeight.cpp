/*
 * test_AF_EqualWeight.cpp
 *
 *  Created on: 2018-2-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
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

/** @par 检测点，不自动调仓 */
TEST_CASE("test_AF_EqualWeight_not_adjust_hold") {
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

    af->setTM(tm);
    af->setParam<bool>("adjust_hold_sys", false);

    /** @arg 当前选中系统数等于最大持仓系统数，已持仓系统数为0 */
    af->setParam<bool>("adjust_hold_sys", false);
    af->setParam<int>("max_sys_num", 1);
    tm->reset();
    se->clear();
    sys->setTM(subtm->clone());
    se->addStock(sm["sh600000"], sys);
    se_list = se->getSelectedSystemList(Datetime(201802100000L));
    CHECK_EQ(se_list.size(), 1);
    CHECK_EQ(hold_list.size(), 0);
    CHECK_EQ(se_list[0]->getTM()->currentCash(), 0);

    sw_list = af->_allocateWeight(Datetime(201802100000L), se_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    CHECK_EQ(sw_list.size(), 1);
    CHECK_EQ(sw_list[0].getSYS()->getStock(), sm["sh600000"]);
    CHECK_EQ(sw_list[0].getWeight(), 1);
    CHECK_EQ(se_list[0]->getTM()->currentCash(), 100000);
    CHECK_EQ(tm->currentCash(), 0);
    CHECK_EQ(ac_list.size(), 1);
    CHECK_EQ(ac_list[0], sw_list[0].getSYS());

    /** @arg 当前选中系统数大于最大持仓系统数，已持仓系统数为0 */
    af->setParam<int>("max_sys_num", 1);
    tm->reset();
    se->clear();
    sys->setTM(subtm->clone());
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sh600004"], sys);
    se_list = se->getSelectedSystemList(Datetime(201802100000L));
    CHECK_GT(se_list.size(), af->getParam<int>("max_sys_num"));
    CHECK_EQ(hold_list.size(), 0);
    CHECK_EQ(se_list[0]->getTM()->currentCash(), 0);
    CHECK_EQ(se_list[1]->getTM()->currentCash(), 0);
    CHECK_EQ(tm->currentCash(), 100000);

    sw_list = af->_allocateWeight(Datetime(201802100000L), se_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    CHECK_EQ(sw_list.size(), 2);
    CHECK_EQ(sw_list[0].getSYS()->getStock(), sm["sh600000"]);
    CHECK_EQ(sw_list[0].getWeight(), 1);
    CHECK_EQ(se_list[0]->getTM()->currentCash(), 0);
    CHECK_EQ(tm->currentCash(), 0);
    CHECK_EQ(ac_list.size(), 1);
    CHECK_EQ(ac_list[0], sw_list[1].getSYS());
    CHECK_EQ(ac_list[0]->getTM()->currentCash(), 100000);

    /** @arg 当前选中系统数不为0，已持仓系统数等于最大持仓数 */
    af->setParam<int>("max_sys_num", 2);
    tm->reset();
    se->clear();
    sys->setTM(subtm->clone());
    se->addStock(sm["sh600000"], sys);
    se_list = se->getSelectedSystemList(Datetime(201802100000L));
    se->clear();
    se->addStock(sm["sh600004"], sys);
    se->addStock(sm["sz000001"], sys);
    hold_list = se->getSelectedSystemList(Datetime(201802100000L));
    CHECK_NE(se_list.size(), 0);
    CHECK_EQ(hold_list.size(), af->getParam<int>("max_sys_num"));
    CHECK_EQ(tm->currentCash(), 100000);

    sw_list = af->_allocateWeight(Datetime(201802100000L), se_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    CHECK_EQ(sw_list.size(), 1);
    CHECK_EQ(sw_list[0].getSYS()->getStock(), sm["sh600000"]);
    CHECK_EQ(sw_list[0].getWeight(), 1);
    CHECK_EQ(se_list[0]->getTM()->currentCash(), 0);
    CHECK_EQ(tm->currentCash(), 100000);
    CHECK_EQ(ac_list.size(), 2);
    CHECK_EQ(ac_list[0]->getStock(), sm["sh600004"]);
    CHECK_EQ(ac_list[1]->getStock(), sm["sz000001"]);
    CHECK_EQ(ac_list[0]->getTM()->currentCash(), 0);
    CHECK_EQ(ac_list[1]->getTM()->currentCash(), 0);

    /** @arg 当前选中系统数不为0，已持仓系统数大于最大持仓数 */
    af->setParam<int>("max_sys_num", 1);
    tm->reset();
    se->clear();
    sys->setTM(subtm->clone());
    se->addStock(sm["sh600000"], sys);
    se_list = se->getSelectedSystemList(Datetime(201802100000L));
    se->clear();
    se->addStock(sm["sh600004"], sys);
    se->addStock(sm["sz000001"], sys);
    hold_list = se->getSelectedSystemList(Datetime(201802100000L));
    CHECK_NE(se_list.size(), 0);
    CHECK_GT(hold_list.size(), af->getParam<int>("max_sys_num"));
    CHECK_EQ(tm->currentCash(), 100000);
    CHECK_EQ(hold_list[0]->getTM()->currentCash(), 0);
    CHECK_EQ(hold_list[1]->getTM()->currentCash(), 0);

    sw_list = af->_allocateWeight(Datetime(201802100000L), se_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    CHECK_EQ(sw_list.size(), 1);
    CHECK_EQ(sw_list[0].getSYS()->getStock(), sm["sh600000"]);
    CHECK_EQ(sw_list[0].getWeight(), 1);
    CHECK_EQ(se_list[0]->getTM()->currentCash(), 0);
    CHECK_EQ(tm->currentCash(), 100000);
    CHECK_EQ(ac_list.size(), 2);
    CHECK_EQ(ac_list[0]->getStock(), sm["sh600004"]);
    CHECK_EQ(ac_list[1]->getStock(), sm["sz000001"]);
    CHECK_EQ(ac_list[0]->getTM()->currentCash(), 0);
    CHECK_EQ(ac_list[1]->getTM()->currentCash(), 0);

    /** @arg 当前选中系统数不为0且不包含持仓系统，持仓系统数不为零且小于最大持仓数 */
    af->setParam<int>("max_sys_num", 4);
    tm->reset();
    se->clear();
    sys->setTM(subtm->clone());
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sh600005"], sys);
    se->addStock(sm["sz000002"], sys);
    se->addStock(sm["sz000004"], sys);
    se->addStock(sm["sz000005"], sys);
    se_list = se->getSelectedSystemList(Datetime(201802100000L));
    se->clear();
    se->addStock(sm["sh600004"], sys);
    se->addStock(sm["sz000001"], sys);
    hold_list = se->getSelectedSystemList(Datetime(201802100000L));
    CHECK_NE(se_list.size(), 0);
    CHECK_LT(hold_list.size(), af->getParam<int>("max_sys_num"));
    CHECK_EQ(tm->currentCash(), 100000);
    CHECK_EQ(hold_list[0]->getTM()->currentCash(), 0);
    CHECK_EQ(hold_list[1]->getTM()->currentCash(), 0);

    sw_list = af->_allocateWeight(Datetime(201802100000L), se_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    CHECK_EQ(sw_list.size(), 5);
    CHECK_EQ(tm->currentCash(), 0);
    CHECK_EQ(ac_list.size(), 4);
    std::set<Stock> stk_set;

    for (auto iter = ac_list.begin(); iter != ac_list.end(); ++iter) {
        stk_set.insert((*iter)->getStock());
    }
    CHECK_NE(stk_set.find(sm["sh600004"]), stk_set.end());
    CHECK_NE(stk_set.find(sm["sz000001"]), stk_set.end());
    CHECK_EQ(ac_list[0], hold_list[0]);
    CHECK_EQ(ac_list[1], hold_list[1]);
    CHECK_EQ(ac_list[0]->getTM()->currentCash(), 0);
    CHECK_EQ(ac_list[1]->getTM()->currentCash(), 0);

    /*
    std::cout << "current cash: " << tm->currentCash() << std::endl;
    std::cout << "sw_list:===============>" << std::endl;
    for (auto iter = sw_list.begin(); iter != sw_list.end(); ++iter) {
        std::cout << *iter << std::endl;;
    }

    std::cout << std::fixed;
    std::cout.precision(4);

    std::cout << "ac_list:===============>" << std::endl;
    for (auto iter = ac_list.begin(); iter != ac_list.end(); ++iter) {
        std::cout << (*iter)->getTM()->currentCash() << std::endl;
        std::cout << (*iter)->getStock() << std::endl;
    }

    std::cout.unsetf(std::ostream::floatfield);
    std::cout.precision();
    */
}

/** @par 检测点，自动调仓 */
// TEST_CASE( test_AF_EqualWeight_adjust_hold) {
// TODO test_AF_EqualWeight_adjust_hold
//}

/** @} */
