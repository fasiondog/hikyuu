/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-19
 *      Author: fasiondog
 */

#pragma once

#include "../SelectorBase.h"

namespace hku {

SEPtr HKU_API SE_Signal();

SEPtr HKU_API SE_Signal(const StockList& stock_list, const SystemPtr& sys);

}