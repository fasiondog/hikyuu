/*
 * KData.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef KDATA_SERIALIZATION_H_
#define KDATA_SERIALIZATION_H_

#include "../config.h"
#include "../KData.h"

#if HKU_SUPPORT_SERIALIZATION
#include "Stock_serialization.h"
#include "KQuery_serialization.h"

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const hku::KData& kdata, unsigned int version) {
    hku::Stock stock = kdata.getStock();
    hku::KQuery query = kdata.getQuery();
    ar& BOOST_SERIALIZATION_NVP(stock);
    ar& BOOST_SERIALIZATION_NVP(query);
}

template <class Archive>
void load(Archive& ar, hku::KData& kdata, unsigned int version) {
    hku::Stock stock;
    hku::KQuery query;
    ar& BOOST_SERIALIZATION_NVP(stock);
    ar& BOOST_SERIALIZATION_NVP(query);
    kdata = stock.isNull() ? hku::KData() : hku::KData(stock, query);
}

}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::KData)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* KDATA_SERIALIZATION_H_ */
