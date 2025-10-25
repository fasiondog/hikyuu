/*
 * SL_FixedValue.h
 *
 *  Created on: 2016年5月7日
 *      Author: Administrator
 */

#pragma once

#include "../SlippageBase.h"

namespace hku {

/**
 * 均匀分布随机价格移滑价差算法, 买入和卖出操作是价格在[min_value,
 * max_value]范围内的均匀分布随机偏移
 * @param min_value 偏移价格下限
 * @param max_value 偏移价格上限
 * @return
 */
SlippagePtr HKU_API SP_Uniform(double min_value = -0.05, double max_value = 0.05);

} /* namespace hku */
