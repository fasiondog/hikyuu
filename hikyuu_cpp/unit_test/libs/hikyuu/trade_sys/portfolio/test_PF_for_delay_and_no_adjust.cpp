/*
 * test_SYS_Simple.cpp
 *
 *  Created on: 2013-4-20
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
#include <hikyuu/trade_sys/portfolio/crt/PF_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>

#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/crt/SG_CrossGold.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/indicator/crt/EMA.h>


using namespace hku;

/**
 * @defgroup test_Portfolio test_Portfolio
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 全部为delay模式系统实例，不调整持仓*/
BOOST_AUTO_TEST_CASE( test_PF_for_delay_and_no_adjust) {
    StockManager& sm = StockManager::instance();


    SYSPtr sys = SYS_Simple();
    sys->setSG(SG_CrossGold(OP(EMA(12)), OP(EMA(26))));
    sys->setMM(MM_FixedCount(100));
    SYSPtr pro_sys = sys->clone(false, false);

    TMPtr tm = crtTM(Datetime(199001010000L), 500000);
    SEPtr se = SE_Fixed();
    AFPtr af = AF_EqualWeight();
    PFPtr pf = PF_Simple(tm, se, af);

    KQuery query = KQueryByDate(Datetime(201101010000L), Null<Datetime>(),
                               KQuery::DAY);



    /** @arg */
    sys->setTM(tm->clone());
    sys->run(sm["sz000001"], query);
    sys->getTM()->tocsv(sm.tmpdir());

    se->addStock(sm["sz000001"], pro_sys);
    pf->run(query);
    tm->name("PF");
    tm->tocsv(sm.tmpdir());

    TradeRecordList tr1 = sys->getTM()->getTradeList();
    TradeRecordList tr2 = tm->getTradeList();
    BOOST_CHECK(tr1.size() == tr2.size());

    /*std::cout << tr1.size() << std::endl;
    for (auto iter = tr1.begin(); iter != tr1.end(); ++iter) {
        std::cout << *iter << std::endl;
    }

    std::cout << "==========================" << std::endl;
    std::cout << tr2.size() << std::endl;
    for (auto iter = tr2.begin(); iter != tr2.end(); ++iter) {
        std::cout << *iter << std::endl;
    }


    std::cout << sys->getTM()->currentCash() << std::endl;
    std::cout << tm->currentCash() << std::endl; */


}

/** @} */


