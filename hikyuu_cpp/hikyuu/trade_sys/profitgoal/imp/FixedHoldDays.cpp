/*
 * FixedHoldDays.cpp
 *
 *  Created on: 2018年1月20日
 *      Author: fasiondog
 */

#include "FixedHoldDays.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedHoldDays)
#endif

namespace hku {

FixedHoldDays::FixedHoldDays() : ProfitGoalBase("PG_FixedHoldDays") {
    setParam<int>("days", 5);
}

FixedHoldDays::~FixedHoldDays() {}

void FixedHoldDays::_calculate() {}

price_t FixedHoldDays::getGoal(const Datetime& datetime, price_t price) {
    HKU_WARN_IF_RETURN(getParam<int>("days") <= 0, 0.0, "param days <= 0! Are you sure?");

    Stock stk = m_kdata.getStock();
    PositionRecord position = m_tm->getPosition(datetime, stk);
    Datetime take_date = position.takeDatetime;

    KQuery query = KQueryByDate(Datetime(take_date.date()), Datetime(datetime.date()), KQuery::DAY);

    size_t start_out, end_out;
    if (stk.getIndexRange(query, start_out, end_out)) {
        size_t d = end_out - start_out;
        if (d >= getParam<int>("days")) {
            return 0.0;
        }
    }

    return Null<price_t>();
}

ProfitGoalPtr HKU_API PG_FixedHoldDays(int days) {
    ProfitGoalPtr ptr = make_shared<FixedHoldDays>();
    ptr->setParam<int>("days", days);
    return ptr;
}

} /* namespace hku */
