/*
 * Portfolio.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include "../../trade_manage/crt/crtTM.h"

#include "Portfolio.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const Portfolio& pf) {
    string strip(",\n");
    string space("  ");
    os << "Portfolio{\n"
       << space << pf.name() << strip << space << pf.getParameter() << strip << space
       << pf.getQuery() << strip << space << pf.getAF() << strip << space << pf.getSE() << strip
       << space << (pf.getTM() ? pf.getTM()->str() : "TradeManager(NULL)") << strip << "}";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const PortfolioPtr& pf) {
    if (pf) {
        os << *pf;
    } else {
        os << "Portfolio(NULL)";
    }

    return os;
}

Portfolio::Portfolio()
: m_name("Portfolio"), m_query(Null<KQuery>()), m_is_ready(false), m_need_calculate(true) {
    setParam<int>("adjust_cycle", 1);  // 调仓周期
    setParam<bool>("trace", false);    // 打印跟踪
}

Portfolio::Portfolio(const string& name)
: m_name(name), m_query(Null<KQuery>()), m_is_ready(false), m_need_calculate(true) {
    setParam<int>("adjust_cycle", 1);  // 调仓周期
    setParam<bool>("trace", false);
}

Portfolio::Portfolio(const TradeManagerPtr& tm, const SelectorPtr& se, const AFPtr& af)
: m_name("Portfolio"),
  m_tm(tm),
  m_se(se),
  m_af(af),
  m_query(Null<KQuery>()),
  m_is_ready(false),
  m_need_calculate(true) {
    setParam<int>("adjust_cycle", 1);  // 调仓周期
    setParam<bool>("trace", false);
}

Portfolio::~Portfolio() {}

void Portfolio::baseCheckParam(const string& name) const {
    if ("adjust_cycle" == name) {
        int adjust_cycle = getParam<int>("adjust_cycle");
        HKU_ASSERT(adjust_cycle >= 1);
    }
}

void Portfolio::paramChanged() {
    m_need_calculate = true;
}

void Portfolio::reset() {
    m_is_ready = false;
    m_pro_sys_list.clear();
    m_real_sys_list.clear();
    m_running_sys_set.clear();
    m_delay_adjust_sys_list.clear();
    m_tmp_selected_list.clear();
    m_tmp_will_remove_sys.clear();
    if (m_tm)
        m_tm->reset();
    if (m_cash_tm)
        m_cash_tm->reset();
    if (m_se)
        m_se->reset();
    if (m_af)
        m_af->reset();
}

PortfolioPtr Portfolio::clone() {
    PortfolioPtr p = make_shared<Portfolio>();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_query = m_query;
    p->m_pro_sys_list = m_pro_sys_list;
    p->m_real_sys_list = m_real_sys_list;
    p->m_is_ready = m_is_ready;
    p->m_need_calculate = m_need_calculate;
    if (m_se)
        p->m_se = m_se->clone();
    if (m_af)
        p->m_af = m_af->clone();
    if (m_tm)
        p->m_tm = m_tm->clone();
    if (m_cash_tm)
        p->m_cash_tm = m_cash_tm->clone();
    return p;
}

bool Portfolio::_readyForRun() {
    if (!m_se) {
        HKU_WARN("m_se is null!");
        m_is_ready = false;
        return false;
    }

    if (!m_tm) {
        HKU_WARN("m_tm is null!");
        m_is_ready = false;
        return false;
    }

    if (!m_af) {
        HKU_WARN("m_am is null!");
        m_is_ready = false;
        return false;
    }

    // se算法和af算法不匹配时，给出告警
    HKU_WARN_IF(!m_se->isMatchAF(m_af), "The current SE and AF do not match!");

    reset();

    // 将影子账户指定给资产分配器
    m_cash_tm = m_tm->clone();
    m_af->setTM(m_tm);
    m_af->setCashTM(m_cash_tm);

    // 为资金分配器设置关联查询条件
    m_af->setQuery(m_query);

    // 从 se 获取原型系统列表
    m_pro_sys_list = m_se->getProtoSystemList();

    // 获取所有备选子系统，为无关联账户的子系统分配子账号，对所有子系统做好启动准备
    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "TM_SUB");
    size_t total = m_pro_sys_list.size();
    for (size_t i = 0; i < total; i++) {
        SystemPtr& pro_sys = m_pro_sys_list[i];
        SystemPtr sys = pro_sys->clone();
        m_real_sys_list.emplace_back(sys);

        // 如果原型子系统没有关联账户，则为其创建一个和总资金金额相同的账户，以便能够运行
        if (!pro_sys->getTM()) {
            pro_sys->setTM(m_tm->clone());
        }

        // 为内部实际执行的系统创建初始资金为0的子账户
        sys->setTM(pro_tm->clone());
        sys->getTM()->name(fmt::format("TM_SUB{}", i));
        sys->name(fmt::format("PF_Real_{}_{}_{}", i, sys->name(), sys->getStock().market_code()));

        HKU_CHECK(sys->readyForRun() && pro_sys->readyForRun(),
                  "Exist invalid system, it could not ready for run!");
        KData k = sys->getStock().getKData(m_query);
        sys->setTO(k);
        pro_sys->setTO(k);
    }

    // 告知 se 当前实际运行的系统列表
    m_se->calculate(m_real_sys_list, m_query);

    m_is_ready = true;
    return true;
}

void Portfolio::run(const KQuery& query, int adjust_cycle, bool force) {
    HKU_CHECK(adjust_cycle > 0, "Invalid param adjust_cycle! {}", adjust_cycle);
    setParam<int>("adjust_cycle", adjust_cycle);

    setQuery(query);
    if (force) {
        m_need_calculate = true;
    }
    HKU_IF_RETURN(!m_need_calculate, void());
    HKU_CHECK(_readyForRun(),
              "readyForRun fails, check to see if a valid TradeManager, Selector, or "
              "AllocateFunds instance have been specified.");

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    HKU_IF_RETURN(datelist.empty(), void());

    size_t cur_adjust_ix = 0;
    for (size_t i = 0, total = datelist.size(); i < total; i++) {
        bool adjust = false;
        if (i == cur_adjust_ix) {
            adjust = true;
            cur_adjust_ix += adjust_cycle;
        }

        const auto& date = datelist[i];
        _runMoment(date, adjust);
    }

    m_need_calculate = false;

    // 释放掉临时数据占用的内存
    m_tmp_selected_list = SystemWeightList();
    m_tmp_will_remove_sys = SystemWeightList();
}

void Portfolio::_runMoment(const Datetime& date, bool adjust) {
    // 当前日期小于账户建立日期，直接忽略
    HKU_IF_RETURN(date < m_cash_tm->initDatetime(), void());

    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "{} ===========================================================", date);
    HKU_INFO_IF(trace && adjust, "[PF] Position adjustment will be made today.");
    HKU_INFO_IF(trace, "[PF] current running system size: {}", m_running_sys_set.size());

    //---------------------------------------------------
    // 开盘前处理各个子账户、资金账户、总账户之间可能的误差
    //---------------------------------------------------
    int precision = m_tm->getParam<int>("precision");

    // 更新所有运行中系统的权息
    price_t sum_cash = 0.0;
    for (auto& running_sys : m_running_sys_set) {
        TMPtr sub_tm = running_sys->getTM();
        sub_tm->updateWithWeight(date);
        sum_cash += sub_tm->currentCash();
    }

    // 开盘前，调整账户权息，并进行轧差处理（平衡 sub_sys, cash_tm, tm 之间的误差）
    m_tm->updateWithWeight(date);

    HKU_INFO_IF(trace, "[PF] The sum cash of sub_tm: {}, cash tm: {}, tm cash: {}", sum_cash,
                m_cash_tm->currentCash(), m_tm->currentCash());
    sum_cash += m_cash_tm->currentCash();

    price_t diff = roundEx(std::abs(m_tm->currentCash() - sum_cash), precision);
    if (diff > 0.) {
        if (m_tm->currentCash() > sum_cash) {
            m_cash_tm->checkin(date, diff);
        } else if (m_tm->currentCash() < sum_cash) {
            if (!m_cash_tm->checkout(date, diff)) {
                m_tm->checkin(date, diff);
            }
        }
        HKU_INFO_IF(trace, "After compensate: the sum cash of sub_tm: {}, cash tm: {}, tm cash: {}",
                    sum_cash, m_cash_tm->currentCash(), m_tm->currentCash());
    }

    //----------------------------------------------------------------------
    // 跟踪打印执行调仓前的资产情况
    //----------------------------------------------------------------------
    if (trace) {
        auto funds = m_tm->getFunds(date, m_query.kType());
        HKU_INFO("[PF] [beforce adjust] - total funds: {},  cash: {}, market_value: {}",
                 funds.cash + funds.market_value, funds.cash, funds.market_value);
    }

    //----------------------------------------------------------------------
    // 开盘时，优先处理上一交易日确定的需延迟调仓卖出的系统，在开盘时先卖出调整
    //----------------------------------------------------------------------
    for (auto& sys : m_delay_adjust_sys_list) {
        auto tr = sys.sys->sellForceOnOpen(date, sys.weight, PART_PORTFOLIO);
        HKU_DEBUG_IF(trace && tr.isNull(), "[PF] Failed to force sell: {}", sys.sys->name());
        if (!tr.isNull()) {
            HKU_INFO_IF(trace, "[PF] Delay adjust sell: {}", tr);
            m_tm->addTradeRecord(tr);

            // 卖出后，尝试将资金取出转移至影子总账户
            TMPtr sub_tm = sys.sys->getTM();
            auto sub_cash = sub_tm->currentCash();
            if (sub_cash > 0.0 && sub_tm->checkout(date, sub_cash)) {
                m_cash_tm->checkin(date, sub_cash);
            }
        }
    }

    // 清空，避免循环至下一个非调仓日时被重复处理
    m_delay_adjust_sys_list.clear();

    //---------------------------------------------------------------
    // 遍历当前运行中的子系统，如果已没有分配资金和持仓，则回收
    //---------------------------------------------------------------
    m_tmp_will_remove_sys.clear();
    for (auto& running_sys : m_running_sys_set) {
        Stock stock = running_sys->getStock();
        TMPtr sub_tm = running_sys->getTM();
        PositionRecord position = sub_tm->getPosition(date, stock);
        price_t cash = sub_tm->currentCash();

        price_t min_cash = 1.0;
        KRecord krecord = stock.getKRecord(date);
        if (krecord.isValid()) {
            min_cash = krecord.openPrice * stock.minTradeNumber();
        }

        // 如果系统的剩余资金小于交易一手的资金，则回收资金
        if (cash != 0 && cash <= min_cash) {
            sub_tm->checkout(date, cash);
            m_cash_tm->checkin(date, cash);
            HKU_INFO_IF(trace, "Recycle cash ({:<.2f}) from {}, current cash: {}", cash,
                        running_sys->name(), m_cash_tm->currentCash());
            // 如果已经没有持仓，则回收
            if (position.number == 0) {
                m_tmp_will_remove_sys.emplace_back(running_sys, 0.);
                HKU_INFO_IF(trace, "[PF] Recycle running sys: {}", running_sys->name());
            }
        }
    }

    // 依据待移除列表将系统从运行中系统列表里删除
    for (auto& sub_sys : m_tmp_will_remove_sys) {
        HKU_INFO_IF(trace, "Recycling system {}", sub_sys.sys->name());
        m_running_sys_set.erase(sub_sys.sys);
    }

    //---------------------------------------------------
    // 调仓日，进行资金分配调整
    //---------------------------------------------------
    if (adjust) {
        // 从选股策略获取选中的系统列表
        m_tmp_selected_list = m_se->getSelected(date);

        if (trace && !m_tmp_selected_list.empty()) {
            for (auto& sys : m_tmp_selected_list) {
                HKU_INFO("[PF] select: {}, score: {:<.4f}", sys.sys->name(), sys.weight);
            }
        }

        // 资产分配算法调整各子系统资产分配
        m_delay_adjust_sys_list = m_af->adjustFunds(date, m_tmp_selected_list, m_running_sys_set);

        // 如果选中的系统不在已有列表中，且账户已经被分配了资金，则将其加入运行系统，并执行
        for (auto& sys : m_tmp_selected_list) {
            if (m_running_sys_set.find(sys.sys) == m_running_sys_set.end()) {
                if (sys.sys->getTM()->cash(date, m_query.kType()) > 0.0) {
                    m_running_sys_set.insert(sys.sys);
                }
            }
        }

        // 从已运行系统列表中立即移除已没有持仓且没有资金的非延迟买入的系统
        m_tmp_will_remove_sys.clear();
        for (auto& sys : m_running_sys_set) {
            auto sub_tm = sys->getTM();
            if (!sys->getParam<bool>("buy_delay") && sub_tm->currentCash() < 1.0 &&
                0 == sub_tm->getHoldNumber(date, sys->getStock())) {
                m_tmp_will_remove_sys.emplace_back(sys, 0.0);
            }
        }

        for (auto& sw : m_tmp_will_remove_sys) {
            m_running_sys_set.erase(sw.sys);
        }
    }

    //----------------------------------------------------------------------
    // 跟踪打印执行调仓后的资产情况
    //----------------------------------------------------------------------
    if (trace) {
        auto funds = m_tm->getFunds(date, m_query.kType());
        HKU_INFO("[PF] [after adjust] - total funds: {},  cash: {}, market_value: {}",
                 funds.cash + funds.market_value, funds.cash, funds.market_value);
    }

    //----------------------------------------------------------------------------
    // 执行所有运行中的系统，无论是延迟还是非延迟，当天运行中的系统都需要被执行一次
    //----------------------------------------------------------------------------
    for (auto& sub_sys : m_running_sys_set) {
        auto tr = sub_sys->runMoment(date);
        if (!tr.isNull()) {
            HKU_INFO_IF(trace, "[PF] {}", tr);
            m_tm->addTradeRecord(tr);
        }
    }

    //----------------------------------------------------------------------
    // 跟踪各个子系统执行后的资产情况
    //----------------------------------------------------------------------
    if (trace) {
        auto funds = m_tm->getFunds(date, m_query.kType());
        HKU_INFO("[PF] [after run] - total funds: {},  cash: {}, market_value: {}",
                 funds.cash + funds.market_value, funds.cash, funds.market_value);
    }

    //----------------------------------------------------------------------
    // 跟踪打印持仓情况
    //----------------------------------------------------------------------
    if (trace && !m_running_sys_set.empty()) {
        HKU_INFO(
          "+------------+------------+------------+--------------+--------------+-------------+");
        HKU_INFO(
          "| code       | name       | position   | market value | remain cash  | close price |");
        HKU_INFO(
          "+------------+------------+------------+--------------+--------------+-------------+");

        size_t count = 0;
        for (const auto& sys : m_running_sys_set) {
            Stock stk = sys->getStock();
            auto funds = sys->getTM()->getFunds(date, m_query.kType());
            size_t position = sys->getTM()->getHoldNumber(date, stk);
#if HKU_OS_WINDOWS
            auto stk_name = StockManager::instance().runningInPython() &&
                                StockManager::instance().pythonInJupyter()
                              ? stk.name()
                              : UTF8ToGB(stk.name());
            if (stk_name.size() < 11) {
                for (size_t i = 0, total = 11 - stk_name.size(); i < total; i++) {
                    stk_name.push_back(' ');
                }
            }
            HKU_INFO("| {:<11}| {}| {:<11}| {:<13.2f}| {:<13.2f}| {:<12.2f}|", stk.market_code(),
                     stk_name, position, funds.market_value, funds.cash,
                     stk.getKRecord(date, m_query.kType()).closePrice);
#else
            auto stk_name = stk.name();
            HKU_INFO("| {:<11}| {:<11}| {:<11}| {:<13.2f}| {:<13.2f}| {:<12.2f}|",
                     stk.market_code(), stk_name, position, funds.market_value, funds.cash,
                     stk.getKRecord(date, m_query.kType()).closePrice);
#endif
            HKU_INFO(
              "+------------+------------+------------+--------------+--------------+-------------"
              "+");
            if (++count >= 10) {
                break;
            }
        }
    }
}

} /* namespace hku */
