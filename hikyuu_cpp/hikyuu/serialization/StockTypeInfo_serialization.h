/*
 * StockTypeInfo.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef STOCKTYPEINFO_SERIALIZATION_H_
#define STOCKTYPEINFO_SERIALIZATION_H_

#include "../config.h"
#include "../StockTypeInfo.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost {
namespace serialization {

template <class Archive>
void save(Archive& ar, const hku::StockTypeInfo& record, unsigned int version) {
    hku::uint32_t type = record.type();
    hku::string description = record.description();
    hku::price_t tick = record.tick();
    hku::price_t tickValue = record.tickValue();
    int precision = record.precision();
    double minTradeNumber = record.minTradeNumber();
    double maxTradeNumber = record.maxTradeNumber();
    ar& BOOST_SERIALIZATION_NVP(type);
    ar& BOOST_SERIALIZATION_NVP(description);
    ar& BOOST_SERIALIZATION_NVP(tick);
    ar& BOOST_SERIALIZATION_NVP(tickValue);
    ar& BOOST_SERIALIZATION_NVP(precision);
    ar& BOOST_SERIALIZATION_NVP(minTradeNumber);
    ar& BOOST_SERIALIZATION_NVP(maxTradeNumber);
}

template <class Archive>
void load(Archive& ar, hku::StockTypeInfo& record, unsigned int version) {
    hku::uint32_t type;
    hku::string description;
    hku::price_t tick, tickValue;
    int precision;
    double minTradeNumber, maxTradeNumber;
    ar& BOOST_SERIALIZATION_NVP(type);
    ar& BOOST_SERIALIZATION_NVP(description);
    ar& BOOST_SERIALIZATION_NVP(tick);
    ar& BOOST_SERIALIZATION_NVP(tickValue);
    ar& BOOST_SERIALIZATION_NVP(precision);
    ar& BOOST_SERIALIZATION_NVP(minTradeNumber);
    ar& BOOST_SERIALIZATION_NVP(maxTradeNumber);
    record = hku::StockTypeInfo(type, description, tick, tickValue, precision, minTradeNumber,
                                maxTradeNumber);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::StockTypeInfo)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* STOCKTYPEINFO_SERIALIZATION_H_ */
