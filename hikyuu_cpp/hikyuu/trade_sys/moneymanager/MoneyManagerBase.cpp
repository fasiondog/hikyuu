/*
 * MoneyManagerBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "MoneyManagerBase.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const MoneyManagerBase& mm) {
    os << "MoneyManager(" << mm.name() << ", " << mm.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const MoneyManagerPtr& mm) {
    if (mm) {
        os << *mm;
    } else {
        os << "MoneyManager(NULL)";
    }

    return os;
}

MoneyManagerBase::MoneyManagerBase()
: m_name("MoneyManagerBase") {
    setParam<bool>("auto-checkin", false);
    setParam<int>("max-stock", 20000);
    setParam<bool>("disable_ev_force_clean_position", false);
    setParam<bool>("disable_cn_force_clean_position", false);
}

MoneyManagerBase::MoneyManagerBase(const string& name)
: m_name(name) {
    setParam<bool>("auto-checkin", false);
    setParam<int>("max-stock", 20000);
    setParam<bool>("disable_ev_force_clean_position", false);
    setParam<bool>("disable_cn_force_clean_position", false);
}

MoneyManagerBase::~MoneyManagerBase() {

}

void MoneyManagerBase::buyNotify(const TradeRecord&) {

}

void MoneyManagerBase::sellNotify(const TradeRecord&) {

}

MoneyManagerPtr MoneyManagerBase::clone() {
    MoneyManagerPtr p;
    try {
        p = _clone();
    } catch(...) {
        HKU_ERROR("Subclass _clone failed! [MoneyManagerBase::clone]");
        p = MoneyManagerPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr! [MoneyManagerBase::clone]" );
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_name = m_name;
    //p->m_tm = m_tm;
    p->m_query = m_query;
    return p;
}

size_t MoneyManagerBase
::getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
    if (!m_tm) {
        HKU_ERROR("m_tm is null! Datetime(" << datetime << ") Stock("
                << stock.market_code() << ") price(" << price
                << ") risk(" << risk
                << ") [MoneyManagerBase::getSellNumber]");
        return 0;
    }

    if (PART_ENVIRONMENT == from) {
        if (false == getParam<bool>("disable_ev_force_clean_position")) {
            //强制全部卖出
            return Null<size_t>();
        }
    }

    if (PART_CONDITION == from) {
        if (false == getParam<bool>("disable_cn_force_clean_position")) {
            return Null<size_t>();
        }
    }

    if (risk <= 0.0) {
        //HKU_WARN("risk is zero! Will not sell! "
        //        << "Datetime(" << datetime << ") Stock("
        //        << stock.market_code() << ") price(" << price
        //        << ") risk(" << risk
        //        << ") [MoneyManagerBase::getSellNumber]");
        return 0;
    }

    return _getSellNumber(datetime, stock, price, risk, from);;
}

size_t MoneyManagerBase
::getBuyNumber(const Datetime& datetime, const Stock& stock,
        price_t price, price_t risk, SystemPart from) {
    if (!m_tm) {
        HKU_ERROR("m_tm is null! Datetime(" << datetime << ") Stock("
                << stock.market_code() << ") price(" << price
                << ") risk(" << risk
                << ") [MoneyManagerBase::getBuyNumber]");
        return 0;
    }

    if (stock.isNull()) {
        HKU_ERROR("stock is Null!" << " [MoneyManagerBase::getBuyNumber]");
        return 0;
    }

    if (risk <= 0.0) {
        HKU_ERROR("risk is zero! Datetime(" << datetime << ") Stock("
                << stock.market_code() << ") price(" << price
                << ") risk(" << risk
                << ") Part(" << getSystemPartName(from)
                << ") [MoneyManagerBase::getBuyNumber]");
        return 0;
    }

    if (m_tm->getStockNumber() >= getParam<int>("max-stock")) {
        return 0;
    }

    size_t n = _getBuyNumber(datetime, stock, price, risk, from);

    if (n < stock.minTradeNumber()) {
        //HKU_INFO("Ignore! Is less than the minimum number of transactions("
        //        << stock.minTradeNumber()
        //        << ")! [MoneyManagerBase::getBuyNumber]");
        return 0;
    }

    //转换为最小交易量的整数倍
    n = (n / stock.minTradeNumber()) * stock.minTradeNumber();

    if (n > stock.maxTradeNumber()) {
        n = stock.maxTradeNumber();
        HKU_INFO("Over stock.maxTradeNumber! MoneyManagerBase::getBuyNumber]");
    }

    //在现金不足时，自动补充存入现金
    if (getParam<bool>("auto-checkin")) {
        price_t cash = m_tm->currentCash();
        CostRecord cost = m_tm->getBuyCost(datetime, stock, price, n);
        int precision = m_tm->precision();
        price_t money = roundUp(price * n * stock.unit() + cost.total, precision);
        if (money > cash) {
            m_tm->checkin(datetime, roundUp(money - cash, precision));
        }
    }

    return n;
}

size_t MoneyManagerBase
::getSellShortNumber(const Datetime& datetime, const Stock& stock,
        price_t price, price_t risk, SystemPart from) {
    if (!m_tm) {
        HKU_ERROR("m_tm is null! Datetime(" << datetime << ") Stock("
                << stock.market_code() << ") price(" << price
                << ") risk(" << risk
                << ") [MoneyManagerBase::getSellShortNumber]");
        return 0;
    }

    if (risk <= 0.0) {
        HKU_ERROR("risk is zero! Datetime(" << datetime << ") Stock("
                << stock.market_code() << ") price(" << price
                << ") risk(" << risk
                << ") [MoneyManagerBase::getSellShortNumber]");
        return 0;
    }

    return _getSellShortNumber(datetime, stock, price, risk, from);
}

size_t MoneyManagerBase
::getBuyShortNumber(const Datetime& datetime, const Stock& stock,
        price_t price, price_t risk, SystemPart from) {
    if (!m_tm) {
        HKU_ERROR("m_tm is null! Datetime(" << datetime << ") Stock("
                << stock.market_code() << ") price(" << price
                << ") risk(" << risk
                << ") [MoneyManagerBase::getBuyShortNumber]");
        return 0;
    }

    if (risk <= 0.0) {
        HKU_ERROR("risk is zero! Datetime(" << datetime << ") Stock("
                << stock.market_code() << ") price(" << price
                << ") risk(" << risk
                << ") [MoneyManagerBase::getBuyShortNumber]");
        return 0;
    }

    return _getBuyShortNumber(datetime, stock, price, risk, from);
}

size_t MoneyManagerBase::_getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {

    //默认卖出全部
    //return m_tm->getHoldNumber(datetime, stock);
    return Null<size_t>();
}

size_t MoneyManagerBase::_getSellShortNumber(const Datetime& datetime,
        const Stock& stock, price_t price, price_t risk, SystemPart from) {
    return 0;
}

size_t MoneyManagerBase::_getBuyShortNumber(const Datetime& datetime,
        const Stock& stock, price_t price, price_t risk, SystemPart from) {
    return 0;
}

} /* namespace hku */
