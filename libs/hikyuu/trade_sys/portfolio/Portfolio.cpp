/*
 * Portfolio.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/portfolio/Portfolio.h>

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const Portfolio& pf) {
    os << "Portfolio(" << pf.name() << ", " << pf.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const PortfolioPtr& pf) {
    if (pf) {
        os << *pf;
    } else {
        os << "Portfolio(NULL)";
    }

    return os;
}

Portfolio::Portfolio(): m_name("Portfolio") {

}

Portfolio::Portfolio(const string& name) : m_name(name) {

}

Portfolio::Portfolio(const TradeManagerPtr& tm,
        const SystemPtr& sys,
        const SelectorPtr& se)
: m_se(se), m_tm(tm), m_name("Portfolio") {

}

Portfolio::~Portfolio() {

}

void Portfolio::reset() {
    if (m_tm) m_tm->reset();
    if (m_se) m_se->reset();
    if (m_af) m_af->reset();
}

PortfolioPtr Portfolio::clone() {
    PortfolioPtr p = make_shared<Portfolio>();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_se = m_se;
    p->m_af = m_af;
    p->m_tm = m_tm;
    return p;
}


void Portfolio::run(const KQuery& query) {
    if (!m_se) {
        HKU_WARN("m_se is null [Portfolio::run]");
        return;
    }

    if (!m_tm) {
        HKU_WARN("m_tm is null [Portfolio::run]");
        return;
    }

    if (!m_af) {
        HKU_WARN("m_am is null [Portfolio::run]");
        return;
    }

    reset();

    m_af->setTM(m_tm);

    SystemList all_sys_list = m_se->getAllSystemList();
    auto sys_iter = all_sys_list.begin();
    for (; sys_iter != all_sys_list.end(); ++sys_iter) {
        SystemPtr sys = *sys_iter;
        sys->setTM(m_tm);
        if (sys->readyForRun()) {
            KData k = sys->getStock().getKData(query);
            sys->setTO(k);
        }
    }

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    DatetimeList::const_iterator date_iter = datelist.begin();
    for(; date_iter != datelist.end(); ++date_iter) {
        //忽略小于账户初始建立日期的交易日
        if (*date_iter < m_tm->initDatetime()) {
            continue;
        }

        //处理延迟操作请求
        sys_iter = all_sys_list.begin();
        for (; sys_iter != all_sys_list.end(); ++sys_iter) {
            (*sys_iter)->_processRequest(*date_iter);
        }

        //计算当前时刻选择的股票列表
        SystemList selected_list = m_se->getSelectedSystemList(*date_iter);
        //selected_list = m_am->getAllocateMoney(selected_list);
        auto sw_iter = selected_list.begin();
        for (; sw_iter != selected_list.end(); ++sw_iter) {
            (*sw_iter)->runMoment(*date_iter);
        }

        /*SystemList selected_sys_list = m_se->getSelectedSystemList(*date_iter);
        sys_iter = selected_sys_list.begin();
        for (; sys_iter != selected_sys_list.end(); ++sys_iter) {
            (*sys_iter)->runMoment(*date_iter);
        }*/
    }
}


} /* namespace hku */
