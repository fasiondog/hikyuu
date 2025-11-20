/*
 * FixedPercentProfitGoal.cpp
 *
 *  Created on: 2016年5月6日
 *      Author: Administrator
 */

#include "FixedPercentProfitGoal.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedPercentProfitGoal)
#endif

namespace hku {

FixedPercentProfitGoal::FixedPercentProfitGoal() : ProfitGoalBase("PG_FixedPercent") {
    setParam<double>("p", 0.2);
}

/**
 * @brief 析构函数，释放 FixedPercentProfitGoal 对象资源
 *
 * 该析构函数是 FixedPercentProfitGoal 类的默认析构函数实现，
 * 负责清理对象占用的资源。由于该类没有动态分配的资源，
 * 因此使用默认的空实现。
 */
FixedPercentProfitGoal::~FixedPercentProfitGoal() {}

void FixedPercentProfitGoal::_checkParam(const string& name) const {
    if ("p" == name) {
        double p = getParam<double>(name);
        HKU_ASSERT(p > 0.0);
    }
}

price_t FixedPercentProfitGoal::getGoal(const Datetime& datetime, price_t price) {
    Stock stock = getTO().getStock();
    PositionRecord position = getTM()->getPosition(datetime, stock);
    return position.number != 0
             ? (position.buyMoney / position.number) * (1 + getParam<double>("p"))
             : price * (1 + getParam<double>("p"));
}

ProfitGoalPtr HKU_API PG_FixedPercent(double p) {
    ProfitGoalPtr ptr = make_shared<FixedPercentProfitGoal>();
    ptr->setParam<double>("p", p);
    return ptr;
}

} /* namespace hku */
