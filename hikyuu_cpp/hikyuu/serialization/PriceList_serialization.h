/*
 * PriceList_serialization.h
 *
 *  Created on: 2019-5-26
 *      Author: fasiondog
 */

#pragma once
#ifndef PRICELIST_SERIALIZATION_H_
#define PRICELIST_SERIALIZATION_H_

#include "../config.h"
#include "../DataType.h"

#if HKU_SUPPORT_SERIALIZATION
#if HKU_SUPPORT_XML_ARCHIVE

namespace boost {
namespace serialization {
template <class Archive>
void save(Archive& ar, hku::PriceList& values, unsigned int version) {
    size_t count = values.size();
    unsigned int item_version = 0;
    ar& BOOST_SERIALIZATION_NVP(count);
    ar& BOOST_SERIALIZATION_NVP(item_version);
    for (size_t i = 0; i < count; i++) {
        if (std::isnan(values[i])) {
            ar& boost::serialization::make_nvp("item", "nan");
        } else if (std::isinf(values[i])) {
            ar& boost::serialization::make_nvp("item", values[i] > 0 ? "+inf" : "-inf");
        } else {
            ar& boost::serialization::make_nvp("item", values[i]);
        }
    }
}

template <class Archive>
void load(Archive& ar, hku::PriceList& values, unsigned int version) {
    size_t count = 0;
    unsigned int item_version = 0;
    ar& BOOST_SERIALIZATION_NVP(count);
    ar& BOOST_SERIALIZATION_NVP(item_version);
    values.resize(count);
    for (size_t i = 0; i < count; i++) {
        std::string vstr;
        ar >> boost::serialization::make_nvp("item", vstr);
        if (vstr == "nan") {
            values[i] = std::numeric_limits<double>::quiet_NaN();
        } else if (vstr == "+inf") {
            values[i] = std::numeric_limits<double>::infinity();
        } else if (vstr == "-inf") {
            values[i] = 0.0 - std::numeric_limits<double>::infinity();
        } else {
            values[i] = std::atof(vstr.c_str());
        }
    }
}
}  // namespace serialization
}  // namespace boost

BOOST_SERIALIZATION_SPLIT_FREE(hku::PriceList)

#endif /* HKU_SUPPORT_XML_ARCHIVE */
#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* PRICELIST_SERIALIZATION_H_ */
