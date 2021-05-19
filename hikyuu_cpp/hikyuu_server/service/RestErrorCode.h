/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-13
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpError.h"

namespace hku {

enum UserErrorCode {
    USER_NAME_REPETITION = 30000,
    USER_NOT_EXIST,
    USER_WRONG_PASSWORD,
    USER_NO_RIGHT,          // 无权限
    USER_INVALID_NAME,      // 用户名称非法
    USER_INVALID_PASSWORD,  // 用户密码非法
};

enum TradeErrorCode {
    TD_ACCOUNT_REPETITION = 31000,
    TD_ACCOUNT_INVALD_TYPE,
};

}  // namespace hku