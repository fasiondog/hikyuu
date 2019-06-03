/*
 * ADVANCE.h
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-6-1
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_ADVANCE_H_
#define INDICATOR_CRT_ADVANCE_H_

#include "../Indicator.h"
#include "../../StockTypeInfo.h"

namespace hku {

/**
 * 上涨家数
 * @ingroup Indicator
 */
Indicator HKU_API ADVANCE(const KQuery& query = KQueryByIndex(-100), 
                          const string& market="SH", int stk_type=STOCKTYPE_A, 
                          bool ignore_context=false);


}

#endif /* INDICATOR_CRT_ADVANCE_H_ */
