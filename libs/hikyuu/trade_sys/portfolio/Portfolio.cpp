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
    setParam<bool>("delay", true);
}

Portfolio::Portfolio(const string& name) : m_name(name) {
    setParam<bool>("delay", true);
}

Portfolio::Portfolio(const TradeManagerPtr& tm,
        const SystemPtr& sys,
        const SelectorPtr& st,
        const AllocateMoneyPtr& am)
: m_st(st), m_tm(tm), m_sys(sys), m_am(am), m_name("Portfolio") {
    if (m_sys) {
        setParam<bool>("delay", m_sys->getParam<bool>("delay"));
    } else {
        setParam<bool>("delay", true);
    }
}

Portfolio::~Portfolio() {
    // TODO Auto-generated destructor stub
}

void Portfolio::setSYS(const SystemPtr& sys) {
    if (sys) {
        m_sys = sys;
        setParam<bool>("delay", m_sys->getParam<bool>("delay"));
    } else {
        HKU_INFO("sys is null,will discard [Portfolio::setSYS]");
    }
}

void Portfolio::reset() {
    if (m_tm) m_tm->reset();
    if (m_st) m_st->reset();
    if (m_am) m_am->reset();
    if (m_sys) m_sys->reset();
}

PortfolioPtr Portfolio::clone() {
    Portfolio* p = new Portfolio();
    p->m_params = m_params;
    p->m_st = m_st;
    p->m_tm = m_tm;
    p->m_sys = m_sys;
    p->m_am = m_am;
    return PortfolioPtr(p);
}

void Portfolio::addStock(const Stock& stock) {
    if (m_st) {
        m_st->addStock(stock);
    } else {
        HKU_WARN("m_st is Null! [Portfolio::addStock]");
    }
}

void Portfolio::addStockList(const StockList& stock_list) {
    if (m_st) {
        m_st->addStockList(stock_list);
    } else {
        HKU_WARN("m_st is Null! [Portfolio::addStockList]");
    }
}


void Portfolio::run(const KQuery& query) {
    if (!m_st) {
        HKU_WARN("m_st is null [Portfolio::run]");
        return;
    }

    if (!m_tm) {
        HKU_WARN("m_tm is null [Portfolio::run]");
        return;
    }

    if (!m_sys) {
        HKU_WARN("m_sys is null [Portfolio::run]");
        return;
    }

    if (!m_am) {
        HKU_WARN("m_am is null [Portfolio::run]");
        return;
    }


    reset();
    m_sys->setTM(m_tm);

    bool delay = getParam<bool>("delay");
    m_sys->setParam<bool>("delay", delay);

    map<hku_uint64, SystemPtr> stock_map_buffer; //缓存
    map<hku_uint64, SystemPtr>::iterator stock_map_iter;

    SystemList pre_selected_sys;
    DatetimeList datelist = StockManager::instance().getTradingCalendar(query);
    DatetimeList::const_iterator date_iter = datelist.begin();
    for(; date_iter != datelist.end(); ++date_iter) {
        //处理当前持仓股
        PositionRecordList positions = m_tm->getPositionList();
        PositionRecordList::const_iterator pos_iter = positions.begin();
        for (; pos_iter != positions.end(); ++pos_iter) {
            stock_map_iter = stock_map_buffer.find(pos_iter->stock.id());
            //有可能TM中已经存在持仓的股票，而该股票并不在预定的股票范围内，则忽略
            if (stock_map_iter != stock_map_buffer.end()) {
                stock_map_iter->second->runMoment(*date_iter);
            }
        }

        //如果滞后操作,处理上次选择的股票
        if (delay) {
            SystemList::iterator sys_iter = pre_selected_sys.begin();
            for (; sys_iter != pre_selected_sys.end(); ++sys_iter) {
                (*sys_iter)->runMoment(*date_iter);
            }
            pre_selected_sys.clear();
        }

        //根据选股规则，计算当前时刻选择的股票列表
        StockList first_selected = m_st->getSelectedStock(*date_iter);

        //过滤掉已经持仓的股票
        StockList stock_selected;
        StockList::const_iterator stk_iter = first_selected.begin();
        for (; stk_iter != first_selected.end(); ++stk_iter) {
            if (!m_tm->have(*stk_iter)) {
                stock_selected.push_back(*stk_iter);
            }
        }

        //根据资金分配策略，返回实际欲购买的股票列表
        StockList act_selected = m_am->tryAllocate(*date_iter, stock_selected);
        stk_iter = act_selected.begin();
        for (; stk_iter != act_selected.end(); ++stk_iter) {
            SystemPtr sys;
            stock_map_iter = stock_map_buffer.find(stk_iter->id());
            if (stock_map_iter != stock_map_buffer.end()) {
                sys = stock_map_iter->second;
                //清除延迟请求,因为是间断的运行，而延迟买入有一个可被延迟的次数，但被再一次延迟后，延迟请求被滞留
                sys->clearRequest();
                sys->runMoment(*date_iter);

            } else {
                sys = m_sys->clone();
                KData k = stk_iter->getKData(query);
                sys->setTO(k);
                sys->runMoment(*date_iter);
                stock_map_buffer[stk_iter->id()] = sys;
            }

            if (delay) {
                pre_selected_sys.push_back(sys);
            }
        }
    }
}


} /* namespace hku */
