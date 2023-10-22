/*
 * TimeDelta_serialization.h
 *
 *  Copyright (C) 2019 hikyuu.org
 *
 *  Created on: 2019-12-14
 *      Author: fasiondog
 */

#pragma once
#ifndef TIMEDELTA_SERIALIZATION_H_
#define TIMEDELTA_SERIALIZATION_H_

#include "hikyuu/utilities/datetime/TimeDelta.h"
#include "../config.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost {
namespace serialization {

template <class Archive>
void save(Archive& ar, const hku::TimeDelta& td, unsigned int version) {
    int64_t ticks = td.ticks();
    ar& BOOST_SERIALIZATION_NVP(ticks);
}

template <class Archive>
void load(Archive& ar, hku::TimeDelta& td, unsigned int version) {
    int64_t ticks;
    ar& BOOST_SERIALIZATION_NVP(ticks);
    td = hku::TimeDelta(boost::posix_time::time_duration(0, 0, 0, ticks));
}

}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::TimeDelta)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* TIMEDELTA_SERIALIZATION_H_ */
