/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-13
 *     Author: fasiondog
 */

#pragma once

namespace hku {

enum RestErrorCode {
    REST_INVALID_VALUE = 10000,
};

enum UserErrorCode {
    USER_NAME_REPETITION = 20000,
};

enum TradeErrorCode {
    TD_ACCOUNT_REPETITION = 30000,
    TD_ACCOUNT_INVALD_TYPE,
};

}  // namespace hku