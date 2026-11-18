/*
 * MarginBase.cpp
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#include <algorithm>
#include <cmath>
#include "MarginBase.h"
#include "../../StockTypeInfo.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MarginBase)
#endif

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const MarginBase& mg) {
    os << "MarginFunc(" << mg.name() << ", " << mg.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const MarginPtr& mg) {
    if (mg) {
        os << *mg;
    } else {
        os << "MarginFunc(NULL)";
    }
    return os;
}

MarginBase::MarginBase(const string& name) : m_name(name) {}

MarginBase::~MarginBase() {}

void MarginBase::baseCheckParam(const string& name) const {}
void MarginBase::paramChanged() {}

MarginPtr MarginBase::clone() {
    MarginPtr result = _clone();
    MarginBase* p = result.get();
    // setParameter (not assignment) re-runs the param check and rebuilds derived state
    p->setParameter(m_params);
    p->m_name = m_name;
    p->m_is_python_object = m_is_python_object;
    return result;
}

price_t MarginBase::nominal(const Stock& stock, price_t price, double num) {
    if (!std::isfinite(price) || !std::isfinite(num) || price <= 0.0 || num <= 0.0)
        return 0.0;
    return price * num * stock.unit();
}

bool MarginBase::needBorrowAsset(const Stock& stock) const {
    // Borrow/return follows A-share securities-lending semantics for built-in categories. Crypto
    // and custom types (>= 12, e.g. futures, forex, US/HK stocks for a new market) short without
    // borrowing; the temporary placeholder is not traded. Override when a market has another rule.
    return stock.type() < STOCKTYPE_TMP &&
           (stock.type() < STOCKTYPE_CRYPTO || stock.type() == STOCKTYPE_A_BJ);
}

price_t MarginBase::getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num, MarginSide side) const {
    return getOpenMargin(datetime, stock, price, num, side);
}

double MarginBase::getMarginRate(const Datetime& datetime, const Stock& stock,
                                 MarginSide side) const {
    return 1.0;
}

double MarginBase::getMaxOpenNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                    price_t available, MarginSide side) const {
    if (!std::isfinite(available) || !std::isfinite(price) || available <= 0.0 || price <= 0.0)
        return 0.0;

    // Query the margin of one minimum trading unit, not one share, so a per-lot margin scales
    double atom = stock.minTradeNumber();
    if (!std::isfinite(atom) || atom <= 0.0)
        atom = 1.0;

    price_t per_atom = getOpenMargin(datetime, stock, price, atom, side);
    if (!std::isfinite(per_atom) || per_atom <= 0.0)
        return 0.0;

    // Tolerate FP rounding at a unit boundary, else a quota equal to one unit's margin floors to 0
    double units = available / per_atom;
    units = std::floor(units + std::max(1e-9, units * 1e-12));
    double num = units * atom;
    if (num <= 0.0)
        return 0.0;

    double max_num = stock.maxTradeNumber();
    return num > max_num ? max_num : num;
}

} /* namespace hku */
