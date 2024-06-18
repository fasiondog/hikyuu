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
    switch (part) {
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
        case PART_PORTFOLIO:
            return "PF";
        default:
            return "--";
    }
}

SystemPart HKU_API getSystemPartEnum(const string& arg) {
    string name(arg);
    to_upper(name);
    HKU_IF_RETURN("EV" == name, PART_ENVIRONMENT);
    HKU_IF_RETURN("CN" == name, PART_CONDITION);
    HKU_IF_RETURN("SG" == name, PART_SIGNAL);
    HKU_IF_RETURN("ST" == name, PART_STOPLOSS);
    HKU_IF_RETURN("TP" == name, PART_TAKEPROFIT);
    HKU_IF_RETURN("PG" == name, PART_PROFITGOAL);
    HKU_IF_RETURN("SP" == name, PART_SLIPPAGE);
    HKU_IF_RETURN("MM" == name, PART_MONEYMANAGER);
    HKU_IF_RETURN("AF" == name, PART_ALLOCATEFUNDS);
    HKU_IF_RETURN("PF" == name, PART_PORTFOLIO);
    return PART_INVALID;
}

} /* namespace hku */
