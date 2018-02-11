/*
 * test_AF_EqualWeight.cpp
 *
 *  Created on: 2018-2-10
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

/** @par 检测点，不自动调仓 */
BOOST_AUTO_TEST_CASE( test_AF_EqualWeight_not_adjust_hold) {
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
    BOOST_CHECK(se_list.size() == 1);
    BOOST_CHECK(hold_list.size() == 0);
    BOOST_CHECK(se_list[0]->getTM()->currentCash() == 0);

    sw_list = af->_allocateWeight(Datetime(201802100000L), se_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    BOOST_CHECK(sw_list.size() == 1);
    BOOST_CHECK(sw_list[0].sys->getStock() == sm["sh600000"]);
    BOOST_CHECK(sw_list[0].weight == 1);
    BOOST_CHECK(se_list[0]->getTM()->currentCash() == 100000);
    BOOST_CHECK(tm->currentCash() == 0);
    BOOST_CHECK(ac_list.size() == 1);
    BOOST_CHECK(ac_list[0] == sw_list[0].sys);

    /** @arg 当前选中系统数大于最大持仓系统数，已持仓系统数为0 */
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

    sw_list = af->_allocateWeight(Datetime(201802100000L), se_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    BOOST_CHECK(sw_list.size() == 2);
    BOOST_CHECK(sw_list[0].sys->getStock() == sm["sh600000"]);
    BOOST_CHECK(sw_list[0].weight == 1);
    BOOST_CHECK(se_list[0]->getTM()->currentCash() == 100000);
    BOOST_CHECK(tm->currentCash() == 0);
    BOOST_CHECK(ac_list.size() == 1);
    BOOST_CHECK(ac_list[0] == sw_list[1].sys);

    /** @arg 当前选中系统数不为0，已持仓系统数等于最大持仓数 */
    /*af->setParam<int>("max_sys_num", 2);
    tm->reset();
    se->clear();
    sys->setTM(subtm->clone());
    se->addStock(sm["sh600000"], sys);
    se_list = se->getSelectedSystemList(Datetime(201802100000L));
    se->clear();
    se->addStock(sm["sh600004"], sys);
    se->addStock(sm["sz000001"], sys);
    hold_list = se->getSelectedSystemList(Datetime(201802100000L));
    BOOST_CHECK(se_list.size() != 0);
    BOOST_CHECK(hold_list.size() == af->getParam<int>("max_sys_num"));
    BOOST_CHECK(hold_list[0]->getTM()->currentCash() == 0);
    BOOST_CHECK(hold_list[1]->getTM()->currentCash() == 0);
    BOOST_CHECK(tm->currentCash() == 100000);

    sw_list = af->allocateWeight(Datetime(201802100000L), se_list, hold_list);
    ac_list = af->getAllocatedSystemList(Datetime(201802100000L), se_list, hold_list);
    BOOST_CHECK(sw_list.size() == 1);
    BOOST_CHECK(sw_list[0].sys->getStock() == sm["sh600000"]);
    BOOST_CHECK(sw_list[0].weight == 50);
    BOOST_CHECK(se_list[0]->getTM()->currentCash() == 100000);
    BOOST_CHECK(tm->currentCash() == 0);
    BOOST_CHECK(ac_list.size() == 1);
    BOOST_CHECK(ac_list[0] == sw_list[0].sys);*/


    /** @arg 当前选中系统数不为0，已持仓系统数不为零且小于最大持仓数 */

    /** @arg 当前选中系统数不为0，已持仓系统数大于最大持仓数 */


    BOOST_CHECK(tm == af->getTM());
    std::cout << tm->currentCash() << std::endl;
    std::cout << "sw_list:===============>" << std::endl;
    for (auto iter = sw_list.begin(); iter != sw_list.end(); ++iter) {
        std::cout << *iter << std::endl;
    }
    std::cout << "ac_list:===============>" << std::endl;
    for (auto iter = ac_list.begin(); iter != ac_list.end(); ++iter) {
        std::cout << (*iter)->getTM()->currentCash() << std::endl;
        std::cout << (*iter)->getStock() << std::endl;
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


/** @par 检测点，自动调仓 */
BOOST_AUTO_TEST_CASE( test_AF_EqualWeight_adjust_hold) {

}

/** @} */





