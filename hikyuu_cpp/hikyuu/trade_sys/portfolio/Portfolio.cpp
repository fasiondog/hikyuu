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
    setParam<bool>("trace", false);  // 打印跟踪
}

Portfolio::Portfolio(const string& name)
: m_name(name), m_query(Null<KQuery>()), m_is_ready(false), m_need_calculate(true) {
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
    setParam<bool>("trace", false);
}

Portfolio::~Portfolio() {}

void Portfolio::reset() {
    m_is_ready = false;
    m_pro_sys_list.clear();
    m_real_sys_list.clear();
    m_running_sys_set.clear();
    m_running_sys_list.clear();
    m_tmp_selected_list_on_open.clear();
    m_tmp_selected_list_on_close.clear();
    m_tmp_will_remove_sys.clear();
    if (m_tm)
        m_tm->reset();
    if (m_shadow_tm)
        m_shadow_tm->reset();
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
    if (m_shadow_tm)
        p->m_shadow_tm = m_shadow_tm->clone();
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
    m_shadow_tm = m_tm->clone();
    m_af->setTM(m_tm);
    m_af->setShadowTM(m_shadow_tm);

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
        sys->name(fmt::format("PF_Real_{}_{}", i, sys->name()));

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

void Portfolio::_runMoment(const Datetime& date) {
    // 当前日期小于账户建立日期，直接忽略
    HKU_IF_RETURN(date < m_shadow_tm->initDatetime(), void());

    _runMomentOnOpen(date);
    _runMomentOnClose(date);

    // 释放掉临时数据占用的内存
    m_running_sys_set = std::unordered_set<System*>();
    m_running_sys_list = std::list<SYSPtr>();
    m_tmp_selected_list_on_open = SystemList();
    m_tmp_selected_list_on_close = SystemList();
    m_tmp_will_remove_sys = SystemList();
}

void Portfolio::_runMomentOnOpen(const Datetime& date) {
    // 从选股策略获取当前选中的系统列表
    m_tmp_selected_list_on_open = m_se->getSelectedOnOpen(date);

    // 资产分配算法调整各子系统资产分配，忽略上一周期收盘时选中的系统
    m_af->adjustFunds(date, m_tmp_selected_list_on_open, m_running_sys_list,
                      m_tmp_selected_list_on_close);

    // 由于系统的交易指令可能被延迟执行，需要保存并判断
    // 遍历当前运行中的子系统，如果已没有分配资金和持仓，则放入待移除列表
    m_tmp_will_remove_sys.clear();
    int precision = m_shadow_tm->getParam<int>("precision");
    for (auto& running_sys : m_running_sys_list) {
        Stock stock = running_sys->getStock();
        TMPtr sub_tm = running_sys->getTM();
        PositionRecord position = sub_tm->getPosition(date, stock);
        price_t cash = sub_tm->cash(date, m_query.kType());

        // 已没有持仓且没有现金，则放入待移除列表
        if (position.number == 0 && cash <= precision) {
            if (cash != 0) {
                sub_tm->checkout(date, cash);
                m_shadow_tm->checkin(date, cash);
            }
            m_tmp_will_remove_sys.push_back(running_sys);
        }
    }

    // 依据待移除列表将系统从运行中系统列表里删除
    for (auto& sub_sys : m_tmp_will_remove_sys) {
        m_running_sys_list.remove(sub_sys);
        m_running_sys_set.erase(sub_sys.get());
    }

    // 遍历本次选择的系统列表，如果存在分配资金且不在运行中列表内，则加入运行列表
    for (auto& sub_sys : m_tmp_selected_list_on_open) {
        price_t cash = sub_sys->getTM()->cash(date, m_query.kType());
        if (cash > 0.0 && m_running_sys_set.find(sub_sys.get()) == m_running_sys_set.end()) {
            m_running_sys_list.push_back(sub_sys);
            m_running_sys_set.insert(sub_sys.get());
        }
    }

    // 在开盘时执行所有运行中的非延迟交易系统系统
    for (auto& sub_sys : m_running_sys_list) {
        if (!sub_sys->getParam<bool>("buy_delay")) {
            auto tr = sub_sys->runMoment(date);
            if (!tr.isNull()) {
                m_tm->addTradeRecord(tr);
            }
        }
    }
}

void Portfolio::_runMomentOnClose(const Datetime& date) {
    // 从选股策略获取当前选中的系统列表
    m_tmp_selected_list_on_close = m_se->getSelectedOnClose(date);

    // 资产分配算法调整各子系统资产分配，忽略开盘时选中的系统
    m_af->adjustFunds(date, m_tmp_selected_list_on_close, m_running_sys_list,
                      m_tmp_selected_list_on_open);
    if (getParam<bool>("trace") &&
        (!m_tmp_selected_list_on_open.empty() || !m_tmp_selected_list_on_close.empty())) {
        HKU_INFO("{} ===========================================================", date);
        for (auto& sys : m_tmp_selected_list_on_open) {
            HKU_INFO("select on open: {}, cash: {}", sys->getTO().getStock(),
                     sys->getTM()->cash(date, m_query.kType()));
        }
        for (auto& sys : m_tmp_selected_list_on_close) {
            HKU_INFO("select on close: {}, cash: {}", sys->getTO().getStock(),
                     sys->getTM()->cash(date, m_query.kType()));
        }
    }

    // 如果选中的系统不在已有列表中，且账户已经被分配了资金，则将其加入运行系统，并执行
    for (auto& sys : m_tmp_selected_list_on_close) {
        if (m_running_sys_set.find(sys.get()) == m_running_sys_set.end()) {
            TMPtr tm = sys->getTM();
            if (tm->cash(date, m_query.kType()) > 0.0) {
                m_running_sys_list.push_back(sys);
                m_running_sys_set.insert(sys.get());
            }
        }
    }

    // 执行所有非延迟运行中系统
    for (auto& sub_sys : m_running_sys_list) {
        if (sub_sys->getParam<bool>("buy_delay")) {
            auto tr = sub_sys->runMoment(date);
            if (!tr.isNull()) {
                m_tm->addTradeRecord(tr);
            }
        }
    }
}

void Portfolio::run(const KQuery& query, bool force) {
    setQuery(query);
    if (force) {
        m_need_calculate = true;
    }
    HKU_IF_RETURN(!m_need_calculate, void());
    HKU_ERROR_IF_RETURN(!_readyForRun(), void(),
                        "readyForRun fails, check to see if a valid TradeManager, Selector, or "
                        "AllocateFunds instance have been specified.");

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    for (const auto& date : datelist) {
        _runMoment(date);
    }
    m_need_calculate = false;
}

} /* namespace hku */
