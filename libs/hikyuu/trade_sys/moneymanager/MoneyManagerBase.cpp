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
: m_name("MoneyManagerBase"), m_ktype(KQuery::DAY){

}

MoneyManagerBase::MoneyManagerBase(const string& name)
: m_name(name), m_ktype(KQuery::DAY) {

}

MoneyManagerBase::~MoneyManagerBase() {

}

void MoneyManagerBase::buyNotify(const TradeRecord&) {

}

void MoneyManagerBase::sellNotify(const TradeRecord&) {

}

MoneyManagerPtr MoneyManagerBase::clone() {
    MoneyManagerPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_tm = m_tm;
    p->m_ktype = m_ktype;
    return p;
}

size_t MoneyManagerBase
::getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
    if (!m_tm) {
        HKU_ERROR("m_tm is null! Datetime(" << datetime << ") Stock("
                << stock.market_code() << ") price(" << price
                << ") risk(" << risk
                << ") [MoneyManagerBase::getSellNumber]");
        return 0;
    }

    if (risk <= 0.0) {
        HKU_ERROR("risk is zero! Datetime(" << datetime << ") Stock("
                << stock.market_code() << ") price(" << price
                << ") risk(" << risk
                << ") [MoneyManagerBase::getSellNumber]");
        return 0;
    }

    return _getSellNumber(datetime, stock, price, risk);
}

size_t MoneyManagerBase
::getBuyNumber(const Datetime& datetime, const Stock& stock,
        price_t price, price_t risk) {
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
                << ") [MoneyManagerBase::getBuyNumber]");
        return 0;
    }

    return _getBuyNumber(datetime, stock, price, risk);
}

size_t MoneyManagerBase
::getSellShortNumber(const Datetime& datetime, const Stock& stock,
        price_t price, price_t risk) {
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

    return _getSellShortNumber(datetime, stock, price, risk);
}

size_t MoneyManagerBase
::getBuyShortNumber(const Datetime& datetime, const Stock& stock,
        price_t price, price_t risk) {
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

    return _getBuyShortNumber(datetime, stock, price, risk);
}

size_t MoneyManagerBase::_getSellNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {

    //默认卖出全部
    return m_tm->getHoldNumber(datetime, stock);
}

size_t MoneyManagerBase::_getSellShortNumber(const Datetime& datetime,
        const Stock& stock, price_t price, price_t risk) {
    return 0;
}

size_t MoneyManagerBase::_getBuyShortNumber(const Datetime& datetime,
        const Stock& stock, price_t price, price_t risk) {
    return 0;
}

} /* namespace hku */
