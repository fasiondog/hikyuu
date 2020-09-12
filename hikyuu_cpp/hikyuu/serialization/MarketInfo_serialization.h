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
#include "../utilities/util.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>

namespace boost {
namespace serialization {

template <class Archive>
void save(Archive& ar, const hku::MarketInfo& record, unsigned int version) {
    hku::string market = record.market();
    hku::string name = record.name();
    hku::string description = record.description();
    hku::string code = record.code();
    hku::uint64 lastDate = record.lastDate().number();
    ar& BOOST_SERIALIZATION_NVP(market);
    ar& BOOST_SERIALIZATION_NVP(name);
    ar& BOOST_SERIALIZATION_NVP(description);
    ar& BOOST_SERIALIZATION_NVP(code);
    ar& BOOST_SERIALIZATION_NVP(lastDate);
}

template <class Archive>
void load(Archive& ar, hku::MarketInfo& record, unsigned int version) {
    hku::string market, name, description, code;
    hku::uint64 lastDate;
    ar& BOOST_SERIALIZATION_NVP(market);
    ar& BOOST_SERIALIZATION_NVP(name);
    ar& BOOST_SERIALIZATION_NVP(description);
    ar& BOOST_SERIALIZATION_NVP(code);
    ar& BOOST_SERIALIZATION_NVP(lastDate);
    record = hku::MarketInfo(market, name, description, code, hku::Datetime(lastDate));
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::MarketInfo)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* MARKETINFO_SERIALIZATION_H_ */
