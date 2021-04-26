/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-13
 *     Author: fasiondog
 */

#pragma once

namespace hku {

enum RestErrorCode {
    INVALID_VALUE = 10000,  // 无效值，主要用于数据库字段非法值判断
    MISS_TOKEN,
    UNAUTHORIZED,
    AUTHORIZE_EXPIRED,  // 鉴权过期
    MISS_PARAMETER,  // 缺失参数，参数不能为空 必填参数不能为空（各个业务接口返回各个接口的参数）
    WRONG_PARAMETER,  // 参数值填写错误（各个业务接口返回各个接口的参数）
    WRONG_PARAMETER_TYPE  // 参数类型错误（各个业务接口返回各个接口的参数）
};

enum UserErrorCode {
    USER_NAME_REPETITION = 20000,
    USER_NOT_EXIST,
    USER_WRONG_PASSWORD,
};

enum TradeErrorCode {
    TD_ACCOUNT_REPETITION = 30000,
    TD_ACCOUNT_INVALD_TYPE,
};

}  // namespace hku