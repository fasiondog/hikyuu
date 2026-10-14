/*
 * FixedHoldDays.cpp
 *
 *  Created on: 2018-1-20
 *      Author: fasiondog
 */

#include "FixedHoldDays.h"

#include <algorithm>

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedHoldDays)
#endif

namespace hku {

FixedHoldDays::FixedHoldDays() : ProfitGoalBase("PG_FixedHoldDays") {
    setParam<int>("days", 5);
}

FixedHoldDays::~FixedHoldDays() {}

void FixedHoldDays::_checkParam(const string& name) const {
    if ("days" == name) {
        int days = getParam<int>(name);
        HKU_ASSERT(days > 0);
    }
}

void FixedHoldDays::_reset() {
    m_trading_days.clear();
}

void FixedHoldDays::_calculate() {
    m_trading_days.clear();
    size_t total = m_kdata.size();
    m_trading_days.reserve(total);
    for (size_t i = 0; i < total; ++i) {
        Datetime day = m_kdata[i].datetime.startOfDay();
        if (m_trading_days.empty() || m_trading_days.back() != day) {
            m_trading_days.emplace_back(day);
        }
    }
}

price_t FixedHoldDays::_getGoal(const Datetime& datetime, price_t price) {
    HKU_IF_RETURN(!m_tm, Null<price_t>());
    PositionRecord position = m_tm->getPosition(datetime, m_kdata.getStock());
    const Datetime& take_date = position.takeDatetime;
    HKU_IF_RETURN(take_date == Null<Datetime>(), Null<price_t>());

    // d = the number of traded days in [take_date, datetime)
    Datetime take_day = take_date.startOfDay();
    Datetime cur_day = datetime.startOfDay();
    auto first = std::lower_bound(m_trading_days.begin(), m_trading_days.end(), take_day);
    auto last = std::lower_bound(m_trading_days.begin(), m_trading_days.end(), cur_day);
    size_t d = last > first ? static_cast<size_t>(last - first) : 0;
    if (d >= getParam<int>("days")) {
        return 0.0;
    }

    return Null<price_t>();
}

ProfitGoalPtr HKU_API PG_FixedHoldDays(int days) {
    ProfitGoalPtr ptr = make_shared<FixedHoldDays>();
    ptr->setParam<int>("days", days);
    return ptr;
}

} /* namespace hku */
