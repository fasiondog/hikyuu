/*
 * StockTypeInfo.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#ifndef STOCKTYPEINFO_SERIALIZATION_H_
#define STOCKTYPEINFO_SERIALIZATION_H_

#include "../config.h"
#include "../StockTypeInfo.h"
#include "../utilities/util.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>

namespace boost {
namespace serialization {

//防止boost::serialization某些情况不能在独立的命名空间中
#if defined(BOOST_WINDOWS) && (PY_VERSION_HEX < 0x03000000)
    #ifndef HKU_GB_TO_UTF8
    #define HKU_GB_TO_UTF8(s)  hku::GBToUTF8(s)
    #endif

    #ifndef HKU_UTF8_TO_GB
    #define HKU_UTF8_TO_GB(s)  hku::UTF8ToGB(s)
    #endif
#else
    #ifndef HKU_GB_TO_UTF8
    #define HKU_GB_TO_UTF8(s)  (s)
    #endif

    #ifndef HKU_UTF8_TO_GB
    #define HKU_UTF8_TO_GB(s)  (s)
    #endif
#endif

template<class Archive>
void save(Archive & ar, const hku::StockTypeInfo& record, unsigned int version) {
    hku::hku_uint32 type = record.type();
    hku::string description = HKU_GB_TO_UTF8(record.description());
    hku::price_t tick = record.tick();
    hku::price_t tickValue = record.tickValue();
    int precision = record.precision();
    size_t minTradeNumber = record.minTradeNumber();
    size_t maxTradeNumber = record.maxTradeNumber();
    ar & BOOST_SERIALIZATION_NVP(type);
    ar & BOOST_SERIALIZATION_NVP(description);
    ar & BOOST_SERIALIZATION_NVP(tick);
    ar & BOOST_SERIALIZATION_NVP(tickValue);
    ar & BOOST_SERIALIZATION_NVP(precision);
    ar & BOOST_SERIALIZATION_NVP(minTradeNumber);
    ar & BOOST_SERIALIZATION_NVP(maxTradeNumber);
}

template<class Archive>
void load(Archive & ar, hku::StockTypeInfo& record, unsigned int version) {
    hku::hku_uint32 type;
    hku::string description;
    hku::price_t tick, tickValue;
    int precision;
    size_t minTradeNumber, maxTradeNumber;
    ar & BOOST_SERIALIZATION_NVP(type);
    ar & BOOST_SERIALIZATION_NVP(description);
    ar & BOOST_SERIALIZATION_NVP(tick);
    ar & BOOST_SERIALIZATION_NVP(tickValue);
    ar & BOOST_SERIALIZATION_NVP(precision);
    ar & BOOST_SERIALIZATION_NVP(minTradeNumber);
    ar & BOOST_SERIALIZATION_NVP(maxTradeNumber);
    record = hku::StockTypeInfo(type, HKU_UTF8_TO_GB(description),
            tick, tickValue, precision, minTradeNumber, maxTradeNumber);
}
}} /* namespace boost::serailization */

BOOST_SERIALIZATION_SPLIT_FREE(hku::StockTypeInfo)

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* STOCKTYPEINFO_SERIALIZATION_H_ */
