/*
 * dboule_serialization.h
 *
 *  Created on: 2019-5-26
 *      Author: fasiondog
 */

#ifndef DOUBLE_SERIALIZATION_H_
#define DOUBLE_SERIALIZATION_H_

#include "../config.h"

#if HKU_SUPPORT_SERIALIZATION
#if HKU_SUPPORT_XML_ARCHIVE
#include <limits>
#include <cmath>
#include <sstream>
#include <iostream>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

template <typename T>
class StreamSelector {
 public:
  static boost::archive::xml_iarchive&
  stream(boost::archive::xml_iarchive& ia,
         const boost::serialization::nvp<T>& nvp)
  {
    ia.operator >>(nvp);
    return ia;
  }

  static boost::archive::xml_oarchive&
  stream(boost::archive::xml_oarchive& oa,
         const boost::serialization::nvp<T>& nvp)
  {
    oa.operator <<(nvp);
    return oa;
  }
};

template <>
class StreamSelector<double> {
 public:
  constexpr static double nan = std::numeric_limits<double>::quiet_NaN();
  constexpr static double inf = std::numeric_limits<double>::infinity();
  constexpr static const char* nanCStr = "nan";
  constexpr static const char* infCStr = "+inf";
  constexpr static const char* pinfCStr = "-inf";

  static boost::archive::xml_iarchive&
  stream(boost::archive::xml_iarchive& ia,
         const boost::serialization::nvp<double>& nvp)
  {
    std::string iStr;
    ia >>  boost::serialization::make_nvp(nvp.name(), iStr);
    if(iStr == nanCStr) {
      nvp.value() = nan;
    } else if (iStr == infCStr) {
      nvp.value() = inf;
    } else if (iStr == pinfCStr) {
      nvp.value() = -inf;
    } else {
      nvp.value() = std::stod(iStr);
    }

    return ia;
  }

  static boost::archive::xml_oarchive&
  stream(boost::archive::xml_oarchive& oa,
         const boost::serialization::nvp<double>& nvp)
  {
    if(std::isnan(nvp.value())) {
      std::string nanStr = nanCStr;
      oa << boost::serialization::make_nvp(nvp.name(), nanStr);
    } else if (std::isinf(nvp.value())) {
      std::string infStr = nvp.value() > 0 ? infCStr : pinfCStr;
      oa << boost::serialization::make_nvp(nvp.name(), infStr);
    } else {
      std::stringstream oStrm;
      oStrm << std::setprecision(std::numeric_limits<double>::digits10 + 1)
            << nvp.value();
      std::string oStr = oStrm.str();
      oa << boost::serialization::make_nvp(nvp.name(), oStr);
    }
    return oa;
  }
};

// C++ I/O Operators
template <typename T>
boost::archive::xml_iarchive&
operator >>(boost::archive::xml_iarchive& ia,
            const boost::serialization::nvp<T>& nvp)
{
  return StreamSelector<T>::stream(ia, nvp);
}

template <typename T>
boost::archive::xml_oarchive&
operator <<(boost::archive::xml_oarchive& oa,
            const boost::serialization::nvp<T>& nvp)
{
  return StreamSelector<T>::stream(oa, nvp);
}

// Boost Archive Style I/O operator
template <typename T>
boost::archive::xml_iarchive&
operator &(boost::archive::xml_iarchive& ia,
           const boost::serialization::nvp<T>& nvp)
{
  return StreamSelector<T>::stream(ia, nvp);
}

template <typename T>
boost::archive::xml_oarchive&
operator &(boost::archive::xml_oarchive& oa,
           const boost::serialization::nvp<T>& nvp)
{
  return StreamSelector<T>::stream(oa, nvp);
}

// Function overload I/O
template <typename T>
boost::archive::xml_iarchive&
stream(boost::archive::xml_iarchive& ia,
       const boost::serialization::nvp<T>& nvp)
{
  return StreamSelector<T>::stream(ia, nvp);
}

template <typename T>
boost::archive::xml_oarchive&
stream(boost::archive::xml_oarchive& oa,
       const boost::serialization::nvp<T>& nvp)
{
  return StreamSelector<T>::stream(oa, nvp);
}

#endif /* HKU_SUPPORT_XML_ARCHIVE */
#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* DOUBLE_SERIALIZATION_H_ */
