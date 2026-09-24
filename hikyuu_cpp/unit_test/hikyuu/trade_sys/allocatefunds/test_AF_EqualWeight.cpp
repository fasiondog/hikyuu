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
#include <hikyuu/trade_sys/allocatefunds/AllocateFundsBase.h>
#include <memory>
#include <unordered_set>

using namespace hku;

// Pass the se_list weights through, avoiding AF_EqualWeight flattening them to 1.0 and making the
// sorting / assertions uncontrollable
class AF_PassThroughWeight : public AllocateFundsBase {
public:
    AF_PassThroughWeight() : AllocateFundsBase("AF_PassThroughWeight") {}
    AFPtr _clone() override {
        return std::make_shared<AF_PassThroughWeight>();
    }
    SystemWeightList _allocateWeight(const Datetime&, const SystemWeightList& se_list) override {
        return se_list;
    }
};

/**
 * @defgroup test_AllocateFunds test_AllocateFunds
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test point, no automatic position adjustment */
TEST_CASE("test_AF_EqualWeight_not_adjust_hold") {
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

    /** @arg The selected system count equals the max holding count and the held count is 0 */
    /** @arg The selected count is greater than the max holding count and the held count is 0 */

    /** @arg The selected count is not 0 and the held count equals the max holding count */
    /** @arg The selected count is not 0 and the held count is greater than the max holding count */
    /** @arg The selected count is not 0 and contains no held system; the held count is not 0 and
     * less than the max */

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

/** @par With adjust_running_sys=false the occupied weight of a running system is computed from the
 * subsystem assets */
TEST_CASE("test_AF_without_running_uses_sub_tm_funds") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sz000001"];
    CHECK_UNARY(!stock.isNull());

    // The adjustment day is later than the buy day, triggering the position revaluation of getFunds
    Datetime buy_date(201101040000L);
    Datetime date(201101050000L);

    // The total account book value is 100000 and the shadow cash account can allocate 100000
    // A simplified PF book: m_tm is the total asset reference, m_cash_tm the allocatable pool and
    // sub_tm the subsystem cash_tm is filled so that the cash limit does not hide the truncation
    // assertion of scenario 2
    TMPtr tm = crtTM(Datetime(200101010000L), 100000);
    TMPtr cash_tm = crtTM(Datetime(200101010000L), 100000);

    // running: cash + position, so total_assets goes through cash+market_value, not the cash
    // shortcut
    TMPtr sub_tm_running = crtTM(Datetime(200101010000L), 30000);
    price_t buy_price = 10.0;
    double buy_num = 1000;
    auto tr = sub_tm_running->buy(buy_date, stock, buy_price, buy_num, 0.0, buy_price, buy_price);
    CHECK_UNARY(!tr.isNull());

    FundsRecord running_funds = sub_tm_running->getFunds(date, KQuery::DAY);
    CHECK_GT(running_funds.market_value, 0.0);
    price_t running_assets = running_funds.total_assets();
    CHECK_GT(running_assets, 0.0);
    CHECK_LT(running_assets, 100000.0);  // The occupied weight < 1, or the case verifies nothing
    CHECK_LT(running_assets, 80000.0);  // Premise of the untruncated 0.2 of scenario 1: w_run < 0.8

    TMPtr sub_tm_new = crtTM(Datetime(200101010000L), 0);
    SYSPtr sys_running = SYS_Simple(sub_tm_running);
    SYSPtr sys_new = SYS_Simple(sub_tm_new);

    // A pass-through weight AF is used so the 0.8/0.2 weights of se_list are not flattened
    AFPtr af = std::make_shared<AF_PassThroughWeight>();
    af->setParam<bool>("adjust_running_sys", false);
    af->setParam<bool>("auto_adjust_weight", false);  // Given weights, running is traversed first
    af->setParam<double>("reserve_percent", 0.0);
    af->setTM(tm);
    af->setCashTM(cash_tm);
    af->setQuery(KQueryByDate(Datetime(201101010000L), Datetime(201201010000L)));

    // -----------------------------------------------------------------
    // Scenario 1: running has a higher weight and is handled first; before the fix sum_weight~1
    // left
    // -----------------------------------------------------------------
    {
        SystemWeightList se_list;
        se_list.emplace_back(sys_running, 0.8);
        se_list.emplace_back(sys_new, 0.2);

        std::unordered_set<SYSPtr> running_set;
        running_set.insert(sys_running);

        price_t cash_before = cash_tm->currentCash();
        price_t running_cash_before = sub_tm_running->currentCash();

        af->adjustFunds(date, se_list, running_set);

        // The new system gets the planned weight 0.2: will_cash = total_funds * 0.2 = 20000
        // Running occupies running_assets/100000 < 0.8, so 0.2 is not truncated
        CHECK_EQ(sys_new->getTM()->currentCash(), doctest::Approx(20000.0));
        // without_running does not adjust the running position
        CHECK_EQ(sub_tm_running->currentCash(), doctest::Approx(running_cash_before));
        CHECK_EQ(cash_tm->currentCash(), doctest::Approx(cash_before - 20000.0));
    }

    // -----------------------------------------------------------------
    // Scenario 2: the remaining weight truncation - the new planned weight is truncated by
    // can_allocate_weight-sum_weight Reuse the same running after resetting the new system
    // -----------------------------------------------------------------
    {
        // Recall the funds given to the new system in scenario 1
        price_t new_cash = sub_tm_new->currentCash();
        if (new_cash > 0.0) {
            CHECK_UNARY(sub_tm_new->checkout(date, new_cash));
            CHECK_UNARY(cash_tm->checkin(date, new_cash));
        }
        CHECK_EQ(sub_tm_new->currentCash(), doctest::Approx(0.0));

        // The planned weights are running 0.9 (first) and new 0.8; new gets 1-w_run after the
        // occupation
        SystemWeightList se_list;
        se_list.emplace_back(sys_running, 0.9);
        se_list.emplace_back(sys_new, 0.8);

        std::unordered_set<SYSPtr> running_set;
        running_set.insert(sys_running);

        price_t total_funds = tm->getFunds(date, KQuery::DAY).total_assets();
        price_t w_run = running_assets / total_funds;
        // The same truncation formula as _adjust_without_running
        price_t plan_w = 0.8;
        price_t current_weight = (plan_w + w_run > 1.0) ? (1.0 - w_run) : plan_w;
        // The implementation applies roundUp(..., precision) to will_cash with the default
        // precision=2
        price_t expect_new_cash = total_funds * current_weight;
        price_t cash_before = cash_tm->currentCash();
        if (expect_new_cash > cash_before) {
            expect_new_cash = cash_before;
        }
        // The scenario intent: a truncation must happen, or the sum_weight squeeze is not measured
        CHECK_GT(plan_w + w_run, 1.0);

        af->adjustFunds(date, se_list, running_set);

        // The scale / epsilon covers roundUp(precision=2) and the floating point weight error
        CHECK_EQ(sys_new->getTM()->currentCash(),
                 doctest::Approx(expect_new_cash).scale(1).epsilon(0.01));
        CHECK_EQ(cash_tm->currentCash(),
                 doctest::Approx(cash_before - expect_new_cash).scale(1).epsilon(0.01));
        // Clearly below the funds of the planned weight 0.8, proving a truncation, not a full
        // allocation
        CHECK_LT(sys_new->getTM()->currentCash(), total_funds * plan_w - 1.0);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
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
