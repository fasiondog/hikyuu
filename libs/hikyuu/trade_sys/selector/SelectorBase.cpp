/*
 * SelectorBase.cpp
 *
 *  Created on: 2016年2月21日
 *      Author: fasiondog
 */

#include "SelectorBase.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const SelectorBase& st) {
    os << "Selector(" << st.name() << ", " << st.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const SelectorPtr& st) {
    if (st) {
        os << *st;
    } else {
        os << "Selector(NULL)";
    }

    return os;
}

SelectorBase::SelectorBase(): m_name("SelectorBase") {
    setParam<int>("freq", KQuery::DAY);
}

SelectorBase::SelectorBase(const string& name): m_name(name) {
    setParam<int>("freq", KQuery::DAY);
}


SelectorBase::~SelectorBase() {

}

void SelectorBase::clear() {
    m_sys_list.clear();
    m_pre_selected_list.clear();
    m_pre_datetime = Null<Datetime>();
}

void SelectorBase::reset() {
    SystemList::const_iterator iter = m_sys_list.begin();
    for (; iter != m_sys_list.end(); ++iter) {
        (*iter)->reset(true, false);
    }

    m_pre_datetime = Null<Datetime>();
    m_pre_selected_list.clear();

    _reset();
}

SelectorPtr SelectorBase::clone() {
    SelectorPtr p;
    try {
        p = _clone();
    } catch(...) {
        HKU_ERROR("Subclass _clone failed! [SelectorBase::clone]");
        p = SelectorPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr! [SelectorBase::clone]" );
        return shared_from_this();
    }

    p->m_params = m_params;

    p->m_name = m_name;
    p->m_query = m_query;
    p->m_pre_datetime = m_pre_datetime;
    p->m_pre_selected_list = m_pre_selected_list;

    SystemList::const_iterator iter = m_sys_list.begin();
    for (; iter != m_sys_list.end(); ++iter) {
        p->m_sys_list.push_back((*iter)->clone(true, false));
    }

    return p;
}


void SelectorBase::addStock(const Stock& stock, const SystemPtr& protoSys) {
    if (stock.isNull()) {
        HKU_WARN("Try add Null stock, will be discard! "
                "[SelectorBase::addStock]");
        return;
    }

    if (!protoSys) {
        HKU_WARN("Try add Null protoSys, will be discard! "
                "[SelectorBase::addStock]");
        return;
    }

    SYSPtr sys = protoSys->clone(true, false);
    sys->setStock(stock);
    m_sys_list.push_back(sys);
}


void SelectorBase::addStockList(const StockList& stkList,
                                const SystemPtr& protoSys) {
    if (!protoSys) {
        HKU_WARN("Try add Null protoSys, will be discard! "
                "[SelectorBase::addStockList]");
        return;
    }

    StockList::const_iterator iter = stkList.begin();
    for (; iter != stkList.end(); ++iter) {
        if (iter->isNull()) {
            HKU_WARN("Try add Null stock, will be discard! "
                    "[SelectorBase::addStockList]");
            continue;
        }

        SYSPtr sys = protoSys->clone(true, false);
        m_sys_list.push_back(sys);
    }
}

bool SelectorBase::changed(Datetime date) {
    if (date == Null<Datetime>())
        return false;

    int freq = getParam<int>("freq");
    Datetime cur_date;
    if (KQuery::DAY == freq) {
        cur_date = Datetime(date.date());

    } else if (KQuery::WEEK == freq) {
        cur_date = Datetime(date.date() - bd::date_duration(date.dayOfWeek()));

    } else if (KQuery::MONTH == freq) {
        cur_date = Datetime(date.year(), date.month(), 1);

    } else if (KQuery::QUARTER == freq) {
        int year = date.year();
        int month = date.month();
        if (month == 1 || month == 2 || month == 3) {
            cur_date = Datetime(year, 1, 1);
        } else if (month == 4 || month == 5 || month == 6) {
            cur_date = Datetime(year, 4, 1);
        } else if (month == 7 || month == 8 || month == 9) {
            cur_date = Datetime(year, 7, 1);
        } else {
            cur_date = Datetime(year, 10, 1);
        }

    } else if (KQuery::HALFYEAR == freq) {
        cur_date = Datetime(date.year(), 7, 1);

    } else if (KQuery::YEAR == freq) {
        cur_date = Datetime(date.year(), 1, 1);

    } else if (KQuery::MIN == freq) {
        cur_date = Datetime(date.year(), date.month(), date.day(),
                            date.hour(), date.minute());

    } else if (KQuery::MIN5 == freq) {
        cur_date = Datetime(date.year(), date.month(), date.day(),
                            date.hour(), date.minute()/5);

    } else if (KQuery::MIN15 == freq) {
        cur_date = Datetime(date.year(), date.month(), date.day(),
                            date.hour(), date.minute()/15);

    } else if (KQuery::MIN30 == freq) {
        cur_date = Datetime(date.year(), date.month(), date.day(),
                            date.hour(), date.minute()/30);

    } else if (KQuery::MIN60 == freq) {
        cur_date = Datetime(date.year(), date.month(), date.day(), date.hour());

    } else {
        HKU_WARN("Invalie param freq " << freq << "! [SelectorBase::changed]");
        return false;
    }

    if (m_pre_datetime != cur_date) {
        m_pre_datetime = cur_date;
        return true;
    }

    return false;

    //return true;
}

SystemList SelectorBase::getSelectedSystemList(Datetime date) {
    if (changed(date)) {
        m_pre_selected_list = _getSelectedSystemList(date);
    }

    return m_pre_selected_list;
}


} /* namespace hku */
