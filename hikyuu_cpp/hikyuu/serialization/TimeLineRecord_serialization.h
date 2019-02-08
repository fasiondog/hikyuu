/*
 * TimeLineRecord_serialization.h
 *
 *  Created on: 2019-2-8
 *      Author: fasiondog
 */

#ifndef TIMELINERECORD_SERIALIZATION_H_
#define TIMELINERECORD_SERIALIZATION_H_

#include "../config.h"
#include "../TimeLineRecord.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>

namespace boost {
namespace serialization {
template<class Archive>
void save(Archive & ar, const hku::TimeLineRecord& record, unsigned int version) {
    hku::hku_uint64 datetime = record.datetime.number();
    ar & BOOST_SERIALIZATION_NVP(datetime);
    ar & make_nvp("price", record.price);
    ar & make_nvp("vol", record.vol);
}

template<class Archive>
void load(Archive & ar, hku::TimeLineRecord& record, unsigned int version) {
    hku::hku_uint64 datetime;
    ar & BOOST_SERIALIZATION_NVP(datetime);
    record.datetime = hku::Datetime(datetime);
    ar & make_nvp("price", record.price);
    ar & make_nvp("vol", record.vol);
}
}} /* namespace boost::serailization */

BOOST_SERIALIZATION_SPLIT_FREE(hku::TimeLineRecord)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* TIMELINERECORD_SERIALIZATION_H_ */
