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
    initParam();
}

Portfolio::Portfolio(const string& name) : m_name(name) {
    initParam();
}

Portfolio::Portfolio(const TradeManagerPtr& tm,
        const SystemPtr& sys,
        const SelectorPtr& se,
        const AFPtr& af)
: m_name("Portfolio"), m_se(se), m_tm(tm), m_af(af) {
    initParam();
}

Portfolio::~Portfolio() {

}

void Portfolio::initParam() {
    //以总账户为原型，自动设置所有系统实例的子账户，子账户初始资金为0
    //从各子账户重建总账户及其交易记录，未自行绑定子账户的系统实例以总账户为原型自动创建子账户
    //从各子账户重建总账户及其交易记录，忽略未自行绑定子账户的系统实例
    //不重建总账户直接使用各系统绑定的子账户，未绑定子账户的系统实例以总账户为原型自动创建子账户
    //不重建总账户直接使用各系统绑定的子账户，忽略未自行绑定子账户的系统实例
    setParam<int>("mode", Portfolio::ONLY_USE_TOTAL_TM);
}

void Portfolio::reset() {
    if (m_tm) m_tm->reset();
    if (m_se) m_se->reset();
    if (m_af) m_af->reset();
    m_sys_list.clear();
}

PortfolioPtr Portfolio::clone() {
    PortfolioPtr p = make_shared<Portfolio>();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_se = m_se;
    p->m_af = m_af;
    p->m_tm = m_tm;
    p->m_sys_list = m_sys_list;
    return p;
}

bool Portfolio::readyForRun() {
    string func_name(" [Portfolio::readyForRun]");
    if (!m_se) {
        HKU_WARN("m_se is null!" << func_name);
        return false;
    }

    if (!m_tm) {
        HKU_WARN("m_tm is null!" << func_name);
        return false;
    }

    if (!m_af) {
        HKU_WARN("m_am is null!" << func_name);
        return false;
    }

    int mode = getParam<int>("mode");
    if (mode < Portfolio::ONLY_USE_TOTAL_TM || mode >= Portfolio::INVLID_MODE) {
        HKU_ERROR("Error param \"mode\": " << mode << "!" << func_name);
        return false;
    }

    reset();

    m_af->setTM(m_tm);

    switch (mode) {
    case ONLY_USE_TOTAL_TM:
        rebuildOnlyTotalTM();
        break;

    case REBUILD_TM_AND_AUTO_CREATE_SUBTM:
        rebuildTMAndAutoCreateSubTM();
        break;

    case REBUILD_TM_AND_IGNORE_NO_SUBTM_SYS:
        rebuildTMAndIgnore();
        break;

    case ONLY_AUTO_CREATE_SUBTM:
        rebuildOnlyAutoCreateSubTM();
        break;

    case ONLY_IGNORE_NO_SUBTM_SYS:
        rebuildOnlyIngore();
        break;

    default:
        HKU_ERROR("Don't in this path!" << func_name);
        return false;
    }

    return true;
}

/*****************************************************************************
 * 以总账户为原型，自动设置所有系统实例的子账户，子账户初始资金为0
 ****************************************************************************/
void Portfolio::rebuildOnlyTotalTM() {
    assert(m_tm && m_se);

    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "SUB");

    SystemList all_sys_list = m_se->getAllSystemList();
    auto sysIter = all_sys_list.begin();
    for (; sysIter != all_sys_list.end(); ++sysIter) {
        (*sysIter)->setTM(pro_tm->clone());
        m_sys_list.push_back(*sysIter);
    }
}

/*****************************************************************************
 * 从各子账户重建总账户及其交易记录，未自行绑定子账户的系统实例以总账户为原型自动创建子账户
 ****************************************************************************/
