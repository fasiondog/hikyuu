/*
 * FixedPercentProfitGoal.cpp
 *
 *  Created on: 2016-5-6
 *      Author: Administrator
 */

#include "FixedPercentProfitGoal.h"

#include <cmath>

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedPercentProfitGoal)
#endif

namespace hku {

namespace {

// Round to the stock tick grid with a tolerance on the scaled value, so that an exact on-grid
// target (e.g. 15 * 1.2 = 18.0) is not pushed to the neighboring tick by representation noise;
// up returns the smallest on-grid price >= v, down the largest <= v
price_t roundToTick(double v, int precision, bool up) {
    double scale = std::pow(10.0, precision);
    return up ? std::ceil(v * scale - 1e-6) / scale : std::floor(v * scale + 1e-6) / scale;
}

}  // namespace

FixedPercentProfitGoal::FixedPercentProfitGoal() : ProfitGoalBase("PG_FixedPercent") {
    setParam<double>("p", 0.2);
}

/**
 * @brief Destructor, it releases the resources of the FixedPercentProfitGoal object
 *
 * This destructor is the default destructor implementation of the FixedPercentProfitGoal class,
 * responsible for cleaning up the resources occupied by the object. Because this class has no
 * dynamically allocated resources, the default empty implementation is used.
 */
FixedPercentProfitGoal::~FixedPercentProfitGoal() {}

void FixedPercentProfitGoal::_checkParam(const string& name) const {
    if ("p" == name) {
        double p = getParam<double>(name);
        HKU_ASSERT(p > 0.0);
    }
}

price_t FixedPercentProfitGoal::_getGoal(const Datetime& datetime, price_t price) {
    HKU_IF_RETURN(!m_tm, Null<price_t>());
    Stock stock = m_kdata.getStock();
    int precision = stock.isNull() ? 2 : stock.precision();
    PositionRecord position = m_tm->getPosition(datetime, stock);
    // buyMoney and totalNumber are both accumulated over the position life, so the cost price
    // stays correct after a partial sell
    double cost = position.number > 0.0 ? position.buyMoney / position.totalNumber : price;
    return roundToTick(cost * (1 + getParam<double>("p")), precision, true);
}

price_t FixedPercentProfitGoal::_getShortGoal(const Datetime& datetime, price_t price) {
    HKU_IF_RETURN(!m_tm, Null<price_t>());
    Stock stock = m_kdata.getStock();
    int precision = stock.isNull() ? 2 : stock.precision();
    PositionRecord position = m_tm->getShortPosition(stock);
    // sellMoney and totalNumber are both accumulated by the short opens
    double cost = position.number > 0.0 ? position.sellMoney / position.totalNumber : price;
    return roundToTick(cost * (1 - getParam<double>("p")), precision, false);
}

ProfitGoalPtr HKU_API PG_FixedPercent(double p) {
    ProfitGoalPtr ptr = make_shared<FixedPercentProfitGoal>();
    ptr->setParam<double>("p", p);
    return ptr;
}

} /* namespace hku */
