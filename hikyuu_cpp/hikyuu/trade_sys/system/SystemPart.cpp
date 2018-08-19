/*
 * SystemPart.cpp
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "SystemPart.h"

namespace hku {

string HKU_API getSystemPartName(int part) {
    switch(part){
    case PART_ENVIRONMENT:
        return "EV";
    case PART_CONDITION:
        return "CN";
    case PART_SIGNAL:
        return "SG";
    case PART_STOPLOSS:
        return "ST";
    case PART_TAKEPROFIT:
        return "TP";
    case PART_MONEYMANAGER:
        return "MM";
    case PART_PROFITGOAL:
        return "PG";
    case PART_SLIPPAGE:
        return "SP";
    case PART_ALLOCATEFUNDS:
        return "AF";
    default:
        return "--";
    }
}


SystemPart HKU_API getSystemPartEnum(const string& arg) {
    string name(arg);
    boost::to_upper(name);
    if ("EV" == name) {
        return PART_ENVIRONMENT;

    } else if ("CN" == name) {
        return PART_CONDITION;

    } else if ("SG" == name) {
        return PART_SIGNAL;

    } else if ("ST" == name) {
        return PART_STOPLOSS;

    } else if ("TP" == name) {
        return PART_TAKEPROFIT;

    } else if ("PG" == name) {
        return PART_PROFITGOAL;

    } else if ("SP" == name) {
        return PART_SLIPPAGE;

    } else if ("MM" == name) {
        return PART_MONEYMANAGER;

    } else if ("AF" == name) {
        return PART_ALLOCATEFUNDS;

    } else {
        return PART_INVALID;
    }
}

} /* namespace hku */


