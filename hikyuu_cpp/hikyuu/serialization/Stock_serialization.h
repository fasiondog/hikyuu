/*
 * Stock_Serialization.h
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */

#ifndef STOCK_SERIALIZATION_H_
#define STOCK_SERIALIZATION_H_

#include "../config.h"
#include "../StockManager.h"
#include "../utilities/util.h"

//===========================================================================
// 以下为Stock的序列化，目前仅实现了在StockManager中管理的Stock的序列化
// 注意，该段代码在namespace hku之外
// TODO 实现非StockManager管理的Stock的序列化
//===========================================================================
#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>

namespace boost {
namespace serialization {
template<class Archive>

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

void save(Archive & ar, const hku::Stock & stock, unsigned int version) {
    hku::string market_code = stock.market_code();
    hku::string name = HKU_GB_TO_UTF8(stock.name());
    ar & BOOST_SERIALIZATION_NVP(market_code);
    ar & BOOST_SERIALIZATION_NVP(name);
}

template<class Archive>
void load(Archive & ar, hku::Stock& stock, unsigned int version) {
    hku::string market_code, name;
    ar & BOOST_SERIALIZATION_NVP(market_code);
    ar & BOOST_SERIALIZATION_NVP(name);
    hku::StockManager& sm = hku::StockManager::instance();
    stock = sm.getStock(market_code);
}
}} /* namespace boost::serailization */

BOOST_SERIALIZATION_SPLIT_FREE(hku::Stock)

#endif /* HKU_SUPPORT_SERIALIZATION */


#endif /* STOCK_SERIALIZATION_H_ */
