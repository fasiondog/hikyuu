/*
 * Portfolio.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include "hikyuu/global/sysinfo.h"
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

Portfolio::Portfolio() : m_name("Portfolio"), m_query(Null<KQuery>()), m_need_calculate(true) {
    initParam();
}

Portfolio::Portfolio(const string& name)
: m_name(name), m_query(Null<KQuery>()), m_need_calculate(true) {
    initParam();
}

Portfolio::Portfolio(const TradeManagerPtr& tm, const SelectorPtr& se, const AFPtr& af)
: m_name("Portfolio"),
  m_tm(tm),
  m_se(se),
  m_af(af),
  m_query(Null<KQuery>()),
  m_need_calculate(true) {
    initParam();
}

Portfolio::~Portfolio() {}

void Portfolio::initParam() {
    setParam<int>("adjust_cycle", 1);    // 调仓周期
    setParam<bool>("trace", false);      // 打印跟踪
    setParam<int>("trace_max_num", 10);  // 打印跟踪时，显示当前持仓证券最大数量
}

void Portfolio::baseCheckParam(const string& name) const {
    if ("adjust_cycle" == name) {
        int adjust_cycle = getParam<int>("adjust_cycle");
        HKU_ASSERT(adjust_cycle >= 1);
    } else if ("trace" == name) {
        if (getParam<bool>("trace") && pythonInJupyter()) {
            HKU_THROW("You can't trace in jupyter!");
        }
    }
}

void Portfolio::paramChanged() {
    m_need_calculate = true;
}

void Portfolio::reset() {
    m_real_sys_list.clear();
    m_running_sys_set.clear();
    m_dlist_sys_list.clear();
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
    m_need_calculate = true;
}

PortfolioPtr Portfolio::clone() {
    PortfolioPtr p = make_shared<Portfolio>();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_query = m_query;
    p->m_real_sys_list = m_real_sys_list;
    p->m_need_calculate = true;
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

void Portfolio::_readyForRun() {
    SPEND_TIME(Portfolio_readyForRun);
    HKU_CHECK(m_se, "m_se is null!");
    HKU_CHECK(m_tm, "m_tm is null!");
    HKU_CHECK(m_af, "m_am is null!");

    // se算法和af算法不匹配
    HKU_CHECK(m_se->isMatchAF(m_af), "The current SE and AF do not match!");

    // 检查账户是否存在初始资产
    FundsRecord funds = m_tm->getFunds();
    HKU_CHECK(funds.total_assets() > 0.0, "The current tm is zero assets!");

    // 从 se 获取原型系统列表
    auto pro_sys_list = m_se->getProtoSystemList();
    HKU_CHECK(!pro_sys_list.empty(), "Can't fetch proto_sys_lsit from Selector!");

    reset();

    // 生成资金账户
    m_cash_tm = m_tm->clone();

    // 配置资产分配器
    m_af->setTM(m_tm);
    m_af->setCashTM(m_cash_tm);
    m_af->setQuery(m_query);

    // 获取所有备选子系统，为无关联账户的子系统分配子账号，对所有子系统做好启动准备
    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "TM_SUB");
    size_t total = pro_sys_list.size();
    m_real_sys_list.reserve(total);
    for (size_t i = 0; i < total; i++) {
        SystemPtr& pro_sys = pro_sys_list[i];
        SystemPtr sys = pro_sys->clone();
        m_real_sys_list.emplace_back(sys);

        // 为内部实际执行的系统创建初始资金为0的子账户
        sys->setTM(pro_tm->clone());
        string sys_name = fmt::format("{}_{}", sys->name(), sys->getStock().market_code());
        sys->getTM()->name(fmt::format("TM_SUB_{}", sys_name));
        sys->name(fmt::format("PF_{}", sys_name));

        sys->readyForRun();
        KData k = sys->getStock().getKData(m_query);
        sys->setTO(k);
    }

    // 告知 se 当前实际运行的系统列表
    m_se->calculate(m_real_sys_list, m_query);
}

void Portfolio::run(const KQuery& query, int adjust_cycle, bool force) {
    SPEND_TIME(Portfolio_run);
    setParam<int>("adjust_cycle", adjust_cycle);
    setQuery(query);

    if (force) {
        m_need_calculate = true;
    }
    HKU_IF_RETURN(!m_need_calculate, void());

    _readyForRun();

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    HKU_IF_RETURN(datelist.empty(), void());

    size_t cur_adjust_ix = 0;
    Datetime cur_cycle_end;
    for (size_t i = 0, total = datelist.size(); i < total; i++) {
        bool adjust = false;
        if (i == cur_adjust_ix) {
            adjust = true;
            cur_adjust_ix += adjust_cycle;
            cur_cycle_end =
              cur_adjust_ix < total ? datelist[cur_adjust_ix] : datelist.back() + Seconds(1);
        }

        const auto& date = datelist[i];
        _runMoment(date, cur_cycle_end, adjust);
    }

    m_need_calculate = false;

    // 释放掉临时数据占用的内存
    m_tmp_selected_list = SystemWeightList();
    m_tmp_will_remove_sys = SystemWeightList();
}

void Portfolio::_runMoment(const Datetime& date, const Datetime& nextCycle, bool adjust) {
    // 当前日期小于账户建立日期，直接忽略
    HKU_IF_RETURN(date < m_cash_tm->initDatetime(), void());

    bool trace = getParam<bool>("trace");
    HKU_INFO_IF(trace, "{} ===========================================================", date);
    if (trace && adjust) {
        HKU_INFO("****************************************************");
        HKU_INFO("**                                                **");
        HKU_INFO("**  [PF] Position adjustment will be made today.  **");
        HKU_INFO("**                                                **");
        HKU_INFO("****************************************************");
    }
    HKU_INFO_IF(trace, "[PF] current running system size: {}", m_running_sys_set.size());

    //---------------------------------------------------
    // 检测运行系统中是否存在已退市的证券
    //---------------------------------------------------
    for (auto iter = m_running_sys_set.begin(); iter != m_running_sys_set.end(); /*++iter*/) {
        auto& sys = *iter;
        if (sys->getStock().getMarketValue(date, m_query.kType()) == 0.0) {
            auto sub_tm = sys->getTM();
            auto sub_cash = sub_tm->currentCash();
            if (sub_cash > 0.0 && sub_tm->checkout(date, sub_cash)) {
                m_cash_tm->checkin(date, sub_cash);
            }
            m_dlist_sys_list.emplace_back(sys);
            m_running_sys_set.erase(iter++);
        } else {
            ++iter;
        }
    }

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
    // 开盘时，优先处理上一交易日遗留的延迟调仓卖出的系统
    //----------------------------------------------------------------------
    HKU_INFO_IF(trace, "[PF] process delay adjust sys, size: {}", m_delay_adjust_sys_list.size());
    SystemWeightList tmp_continue_adjust_sys_list;
    for (auto& sys : m_delay_adjust_sys_list) {
        auto tr = sys.sys->sellForceOnOpen(date, sys.weight, PART_PORTFOLIO);
        if (!tr.isNull()) {
            HKU_INFO_IF(trace, "[PF] Delay adjust sell: {}", tr);
            m_tm->addTradeRecord(tr);

            // 卖出后，尝试将资金取出转移至影子总账户
            TMPtr sub_tm = sys.sys->getTM();
            auto sub_cash = sub_tm->currentCash();
            if (sub_cash > 0.0 && sub_tm->checkout(date, sub_cash)) {
                m_cash_tm->checkin(date, sub_cash);
            }

        } else {
            // 强制卖出失败的情况下，如果当前仍有持仓，则需要下一交易日继续进行处理
            PositionRecord position = sys.sys->getTM()->getPosition(date, sys.sys->getStock());
            if (position.number > 0.0) {
                tmp_continue_adjust_sys_list.emplace_back(sys);
            }
        }
    }

    m_delay_adjust_sys_list.swap(tmp_continue_adjust_sys_list);

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
        tmp_continue_adjust_sys_list =
          m_af->adjustFunds(date, m_tmp_selected_list, m_running_sys_set);

        if (m_delay_adjust_sys_list.empty()) {
            m_delay_adjust_sys_list.swap(tmp_continue_adjust_sys_list);
        } else {
            for (auto& sw : tmp_continue_adjust_sys_list) {
                m_delay_adjust_sys_list.emplace_back(sw);
            }
        }

        // 如果选中的系统不在已有列表中，且账户已经被分配了资金，则将其加入运行系统列表
        for (auto& sys : m_tmp_selected_list) {
            if (m_running_sys_set.find(sys.sys) == m_running_sys_set.end()) {
                if (sys.sys->getTM()->cash(date, m_query.kType()) > 0.0) {
                    m_running_sys_set.insert(sys.sys);
                }
            }
        }

        // 从已运行系统列表中立即移除已没有持仓且没有资金的系统
        m_tmp_will_remove_sys.clear();
        for (auto& sys : m_running_sys_set) {
            auto sub_tm = sys->getTM();
            if (sub_tm->currentCash() < 1.0 && 0 == sub_tm->getHoldNumber(date, sys->getStock())) {
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
        HKU_TRACE_IF(trace, "run: {}", sub_sys->name());
        if (adjust) {
            auto sg = sub_sys->getSG();
            sg->startCycle(date, nextCycle);
        }

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
        // clang-format off
        HKU_INFO(
          "+------------+------------+------------+--------------+--------------+-------------+-------------+");
        HKU_INFO(
          "| code       | name       | position   | market value | remain cash  | open price  | close price  |");
        HKU_INFO(
          "+------------+------------+------------+--------------+--------------+-------------+-------------+");
        // clang-format on

        size_t count = 0;
        for (const auto& sys : m_running_sys_set) {
            Stock stk = sys->getStock();
            auto funds = sys->getTM()->getFunds(date, m_query.kType());
            size_t position = sys->getTM()->getHoldNumber(date, stk);
            KRecord krecord = stk.getKRecord(date, m_query.kType());
            auto stk_name = stk.name();
            HKU_INFO("| {:<11}| {:<11}| {:<11}| {:<13.2f}| {:<13.2f}| {:<12.2f}| {:<12.2f}|",
                     stk.market_code(), stk_name, position, funds.market_value, funds.cash,
                     krecord.openPrice, krecord.closePrice);
            // clang-format off
            HKU_INFO("+------------+------------+------------+--------------+--------------+-------------+-------------+");
            count++;
            int trace_max_num = getParam<int>("trace_max_num");
            if (count >= trace_max_num) {
                if (m_running_sys_set.size() > trace_max_num) {
                    HKU_INFO("+ ... ... more                                                                                   +");
                    HKU_INFO("+------------+------------+------------+--------------+--------------+-------------+-------------+");
                }
                break;
            }
            // clang-format on
        }
    }
}

} /* namespace hku */
