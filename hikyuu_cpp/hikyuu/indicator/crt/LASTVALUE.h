/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-04
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LASTVALUE_H_
#define INDICATOR_CRT_LASTVALUE_H_

#include "../Indicator.h"

namespace hku {

/**
 * 取输入指标最后值为常数，即结果中所有值均为输入指标的最后值，谨慎使用
 * @note 等同于通达信 CONST 指标, 由于 Windows 下 CONST 命名冲突，取名 LASTVALUE
 * @ingroup Indicator
 */
Indicator HKU_API LASTVALUE(bool ignore_discard = false);

inline Indicator LASTVALUE(const Indicator& ind, bool ignore_discard = false) {
    return LASTVALUE(ignore_discard)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_LASTVALUE_H_ */
