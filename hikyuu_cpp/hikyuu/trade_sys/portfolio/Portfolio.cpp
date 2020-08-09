/*
 * Portfolio.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include <boost/bind.hpp>
#include "../../trade_manage/crt/crtTM.h"

#include "Portfolio.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const Portfolio& pf) {
    os << "Portfolio(" << pf.name() << ", " << pf.getParameter() << ")";
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

Portfolio::Portfolio() : m_name("Portfolio"), m_is_ready(false) {}

Portfolio::Portfolio(const string& name) : m_name(name), m_is_ready(false) {}

Portfolio::Portfolio(const TradeManagerPtr& tm, const SelectorPtr& se, const AFPtr& af)
: m_name("Portfolio"), m_tm(tm), m_se(se), m_af(af), m_is_ready(false) {}

Portfolio::~Portfolio() {}

void Portfolio::reset() {
    m_is_ready = false;
    m_running_sys_set.clear();
    m_running_sys_list.clear();
    m_all_sys_set.clear();
    if (m_tm)
        m_tm->reset();
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
    p->m_running_sys_set = m_running_sys_set;
    p->m_running_sys_list = m_running_sys_list;
    p->m_all_sys_set = m_all_sys_set;
    p->m_is_ready = m_is_ready;
    if (m_se)
        p->m_se = m_se->clone();
    if (m_af)
        p->m_af = m_af->clone();
    if (m_tm)
        p->m_tm = m_tm->clone();
    return p;
}

bool Portfolio::readyForRun() {
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

    reset();

    // 将影子账户指定给资产分配器
    // m_tm_shadow = m_tm->clone();
    // m_af->setTM(m_tm_shadow);
    m_af->setTM(m_tm);

    // 为资金分配器设置关联查询条件
    m_af->setQuery(m_query);

    // 获取所有备选子系统，为无关联账户的子系统分配子账号，对所有子系统做好启动准备
    SystemList all_sys_list = m_se->getAllSystemList();
    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "TM_SUB");
    auto sys_iter = all_sys_list.begin();
    for (; sys_iter != all_sys_list.end(); ++sys_iter) {
        SystemPtr& sys = *sys_iter;

        // 如果子系统没有关联账户，则为其分配一个子账户
        if (!sys->getTM()) {
            sys->setTM(pro_tm->clone());
        }

        if (sys->readyForRun()) {
            KData k = sys->getStock().getKData(m_query);
            sys->setTO(k);

            // 保存记录子系统
            m_running_sys_set.insert(sys);
            m_running_sys_list.push_back(sys);
            m_all_sys_set.insert(sys);

        } else {
            HKU_WARN("Exist invalid system, it could not ready for run!");
        }
    }

    m_is_ready = true;
    return true;
}

void Portfolio::runMoment(const Datetime& date) {
    HKU_CHECK(isReady(), "Not ready to run! Please perform readyForRun() first!");

    // 当前日期小于账户建立日期，直接忽略
    if (date < m_tm->initDatetime()) {
        return;
    }

    int precision = m_tm->getParam<int>("precision");
    SystemList cur_selected_list;        //当前选中系统列表
    std::set<SYSPtr> cur_selected_sets;  //当前选中系统集合，方便计算使用
    SystemList cur_allocated_list;       //当前分配了资金的系统
    SystemList cur_hold_sys_list;        //当前时刻有持仓的系统，每个时刻重新收集

    // 从选股策略获取当前选中的系统列表
    cur_selected_list = m_se->getSelectedSystemList(date);

    // 资产分配算法调整各子系统资产分配
    m_af->adjustFunds(date, cur_selected_list, m_running_sys_list);

    // 遍历当前运行中的子系统，如果已没有分配资金和持仓，则移除
    SystemList will_remove_sys;
    for (auto& running_sys : m_running_sys_list) {
        Stock stock = running_sys->getStock();
        TMPtr sub_tm = running_sys->getTM();
        KRecord krecord = running_sys->getTO().getKRecordByDate(date);
        PositionRecord position = sub_tm->getPosition(stock);
        running_sys->_sellForce(krecord, position.number, PART_PORTFOLIO);

        price_t cash = sub_tm->currentCash();
        if (cash > 0) {
            if (sub_tm->checkout(date, cash)) {
                m_tm->checkin(date, cash);
                // 重新获取此时的子账户资金
                cash = sub_tm->currentCash();
            }
        }

        position = sub_tm->getPosition(stock);

        // 已没有持仓且没有现金，则放入待移除列表
        if (position.number == 0 && cash <= std::abs(roundDown(0, precision))) {
            will_remove_sys.push_back(running_sys);
        }
    }

    for (auto& sub_sys : will_remove_sys) {
        m_running_sys_list.remove(sub_sys);
        m_running_sys_set.erase(sub_sys);
    }

    // 遍历本次选择的系统列表，如果存在分配资金且不在运行中列表内，则加入运行列表
    for (auto& sub_sys : cur_selected_list) {
        price_t cash = sub_sys->getTM()->currentCash();
        if (cash > 0 && m_running_sys_set.find(sub_sys) == m_running_sys_set.end()) {
            m_running_sys_list.push_back(sub_sys);
            m_running_sys_set.insert(sub_sys);

            if (m_all_sys_set.find(sub_sys) == m_all_sys_set.end()) {
                m_all_sys_set.insert(sub_sys);
            }
        }
    }

    // 执行所有运行中的系统
    for (auto& sub_sys : m_running_sys_list) {
        sub_sys->runMoment(date);
    }
}

void Portfolio::run(const KQuery& query) {
    HKU_CHECK(readyForRun(),
              "readyForRun fails, check to see if a valid TradeManager, Selector, or "
              "AllocateFunds instance have been specified.");

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    for (auto& date : datelist) {
        runMoment(date);
    }
}

} /* namespace hku */
