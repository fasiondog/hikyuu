/*
 * TRG.h
 *
 *  Created on: 2019年3月6日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_TR_H_
#define INDICATOR_CRT_TR_H_

#include "../Indicator.h"

namespace hku {

/**
 * @brief 真实波动幅度（TR）
 * @details
 * <pre>
 * 真实波动幅度（TR）是以下三个值中的最大值：
 *  1. 当前周期最高价（H）与最低价（L）之差
 *  2. 当前周期最高价与前一周期收盘价（PC）之差的绝对值
 *  3. 当前周期最低价与前一周期收盘价之差的绝对值
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API TR();
Indicator HKU_API TR(const KData&);

}  // namespace hku

#endif /* INDICATOR_CRT_TR_H_ */
