/*
 * OPCondition.cpp
 *
 *  Created on: 2016-5-9
 *      Author: Administrator
 */

#include <cmath>
#include "OPLineCondition.h"
#include "../../../trade_manage/crt/crtTM.h"
#include "../../../trade_manage/crt/TC_Zero.h"
#include "../../../indicator/crt/PRICELIST.h"
#include "../../system/crt/SYS_Simple.h"
#include "../../moneymanager/crt/MM_FixedCount.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OPLineCondition)
#endif

namespace hku {

OPLineCondition::OPLineCondition() : ConditionBase("CN_OPLine") {}

OPLineCondition::OPLineCondition(const Indicator& op)
: ConditionBase("CN_OPLine"), m_op(op.clone()) {}

OPLineCondition::~OPLineCondition() {}

ConditionPtr OPLineCondition::_clone() {
    return make_shared<OPLineCondition>(m_op.clone());
}

void OPLineCondition::_calculate() {
    HKU_CHECK(m_sg, "CN_OPLine requires a signal generator; set it via setSG() before use");
    Stock stock = m_kdata.getStock();
    KQuery query = m_kdata.getQuery();

    // Reuse the previous embedded-backtest result when the inputs are unchanged; the backtest is the
    // dominant cost and is run in full on every setTO otherwise
    if (!m_last_x.empty() && m_last_stock == stock && m_last_query == query && m_last_sg == m_sg) {
        DatetimeList dates = m_kdata.getDatetimeList();
        auto const* xdata = m_last_x.data();
        for (size_t i = 0; i < m_last_x.size(); i++) {
            if (std::isfinite(xdata[i]) && xdata[i] > 0) {
                _addValid(dates[i]);
            }
        }
        return;
    }

    // valid range follows SYS_Simple/System default params, intentionally unpinned
    SYSPtr sys = SYS_Simple();

    TMPtr tm = crtTM(m_kdata[0].datetime, 0.0, TC_Zero());
    MMPtr mm = MM_FixedCount(stock.minTradeNumber());
    mm->setParam<bool>("auto_checkin", true);

    sys->setTM(tm);
    sys->setMM(mm);
    // The embedded backtest owns a cloned SG: SYS_Simple resets and re-runs its SG during run(),
    // which would otherwise wipe and mutate the SG shared with the outer system
    sys->setSG(m_sg->clone());

    sys->run(m_kdata.getStock(), m_kdata.getQuery());
    KQuery::KType ktype = query.kType();
    DatetimeList dates = m_kdata.getDatetimeList();
    Indicator profit = PRICELIST(tm->getProfitCurve(dates, ktype));
    Indicator op = m_op(profit);

    Indicator x = profit - op;
    HKU_CHECK(x.size() == dates.size(),
              "The result size ({}) of the OP line indicator must be equal to the KData size ({})",
              x.size(), dates.size());
    auto const* xdata = x.data();
    for (size_t i = 0; i < x.size(); i++) {
        if (std::isfinite(xdata[i]) && xdata[i] > 0) {
            _addValid(dates[i]);
        }
    }

    m_last_stock = stock;
    m_last_query = query;
    m_last_sg = m_sg;
    m_last_x = x;
}

CNPtr HKU_API CN_OPLine(const Indicator& op) {
    return make_shared<OPLineCondition>(op);
}

} /* namespace hku */
