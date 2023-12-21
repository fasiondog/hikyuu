/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/trade_sys/condition/ConditionBase.h"

namespace hku {

/**
 * 布尔信号指标系统有效条件
 * @param ind bool类型的指标，指标中相应位置>0则代表系统有效，否则无效
 * @return
 */
CNPtr HKU_API CN_Bool(const Indicator& ind);

} /* namespace hku */