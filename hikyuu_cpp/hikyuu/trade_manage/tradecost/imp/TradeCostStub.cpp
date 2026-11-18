/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-01-24
 *      Author: fasiondog
 */

#include "TradeCostStub.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TradeCostStub)
#endif

namespace hku {

TradeCostStub::TradeCostStub() : TradeCostBase("TestStub") {}

TradeCostStub::~TradeCostStub() {}

CostRecord TradeCostStub::getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                     double num) const {
    return CostRecord(0, 0, 0, 10, 10);
}

CostRecord TradeCostStub::getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num) const {
    return CostRecord(0, 0, 0, 20, 20);
}

CostRecord TradeCostStub::getBorrowCashCost(const Datetime& datetime, price_t cash) const {
    return CostRecord(0, 0, 0, 30, 30);
}

CostRecord TradeCostStub::getReturnCashCost(const Datetime& borrow_datetime,
                                            const Datetime& return_datetime, price_t cash) const {
    return CostRecord(0, 0, 0, 40, 40);
}

CostRecord TradeCostStub::getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                             price_t price, double num) const {
    return CostRecord(0, 0, 0, 50, 50);
}

CostRecord TradeCostStub::getReturnStockCost(const Datetime& borrow_datetime,
                                             const Datetime& return_datetime, const Stock& stock,
                                             price_t price, double num) const {
    return CostRecord(0, 0, 0, 60, 60);
}

TradeCostPtr TradeCostStub::_clone() {
    return make_shared<TradeCostStub>();
}

}  // namespace hku