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
    USER_TRY_DELETE_ADMIN,          // 试图删除管理员账号
    USER_NO_RIGHT,                  // 无权限
    USER_INVALID_NAME_OR_PASSWORD,  // 用户名或密码非法
};

enum TradeErrorCode {
    TD_ACCOUNT_REPETITION = 31000,
    TD_ACCOUNT_NOT_EXIST,
    TD_ACCOUNT_INVALD_TYPE,
};

}  // namespace hku