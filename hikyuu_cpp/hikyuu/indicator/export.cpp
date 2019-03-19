/*
 * export.cpp
 *
 *  Created on: 2013-4-27
 *      Author: fasiondog
 */

#include "../config.h"

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

#include "imp/Ama.h"
#include "imp/Atr.h"
#include "imp/Diff.h"
#include "imp/Ema.h"
#include "imp/HighLine.h"
#include "imp/LowLine.h"
#include "imp/IKData.h"
#include "imp/IPriceList.h"
#include "imp/Sma.h"
#include "imp/Macd.h"
#include "imp/RightShift.h"
#include "imp/SaftyLoss.h"
#include "imp/StdDeviation.h"
#include "imp/Vigor.h"
#include "imp/Weave.h"
#include "imp/ConstantValue.h"

BOOST_CLASS_EXPORT(hku::Ama)
BOOST_CLASS_EXPORT(hku::Atr)
BOOST_CLASS_EXPORT(hku::Diff)
BOOST_CLASS_EXPORT(hku::Ema)
BOOST_CLASS_EXPORT(hku::HighLine)
BOOST_CLASS_EXPORT(hku::IKData)
BOOST_CLASS_EXPORT(hku::IPriceList)
BOOST_CLASS_EXPORT(hku::LowLine)
BOOST_CLASS_EXPORT(hku::Macd)
BOOST_CLASS_EXPORT(hku::RightShift)
BOOST_CLASS_EXPORT(hku::SaftyLoss)
BOOST_CLASS_EXPORT(hku::Sma)
BOOST_CLASS_EXPORT(hku::StdDeviation)
BOOST_CLASS_EXPORT(hku::Vigor)
BOOST_CLASS_EXPORT(hku::Weave)
BOOST_CLASS_EXPORT(hku::ConstantValue)

#endif /* HKU_SUPPORT_SERIALIZATION */