void Portfolio::rebuildTMAndAutoCreateSubTM() {
    assert(m_tm && m_se);

    Datetime min_init_date = m_tm->initDatetime();
    Datetime max_last_date = m_tm->lastDatetime();
    price_t init_cash = 0.0;
    TradeRecordList all_tr_list;

    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "SUB");

    SystemList all_sys_list = m_se->getAllSystemList();
    auto sys_iter = all_sys_list.begin();
    for (; sys_iter != all_sys_list.end(); ++sys_iter) {
        TMPtr tm = (*sys_iter)->getTM();
        if (tm) {
            if (tm->initDatetime() < min_init_date) {
                min_init_date = tm->initDatetime();
            }
            if (tm->initDatetime() == min_init_date) {
                init_cash += tm->initCash();
            }
            if (tm->lastDatetime() > max_last_date) {
                max_last_date = tm->lastDatetime();
            }

            TradeRecordList tr_list = tm->getTradeList();
            all_tr_list.insert(all_tr_list.end(), tr_list.begin(), tr_list.end());

        } else {
            (*sys_iter)->setTM(pro_tm->clone());
        }

        m_sys_list.push_back(*sys_iter);
    }

    sort(all_tr_list.begin(), all_tr_list.end(),
         boost::bind(std::less<Datetime>(),
                     boost::bind(&TradeRecord::datetime, _1),
                     boost::bind(&TradeRecord::datetime, _2)));

    m_tm->addTradeRecord(TradeRecord(Null<Stock>(), min_init_date,
            BUSINESS_INIT, init_cash, init_cash, 0.0, 0,
            CostRecord(), 0.0,  init_cash, PART_INVALID));
    auto tr_iter = all_tr_list.begin();
    for (; tr_iter != all_tr_list.end(); ++tr_iter) {
        if (tr_iter->business == BUSINESS_INIT) {
            if (tr_iter->datetime != min_init_date) {
                TradeRecord new_tr(*tr_iter);
                new_tr.business = BUSINESS_CHECKIN;
                m_tm->addTradeRecord(new_tr);
            }
        } else {
            m_tm->addTradeRecord(*tr_iter);
        }
    }
}

/*****************************************************************************
 * 从各子账户重建总账户及其交易记录，忽略未自行绑定子账户的系统实例
 ****************************************************************************/
void Portfolio::rebuildTMAndIgnore() {
    assert(m_tm && m_se);

    Datetime min_init_date = m_tm->initDatetime();
    Datetime max_last_date = m_tm->lastDatetime();
    price_t init_cash = 0.0;
    TradeRecordList all_tr_list;

    SystemList all_sys_list = m_se->getAllSystemList();
    auto sys_iter = all_sys_list.begin();
    for (; sys_iter != all_sys_list.end(); ++sys_iter) {
        TMPtr tm = (*sys_iter)->getTM();
        if (tm) {
            //加入内部系统实例列表
            m_sys_list.push_back(*sys_iter);

            if (tm->initDatetime() < min_init_date) {
                min_init_date = tm->initDatetime();
            }
            if (tm->initDatetime() == min_init_date) {
                init_cash += tm->initCash();
            }
            if (tm->lastDatetime() > max_last_date) {
                max_last_date = tm->lastDatetime();
            }

            TradeRecordList tr_list = tm->getTradeList();
            all_tr_list.insert(all_tr_list.end(), tr_list.begin(), tr_list.end());
        }
    }

    sort(all_tr_list.begin(), all_tr_list.end(),
         boost::bind(std::less<Datetime>(),
                     boost::bind(&TradeRecord::datetime, _1),
                     boost::bind(&TradeRecord::datetime, _2)));

    m_tm->addTradeRecord(TradeRecord(Null<Stock>(), min_init_date,
            BUSINESS_INIT, init_cash, init_cash, 0.0, 0,
            CostRecord(), 0.0,  init_cash, PART_INVALID));
    auto tr_iter = all_tr_list.begin();
    for (; tr_iter != all_tr_list.end(); ++tr_iter) {
        if (tr_iter->business == BUSINESS_INIT) {
            if (tr_iter->datetime != min_init_date) {
                TradeRecord new_tr(*tr_iter);
                new_tr.business = BUSINESS_CHECKIN;
                m_tm->addTradeRecord(new_tr);
            }
        } else {
            m_tm->addTradeRecord(*tr_iter);
        }
    }
}

