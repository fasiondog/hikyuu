/*
 * MarketInfo.h
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

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
void save(Archive & ar, const hku::MarketInfo& record, unsigned int version) {
    hku::string market = record.market();
    hku::string name = HKU_GB_TO_UTF8(record.name());
    hku::string description = HKU_GB_TO_UTF8(record.description());
    hku::string code = record.code();
    hku::hku_uint64 lastDate = record.lastDate().number();
    ar & BOOST_SERIALIZATION_NVP(market);
    ar & BOOST_SERIALIZATION_NVP(name);
    ar & BOOST_SERIALIZATION_NVP(description);
    ar & BOOST_SERIALIZATION_NVP(code);
    ar & BOOST_SERIALIZATION_NVP(lastDate);
}

template<class Archive>
void load(Archive & ar, hku::MarketInfo& record, unsigned int version) {
    hku::string market, name, description, code;
    hku::hku_uint64 lastDate;
    ar & BOOST_SERIALIZATION_NVP(market);
    ar & BOOST_SERIALIZATION_NVP(name);
    ar & BOOST_SERIALIZATION_NVP(description);
    ar & BOOST_SERIALIZATION_NVP(code);
    ar & BOOST_SERIALIZATION_NVP(lastDate);
    record = hku::MarketInfo(market, HKU_UTF8_TO_GB(name),
                             HKU_UTF8_TO_GB(description),
                             code, hku::Datetime(lastDate));
}
}} /* namespace boost::serailization */

BOOST_SERIALIZATION_SPLIT_FREE(hku::MarketInfo)

#endif /* HKU_SUPPORT_SERIALIZATION */


#endif /* MARKETINFO_SERIALIZATION_H_ */
