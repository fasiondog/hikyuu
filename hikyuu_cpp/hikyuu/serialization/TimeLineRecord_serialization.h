/*
 * TimeLineRecord_serialization.h
 *
 *  Created on: 2019-2-8
 *      Author: fasiondog
 */

#pragma once
#ifndef TIMELINERECORD_SERIALIZATION_H_
#define TIMELINERECORD_SERIALIZATION_H_

#include "../config.h"
#include "../TimeLineRecord.h"

#if HKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const hku::TimeLineRecord& record, unsigned int version) {
    hku::uint64_t datetime = record.datetime.number();
    ar& BOOST_SERIALIZATION_NVP(datetime);
    ar& make_nvp("price", record.price);
    ar& make_nvp("vol", record.vol);
}

template <class Archive>
void load(Archive& ar, hku::TimeLineRecord& record, unsigned int version) {
    hku::uint64_t datetime;
    ar& BOOST_SERIALIZATION_NVP(datetime);
    record.datetime = hku::Datetime(datetime);
    ar& make_nvp("price", record.price);
    ar& make_nvp("vol", record.vol);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::TimeLineRecord)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* TIMELINERECORD_SERIALIZATION_H_ */