/*****************************************************************************
 * 不重建总账户直接使用各系统绑定的子账户，未绑定子账户的系统实例以总账户为原型自动创建子账户
 ****************************************************************************/
void Portfolio::rebuildOnlyAutoCreateSubTM() {
    assert(m_tm && m_se);

    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "SUB");

    SystemList all_sys_list = m_se->getAllSystemList();
    auto sysIter = all_sys_list.begin();
    for (; sysIter != all_sys_list.end(); ++sysIter) {
        if (!(*sysIter)->getTM()) {
            (*sysIter)->setTM(pro_tm->clone());
        }
        m_sys_list.push_back(*sysIter);
    }
}

/*****************************************************************************
 * 不重建总账户直接使用各系统绑定的子账户，忽略未自行绑定子账户的系统实例
 ****************************************************************************/
void Portfolio::rebuildOnlyIngore() {
    assert(m_tm && m_se);

    TMPtr pro_tm = crtTM(m_tm->initDatetime(), 0.0, m_tm->costFunc(), "SUB");

    SystemList all_sys_list = m_se->getAllSystemList();
    auto sysIter = all_sys_list.begin();
    for (; sysIter != all_sys_list.end(); ++sysIter) {
        if ((*sysIter)->getTM()) {
            m_sys_list.push_back(*sysIter);
        }
    }
}

void Portfolio::run(const KQuery& query) {
    if (!readyForRun()) {
        return;
    }

    auto sys_iter = m_sys_list.begin();
    for (; sys_iter != m_sys_list.end(); ++sys_iter) {
        SystemPtr sys = *sys_iter;
        if (sys->readyForRun()) {
            KData k = sys->getStock().getKData(query);
            sys->setTO(k);
        }
    }

    std::set<SYSPtr> cur_hold_sys_sets;

    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    DatetimeList::const_iterator date_iter = datelist.begin();
    for(; date_iter != datelist.end(); ++date_iter) {
        //忽略小于账户初始建立日期的交易日
        if (*date_iter < m_tm->initDatetime()) {
            continue;
        }

        //处理延迟操作请求
        sys_iter = m_sys_list.begin();
        for (; sys_iter != m_sys_list.end(); ++sys_iter) {
            (*sys_iter)->_processRequest(*date_iter);

            if ((*sys_iter)->getTM()->have((*sys_iter)->getStock())) {
                cur_hold_sys_sets.insert(*sys_iter);
            } else {
                cur_hold_sys_sets.erase(*sys_iter);
            }

            //同步交易记录
            TradeRecordList tr_list = (*sys_iter)->getTM()->getTradeList(*date_iter, Null<Datetime>());
            auto tr_iter = tr_list.begin();
            for (; tr_iter != tr_list.end(); ++tr_iter) {
                m_tm->addTradeRecord(*tr_iter);
            }
        }

        SystemList cur_hold_sys_list;
        auto hold_iter = cur_hold_sys_sets.begin();
        for (; hold_iter != cur_hold_sys_sets.end(); ++hold_iter) {
            cur_hold_sys_list.push_back(*hold_iter);
        }

        //计算当前时刻选择的系统实例
        SystemList selected_list = m_se->getSelectedSystemList(*date_iter);
        SystemList sw_list = m_af->getAllocateSystem(*date_iter, selected_list, cur_hold_sys_list);

        auto sw_iter = sw_list.begin();
        for (; sw_iter != sw_list.end(); ++sw_iter) {
            (*sw_iter)->runMoment(*date_iter);
        }

    }
}


void Portfolio::runOneMoment(Datetime date) {
    if (date < m_tm->initDatetime()) {
        return;
    }

}


} /* namespace hku */
