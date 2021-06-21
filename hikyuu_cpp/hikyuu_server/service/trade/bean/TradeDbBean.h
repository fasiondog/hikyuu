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
};

}  // namespace hku