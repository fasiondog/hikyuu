/*
 * SystemPart.h
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */

#pragma once
#ifndef SYSTEMPART_H_
#define SYSTEMPART_H_

#include "../../DataType.h"

namespace hku {

/**
 * Enumeration definition of the system associated parts (the respective strategies), used to modify
 * the parameters of the related parts
 * @ingroup System
 */
enum SystemPart {
    PART_ENVIRONMENT = 0,  /**< External environment */
    PART_CONDITION = 1,    /**< System precondition */
    PART_SIGNAL = 2,       /**< Signal generator */
    PART_STOPLOSS = 3,     /**< Stop-loss strategy */
    PART_TAKEPROFIT = 4,   /**< Take-profit strategy */
    PART_MONEYMANAGER = 5, /**< Money management strategy */
    PART_PROFITGOAL = 6,   /**< Profit goal strategy */
    PART_SLIPPAGE = 7,     /**< Slippage algorithm */

    PART_ALLOCATEFUNDS = 8, /**< [Deprecated] Asset allocation algorithm (AF has been removed with
                             *   the refactoring; the enum value is kept for the compatibility with
                             *   the old serialized data, do not use it in the new code) */
    PART_PORTFOLIO = 9,     /**< [Deprecated] Portfolio (PF has been removed with the refactoring;
                             *   the enum value is kept for the compatibility with the old
                             *   serialized data, do not use it in the new code) */

    PART_SYSTEM = 10,  /**< Aggregate sub-system (added by the recursive combination refactoring) */
    PART_INVALID = 11, /**< Invalid value (the sentinel, it must be the last enum item) */
};

/**
 * Get the string name of the SystemPart enumeration value
 * @param part the system part enumeration value
 * @return
 * @ingroup System
 */
string HKU_API getSystemPartName(int part);

/**
 * Get the SystemPart enumeration value by the string name
 * @param name the string name
 * @return
 * @ingroup System
 */
SystemPart HKU_API getSystemPartEnum(const string& name);

}  // namespace hku

#endif /* SYSTEMPART_H_ */
