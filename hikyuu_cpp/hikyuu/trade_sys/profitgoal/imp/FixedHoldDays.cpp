/*
 * FixedHoldDays.cpp
 *
 *  Created on: 2018年1月20日
 *      Author: fasiondog
 */

#include "FixedHoldDays.h"

namespace hku {

FixedHoldDays::FixedHoldDays(): ProfitGoalBase("PG_FixedHoldDays") {
    setParam<int>("days", 5);
}

FixedHoldDays::~FixedHoldDays() {

}

void FixedHoldDays::_calculate() {

}

price_t FixedHoldDays::getGoal(const Datetime& datetime, price_t price) {
    if (getParam<int>("days") <= 0) {
        HKU_WARN("param days <= 0! Are you sure? [FixedHoldDays::getGoal]");
        return 0.0;
    }

    Stock stk = m_kdata.getStock();
    PositionRecord position = m_tm->getPosition(stk);
    Datetime take_date = position.takeDatetime;

    KQuery query = KQueryByDate(take_date.date(), datetime.date(), KQuery::DAY);

    size_t start_out, end_out;
    if (stk.getIndexRange(query, start_out, end_out)) {
        size_t d = end_out - start_out;
        if (d >=  getParam<int>("days")) {
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
