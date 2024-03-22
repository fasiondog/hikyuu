/*
 * TimeLineRecord_serialization.h
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#pragma once
#ifndef TRANSRECORD_SERIALIZATION_H_
#define TRANSRECORD_SERIALIZATION_H_

#include "../config.h"
#include "../TransRecord.h"

#if HKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const hku::TransRecord& record, unsigned int version) {
    hku::uint64_t datetime = record.datetime.number();
    ar& BOOST_SERIALIZATION_NVP(datetime);
    ar& make_nvp("price", record.price);
    ar& make_nvp("vol", record.vol);
    ar& make_nvp("direct", record.direct);
}

template <class Archive>
void load(Archive& ar, hku::TransRecord& record, unsigned int version) {
    hku::uint64_t datetime;
    ar& BOOST_SERIALIZATION_NVP(datetime);
    record.datetime = hku::Datetime(datetime);
    ar& make_nvp("price", record.price);
    ar& make_nvp("vol", record.vol);
    ar& make_nvp("direct", record.direct);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::TransRecord)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* TRANSRECORD_SERIALIZATION_H_ */
