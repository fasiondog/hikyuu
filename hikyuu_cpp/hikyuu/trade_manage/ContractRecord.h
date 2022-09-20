/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-09-18
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"
#include "hikyuu/Stock.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#endif

namespace hku {

struct ContractRecord {
    ContractRecord() = default;
    ContractRecord(const Stock& stk, const Datetime& datetime, price_t price, double number,
                   double marginRatio)
    : stock(stk), datetime(datetime), price(price), number(number), marginRatio(marginRatio) {
        frozenCash = price * number * stock.unit() * marginRatio;
    }

    Stock stock;
    Datetime datetime;         // 交易日期
    price_t price = 0.0;       // 成交价格
    double number = 0.0;       // 成交数量
    price_t frozenCash = 0.0;  // 占用保证金
    double marginRatio = 0.0;  // 上次结算时的保证金比例

private:
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(datetime);
        ar& BOOST_SERIALIZATION_NVP(price);
        ar& BOOST_SERIALIZATION_NVP(number);
        ar& BOOST_SERIALIZATION_NVP(frozenCash);
        ar& BOOST_SERIALIZATION_NVP(marginRatio);
    }
#endif
};

}  // namespace hku