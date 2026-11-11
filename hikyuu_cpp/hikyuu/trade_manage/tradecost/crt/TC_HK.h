/*
 * TC_HK.h
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_CRT_TC_HK_H_
#define TRADE_MANAGE_CRT_TC_HK_H_

#include "../TradeCostBase.h"
#include "../imp/FixedHKTradeCost.h"

namespace hku {

/**
 * 创建港股交易成本算法：买卖双边收取佣金（含最低佣金）、印花税、交易费及征费，
 * 可附加每笔固定费用
 * @param commission 佣金比例（双边，按成交金额）
 * @param lowestCommission 最低佣金值（双边，按笔）
 * @param stamptax 印花税率（双边收取，2023 年 11 月 17 日起为 0.1%）
 * @param tradefee 交易费及征费合计比例（双边，联交所 0.0027% + 证监会 0.0027% +
 *        财汇局 0.00015%，默认 0.0000555）
 * @param perTrade 每笔固定费用（双边，如经纪商代收的结算费）
 * @return TradeCostPtr
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_HK(price_t commission = 0.00025, price_t lowestCommission = 3.0,
                           price_t stamptax = 0.001, price_t tradefee = 0.0000555,
                           price_t perTrade = 0.0);

void registerPart_TC_HK();

}  // namespace hku

#endif /* TRADE_MANAGE_CRT_TC_HK_H_ */
