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

// 透传 se_list 权重，避免 AF_EqualWeight 把权重改成全 1.0 导致排序/断言不可控
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

/** @par 检测点，不自动调仓 */
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

/** @par adjust_running_sys=false 时，running 系统占用权重应按子系统资产计算 */
TEST_CASE("test_AF_without_running_uses_sub_tm_funds") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sz000001"];
    CHECK_UNARY(!stock.isNull());

    // 调仓日：晚于买入日，触发 getFunds(date) 持仓市值重估路径
    Datetime buy_date(201101040000L);
    Datetime date(201101050000L);

    // 总账户账面 100000；影子资金账户可分配 100000
    // （简化 PF 账本：m_tm 作总资产参考，m_cash_tm 为可分配池，sub_tm 为子系统）
    // cash_tm 给足，避免场景2被现金上限掩盖权重截断断言
    TMPtr tm = crtTM(Datetime(200101010000L), 100000);
    TMPtr cash_tm = crtTM(Datetime(200101010000L), 100000);

    // running：现金 + 持仓，确保 total_assets 走 cash+market_value，而非纯现金捷径
    TMPtr sub_tm_running = crtTM(Datetime(200101010000L), 30000);
    price_t buy_price = 10.0;
    double buy_num = 1000;
    auto tr = sub_tm_running->buy(buy_date, stock, buy_price, buy_num, 0.0, buy_price, buy_price);
    CHECK_UNARY(!tr.isNull());

    FundsRecord running_funds = sub_tm_running->getFunds(date, KQuery::DAY);
    CHECK_GT(running_funds.market_value, 0.0);
    price_t running_assets = running_funds.total_assets();
    CHECK_GT(running_assets, 0.0);
    CHECK_LT(running_assets, 100000.0);  // 占用权重 < 1，否则本用例无法验证新系统分配
    CHECK_LT(running_assets, 80000.0);   // 场景1 计划权重 0.2 不被截断的前提：w_run < 0.8

    TMPtr sub_tm_new = crtTM(Datetime(200101010000L), 0);
    SYSPtr sys_running = SYS_Simple(sub_tm_running);
    SYSPtr sys_new = SYS_Simple(sub_tm_new);

    // 使用透传权重 AF，确保 se_list 中 0.8/0.2 等指定权重不被 EqualWeight 抹平
    AFPtr af = std::make_shared<AF_PassThroughWeight>();
    af->setParam<bool>("adjust_running_sys", false);
    af->setParam<bool>("auto_adjust_weight", false);  // 指定权重，保证 running 先被遍历
    af->setParam<double>("reserve_percent", 0.0);
    af->setTM(tm);
    af->setCashTM(cash_tm);
    af->setQuery(KQueryByDate(Datetime(201101010000L), Datetime(201201010000L)));

    // -----------------------------------------------------------------
    // 场景 1：running 权重更高，排序后先处理；修复前 sum_weight≈1 导致新系统分到 0
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

        // 新系统按计划权重 0.2 分配：will_cash = total_funds * 0.2 = 20000
        // （running 实际占用 running_assets/100000 < 0.8，不会截断 0.2）
        CHECK_EQ(sys_new->getTM()->currentCash(), doctest::Approx(20000.0));
        // without_running 不调 running 仓位/现金
        CHECK_EQ(sub_tm_running->currentCash(), doctest::Approx(running_cash_before));
        CHECK_EQ(cash_tm->currentCash(), doctest::Approx(cash_before - 20000.0));
    }

    // -----------------------------------------------------------------
    // 场景 2：剩余权重截断——running 实际占用后，新系统计划权重被 can_allocate_weight-sum_weight 截断
    // 重置新系统与 cash_tm 后复用同一 running
    // -----------------------------------------------------------------
    {
        // 回收场景1 分给新系统的资金
        price_t new_cash = sub_tm_new->currentCash();
        if (new_cash > 0.0) {
            CHECK_UNARY(sub_tm_new->checkout(date, new_cash));
            CHECK_UNARY(cash_tm->checkin(date, new_cash));
        }
        CHECK_EQ(sub_tm_new->currentCash(), doctest::Approx(0.0));

        // 计划权重：running 0.9（先遍历），new 0.8；running 实际占用 w_run 后，new 只能拿到 1-w_run
        SystemWeightList se_list;
        se_list.emplace_back(sys_running, 0.9);
        se_list.emplace_back(sys_new, 0.8);

        std::unordered_set<SYSPtr> running_set;
        running_set.insert(sys_running);

        price_t total_funds = tm->getFunds(date, KQuery::DAY).total_assets();
        price_t w_run = running_assets / total_funds;
        // 与 _adjust_without_running 相同的截断公式
        price_t plan_w = 0.8;
        price_t current_weight =
          (plan_w + w_run > 1.0) ? (1.0 - w_run) : plan_w;
        // 实现里对 will_cash 做 roundUp(..., precision)，默认 precision=2
        price_t expect_new_cash = total_funds * current_weight;
        price_t cash_before = cash_tm->currentCash();
        if (expect_new_cash > cash_before) {
            expect_new_cash = cash_before;
        }
        // 场景意图：必须发生截断，否则测不到 sum_weight 挤占
        CHECK_GT(plan_w + w_run, 1.0);

        af->adjustFunds(date, se_list, running_set);

        // scale/epsilon 覆盖 roundUp(precision=2) 与浮点权重误差
        CHECK_EQ(sys_new->getTM()->currentCash(),
                 doctest::Approx(expect_new_cash).scale(1).epsilon(0.01));
        CHECK_EQ(cash_tm->currentCash(),
                 doctest::Approx(cash_before - expect_new_cash).scale(1).epsilon(0.01));
        // 明确低于计划权重 0.8 对应资金，证明发生了截断而非按 plan 全额分配
        CHECK_LT(sys_new->getTM()->currentCash(), total_funds * plan_w - 1.0);
    }
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
