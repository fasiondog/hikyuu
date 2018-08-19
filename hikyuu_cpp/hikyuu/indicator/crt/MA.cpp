/*
 * MA.cpp
 *
 *  Created on: 2015年2月16日
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "MA.h"
#include "SMA.h"
#include "EMA.h"
#include "AMA.h"

namespace hku {

Indicator HKU_API MA(const Indicator& data, int n, const string& type) {
    string str_type(type);
    boost::to_upper(str_type);
    if (str_type == "SMA") {
        return SMA(data, n);
    } else if (str_type == "EMA") {
        return EMA(data, n);
    } else if (str_type == "AMA") {
        return AMA(data, n);
    } else {
        return SMA(data, n);
    }
}

Indicator HKU_API MA(int n, const string& type) {
    string str_type(type);
    boost::to_upper(str_type);
    if (str_type == "SMA") {
        return SMA(n);
    } else if (str_type == "EMA") {
        return EMA(n);
    } else if (str_type == "AMA") {
        return AMA(n);
    } else {
        return SMA(n);
    }
}

} /* namespae hku */


