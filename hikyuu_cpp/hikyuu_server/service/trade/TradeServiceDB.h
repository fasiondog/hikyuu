/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-08
 *     Author: fasiondog
 */

#pragma once

#include <hikyuu/utilities/db_connect/DBConnect.h>
#include <hikyuu/utilities/Parameter.h>

namespace hku {

DBConnectPtr getDBConnect();

void initTradeServiceDB(const Parameter& params);

}  // namespace hku