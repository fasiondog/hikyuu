/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2026-09-28
 *      Author: fasiondog
 */

#pragma once

#include <ta-lib/ta_common.h>
#include <ta-lib/ta_func.h>

#include <string>

#include "hikyuu/utilities/Log.h"

namespace hku {

/**
 * Check the return code of a TA-Lib function and throw with an informative message on failure,
 * so an uninitialized outBegIdx/outNbElement never triggers a misleading assertion afterwards
 */
inline void checkTARetCode(TA_RetCode retCode, const string& indicator) {
    HKU_IF_RETURN(retCode == TA_SUCCESS, void());
    TA_RetCodeInfo info;
    TA_SetRetCodeInfo(retCode, &info);
    HKU_THROW("TA-Lib {} failed! retCode: {} ({}): {}", indicator, int(retCode), info.enumStr,
              info.infoStr);
}

}  // namespace hku
