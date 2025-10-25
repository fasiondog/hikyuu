/*
 * SP_Normal.h
 *
 *  Created on: 2025年10月25日
 *      Author: fasiondog
 */

#pragma once

#include "../SlippageBase.h"

namespace hku {

/**
 * 正态分布随机价格移滑价差算法, 买入和卖出操作是基于正态分布的随机价格偏移
 * @param mean 正态分布均值
 * @param stddev 正态分布标准差
 * @return 滑点对象指针
 */
SlippagePtr HKU_API SP_Normal(double mean = 0.0, double stddev = 0.05);

} /* namespace hku */