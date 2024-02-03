/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-03
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/trade_sys/environment/EnvironmentBase.h"

namespace hku {

/**
 * 布尔信号指标市场环境
 * @param ind bool类型的指标，指标中相应位置>0则代表市场有效，否则无效
 * @param market 指定的市场，用于获取相应的交易日历
 * @return
 */
EVPtr HKU_API EV_Bool(const Indicator& ind, const string& market = "SH");

}