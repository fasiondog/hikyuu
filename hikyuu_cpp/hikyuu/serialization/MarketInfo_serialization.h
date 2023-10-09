/*
 * MarketInfo.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef MARKETINFO_SERIALIZATION_H_
#define MARKETINFO_SERIALIZATION_H_

#include "../config.h"
#include "../MarketInfo.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>
#include "TimeDelta_serialization.h"

namespace boost {
namespace serialization {

template <class Archive>
void save(Archive& ar, const hku::MarketInfo& record, unsigned int version) {
    hku::string market = record.market();
    hku::string name = record.name();
    hku::string description = record.description();
    hku::string code = record.code();
    hku::uint64_t lastDate = record.lastDate().number();
    hku::TimeDelta openTime1 = record.openTime1();
    hku::TimeDelta openTime2 = record.openTime2();
    hku::TimeDelta closeTime1 = record.closeTime1();
    hku::TimeDelta closeTime2 = record.closeTime2();
    ar& BOOST_SERIALIZATION_NVP(market);
    ar& BOOST_SERIALIZATION_NVP(name);
    ar& BOOST_SERIALIZATION_NVP(description);
    ar& BOOST_SERIALIZATION_NVP(code);
    ar& BOOST_SERIALIZATION_NVP(lastDate);
    ar& BOOST_SERIALIZATION_NVP(openTime1);
    ar& BOOST_SERIALIZATION_NVP(closeTime1);
    ar& BOOST_SERIALIZATION_NVP(openTime2);
    ar& BOOST_SERIALIZATION_NVP(closeTime2);
}

template <class Archive>
void load(Archive& ar, hku::MarketInfo& record, unsigned int version) {
    hku::string market, name, description, code;
    hku::uint64_t lastDate;
    hku::TimeDelta openTime1, openTime2, closeTime1, closeTime2;
    ar& BOOST_SERIALIZATION_NVP(market);
    ar& BOOST_SERIALIZATION_NVP(name);
    ar& BOOST_SERIALIZATION_NVP(description);
    ar& BOOST_SERIALIZATION_NVP(code);
    ar& BOOST_SERIALIZATION_NVP(lastDate);
    ar& BOOST_SERIALIZATION_NVP(openTime1);
    ar& BOOST_SERIALIZATION_NVP(closeTime1);
    ar& BOOST_SERIALIZATION_NVP(openTime2);
    ar& BOOST_SERIALIZATION_NVP(closeTime2);
    record = hku::MarketInfo(market, name, description, code, hku::Datetime(lastDate), openTime1,
                             closeTime1, openTime2, closeTime2);
}

}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::MarketInfo)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* MARKETINFO_SERIALIZATION_H_ */
