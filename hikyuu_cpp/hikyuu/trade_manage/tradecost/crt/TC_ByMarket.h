/*
 * TC_ByMarket.h
 *
 *  Created on: 2026-11-12
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_TC_BYMARKET_H_
#define TRADE_MANAGE_CRT_TC_BYMARKET_H_

#include "../TradeCostBase.h"
#include "../imp/ByMarketTradeCost.h"

namespace hku {

/**
 * 创建按市场 / 证券类型路由的交易成本算法：本身不收取任何费用，买卖与借券成本均转交
 * 该证券所属市场（或类型）已注册的成本算法，未注册时使用默认算法
 * @param defaultCost 未注册市场的兜底成本算法，为空时使用零成本算法
 * @return TradeCostPtr，指向 ByMarketTradeCost（调用方可继续 add 注册市场）
 * @note 市场按代码（如 "HK"）匹配，其次按类型（如 "type:12"）匹配；请在回测开始前配置完毕
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_ByMarket(const TradeCostPtr& defaultCost = TradeCostPtr());

void registerPart_TC_ByMarket();

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_TC_BYMARKET_H_ */
