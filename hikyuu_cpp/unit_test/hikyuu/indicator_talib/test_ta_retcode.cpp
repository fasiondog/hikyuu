/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-28
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/indicator_talib/imp/ta_retcode.h>

using namespace hku;

/**
 * @defgroup test_indicator_talib_retcode test_indicator_talib_retcode
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

TEST_CASE("test_checkTARetCode") {
    /** @arg TA_SUCCESS does not throw */
    CHECK_NOTHROW(checkTARetCode(TA_SUCCESS, "TA_MA"));

    /** @arg a TA-Lib failure throws and the message carries the indicator and error info */
    try {
        checkTARetCode(TA_BAD_PARAM, "TA_MA");
        FAIL("checkTARetCode should throw on a TA-Lib error code");
    } catch (const hku::exception& e) {
        std::string msg = e.what();
        CHECK(msg.find("TA_MA") != std::string::npos);
        CHECK(msg.find("TA_BAD_PARAM") != std::string::npos);
        CHECK(msg.find(std::to_string(int(TA_BAD_PARAM))) != std::string::npos);
    } catch (...) {
        FAIL("checkTARetCode should throw hku::exception");
    }
}

/** @} */
