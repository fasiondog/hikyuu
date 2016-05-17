/*
 * export.cpp
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */

#include "../../config.h"

#if HKU_SUPPORT_SERIALIZATION

#if HKU_SUPPORT_XML_ARCHIVE
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif /* HKU_SUPPORT_XML_ARCHIVE */

#if HKU_SUPPORT_TEXT_ARCHIVE
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif /* HKU_SUPPORT_TEXT_ARCHIVE */

#if HKU_SUPPORT_BINARY_ARCHIVE
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif /* HKU_SUPPORT_BINARY_ARCHIVE */

#include <boost/serialization/export.hpp>

#include "imp/TwoLineEnvironment.h"

BOOST_CLASS_EXPORT(hku::TwoLineEnvironment)

#endif /* HKU_SUPPORT_SERIALIZATION */


