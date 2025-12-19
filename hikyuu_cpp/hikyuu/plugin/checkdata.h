/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"
#include "interface/CheckDataPluginInterface.h"

namespace hku {

std::pair<std::string, vector<std::string>> HKU_API checkData(const StockList& stock_list,
                                                              const Datetime& start_date,
                                                              const Datetime& end_date,
                                                              const KQuery::KType& check_ktype);

}