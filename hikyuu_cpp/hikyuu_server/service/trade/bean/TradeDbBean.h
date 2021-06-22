/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-06-21
 *     Author: fasiondog
 */

#pragma once

#include "db/db.h"
#include "../model/XueqiuAccountModel.h"

namespace hku {

class TradeDbBean {
public:
    TradeDbBean() = delete;

    static DBConnectPtr getConnect() {
        return DB::getConnect();
    }

    /** 查询所有雪球模拟账户信息 */
    static std::vector<XueqiuAccountModel> queryAllXueqiuAccount();

    /**
     * 获取指定的雪球模拟账户信息
     * @param id 指定的账户id
     * @param [out] out_mo 输出雪球模拟账户列表
     * @return true 成功 | false 失败
     */
    static bool queryXueqiuAccountsByTdId(int64_t id, XueqiuAccountModel& out_mo);
};

}  // namespace hku