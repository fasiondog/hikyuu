/*
 * test_AF_EqualWeight.cpp
 *
 *  Created on: 2018-2-10
 *      Author: fasiondog
 */

#include "../../test_config.h"
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
    /** @arg 当前选中系统数大于最大持仓系统数，已持仓系统数为0 */

    /** @arg 当前选中系统数不为0，已持仓系统数等于最大持仓数 */
    /** @arg 当前选中系统数不为0，已持仓系统数大于最大持仓数 */
    /** @arg 当前选中系统数不为0且不包含持仓系统，持仓系统数不为零且小于最大持仓数 */

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

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_AF_EqualWeight_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AF_EQUALWEIGHT.xml";

    AFPtr af1 = AF_EqualWeight();
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(af1);
    }

    AFPtr af2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(af2);
    }

    CHECK_EQ(af1->name(), af2->name());
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
