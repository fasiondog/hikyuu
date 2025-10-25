/*
 * SP_LogNormal.h
 *
 *  Created on: 2025年10月25日
 *      Author: fasiondog
 */

#pragma once

#include "../SlippageBase.h"

namespace hku {

/**
 * 对数正态分布随机价格移滑价差算法, 买入和卖出操作是基于对数正态分布的随机价格偏移
 * @param mean 对数正态分布的均值参数
 * @param stddev 对数正态分布的标准差参数
 * @return 滑点对象指针
 */
SlippagePtr HKU_API SP_LogNormal(double mean = 0.0, double stddev = 0.05);

} /* namespace hku */