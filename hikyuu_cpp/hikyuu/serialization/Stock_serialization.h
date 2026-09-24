/*
 * Stock_Serialization.h
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */

#pragma once
#ifndef STOCK_SERIALIZATION_H_
#define STOCK_SERIALIZATION_H_

#include "../config.h"
#include "../Stock.h"

//===========================================================================
// The following is the serialization of Stock; currently only the serialization of the Stock
// managed in StockManager is implemented
// Note: this piece of code is outside namespace hku
// TODO implement the serialization of the Stock that is not managed by StockManager
//===========================================================================
#if HKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {
template <class Archive>

void save(Archive& ar, const hku::Stock& stock, unsigned int version) {
    hku::string market_code = stock.market_code();
    ar& BOOST_SERIALIZATION_NVP(market_code);
}

template <class Archive>
void load(Archive& ar, hku::Stock& stock, unsigned int version) {
    hku::string market_code, name;
    ar& BOOST_SERIALIZATION_NVP(market_code);
    stock = hku::getStock(market_code);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::Stock)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* STOCK_SERIALIZATION_H_ */
