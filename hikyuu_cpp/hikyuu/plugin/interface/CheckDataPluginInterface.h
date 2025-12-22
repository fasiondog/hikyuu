/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/Stock.h"
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class CheckDataPluginInterface : public PluginBase {
public:
    CheckDataPluginInterface() = default;
    virtual ~CheckDataPluginInterface() = default;

    virtual std::pair<std::string, vector<std::string>> checkData(
      const StockList& stock_list, const Datetime& start_date, const Datetime& end_date,
      const KQuery::KType& check_ktype) = 0;
};

}  // namespace hku
