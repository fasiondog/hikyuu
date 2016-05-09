/*
 * OPCondition.cpp
 *
 *  Created on: 2016年5月9日
 *      Author: Administrator
 */

#include <hikyuu/trade_sys/condition/imp/OPLineCondition.h>
#include "../../../trade_manage/crt/crtTM.h"
#include "../../../trade_manage/crt/TC_Zero.h"
#include "../../../indicator/crt/PRICELIST.h"

namespace hku {

OPLineCondition::OPLineCondition()
: ConditionBase("OPLine") {

}

OPLineCondition::OPLineCondition(const Operand& op)
: ConditionBase("OPLine"), m_op(op) {

}

OPLineCondition::~OPLineCondition() {

}

ConditionPtr OPLineCondition::_clone() {
    OPLineCondition *ptr = new OPLineCondition(m_op);
    return ConditionPtr(ptr);
}

void OPLineCondition::_reset() {

}

void OPLineCondition::_calculate() {
    if (m_kdata.size() == 0)
        return;

    TradeManagerPtr tm = crtTM(m_kdata[0].datetime, 0.0, TC_Zero());
    KQuery::KType ktype = m_kdata.getQuery().kType();
    DatetimeList dates = m_kdata.getDatetimeList();
    Indicator profit = PRICELIST(tm->getProfitCurve(dates, ktype));
    Indicator op = m_op(profit);

    size_t discard = profit.discard() > op.discard()
            ? profit.discard() : op.discard();

    for (size_t i = 0; i < discard; i++) {
        _addInvalid(dates[i]);
    }

    size_t total = dates.size();
    bool hold = false;
    for (size_t i = discard + 1; i < total; i++) {
        if (profit[i-1] < op[i-1] && profit[i] >= op[i]) {
            hold = true;
        } else if (profit[i-1] > op[i-1] && profit[i] >= op[i]) {
            hold = false;
        }

        if (hold) {
            _addValid(dates[i]);
        } else {
            _addInvalid(dates[i]);
        }
    }
}

bool OPLineCondition::isValid(const Datetime& datetime) {
    if (m_valid.count(datetime) != 0) {
        return true;
    } else if (m_invalid.count(datetime) != 0) {
        return false;
    }
    return false;
}

CNPtr HKU_API CN_OPLine(const Operand& op) {
    return CNPtr(new OPLineCondition(op));
}

} /* namespace hku */
