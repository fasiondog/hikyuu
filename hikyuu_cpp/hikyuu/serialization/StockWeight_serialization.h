/*
 * StockWeight.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef STOCKWEIGHT_SERIALIZATION_H_
#define STOCKWEIGHT_SERIALIZATION_H_

#include "../config.h"
#include "../StockWeight.h"

#if HKU_SUPPORT_SERIALIZATION

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, const hku::StockWeight& record, unsigned int version) {
    hku::uint64_t datetime = record.datetime().number();
    hku::price_t countAsGift = record.countAsGift();
    hku::price_t countForSell = record.countForSell();
    hku::price_t priceForSell = record.priceForSell();
    hku::price_t bonus = record.bonus();
    hku::price_t increasement = record.increasement();
    hku::price_t totalCount = record.totalCount();
    hku::price_t freeCount = record.freeCount();
    ar& BOOST_SERIALIZATION_NVP(datetime);
    ar& BOOST_SERIALIZATION_NVP(countAsGift);
    ar& BOOST_SERIALIZATION_NVP(countForSell);
    ar& BOOST_SERIALIZATION_NVP(priceForSell);
    ar& BOOST_SERIALIZATION_NVP(bonus);
    ar& BOOST_SERIALIZATION_NVP(increasement);
    ar& BOOST_SERIALIZATION_NVP(totalCount);
    ar& BOOST_SERIALIZATION_NVP(freeCount);
}

template <class Archive>
void load(Archive& ar, hku::StockWeight& record, unsigned int version) {
    hku::uint64_t datetime;
    hku::price_t countAsGift, countForSell, priceForSell, bonus;
    hku::price_t increasement, totalCount, freeCount;
    ar& BOOST_SERIALIZATION_NVP(datetime);
    ar& BOOST_SERIALIZATION_NVP(countAsGift);
    ar& BOOST_SERIALIZATION_NVP(countForSell);
    ar& BOOST_SERIALIZATION_NVP(priceForSell);
    ar& BOOST_SERIALIZATION_NVP(bonus);
    ar& BOOST_SERIALIZATION_NVP(increasement);
    ar& BOOST_SERIALIZATION_NVP(totalCount);
    ar& BOOST_SERIALIZATION_NVP(freeCount);
    record = hku::StockWeight(hku::Datetime(datetime), countAsGift, countForSell, priceForSell,
                              bonus, increasement, totalCount, freeCount);
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::StockWeight)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* STOCKWEIGHT_SERIALIZATION_H_ */
