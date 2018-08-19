/*
 * Datetime_Serialization.h
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */

#ifndef DATETIME_SERIALIZATION_H_
#define DATETIME_SERIALIZATION_H_

#include "../datetime/Datetime.h"
#include "../config.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost {
namespace serialization {
template<class Archive>
void save(Archive & ar, const hku::Datetime & date, unsigned int version) {
    std::string datetime = date.toString();
    ar & BOOST_SERIALIZATION_NVP(datetime);
}

template<class Archive>
void load(Archive & ar, hku::Datetime& date, unsigned int version) {
    std::string datetime;
    ar & BOOST_SERIALIZATION_NVP(datetime);
    date = hku::Datetime(datetime);
}
}} /* namespace boost::serailization */

BOOST_SERIALIZATION_SPLIT_FREE(hku::Datetime)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* DATETIME_SERIALIZATION_H_ */
