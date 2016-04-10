/*
 * build_in.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#ifndef TRADE_MANAGE_BUILD_IN_H_
#define TRADE_MANAGE_BUILD_IN_H_

#include "TradeManager.h"

#define FixedA_TC crtFixedATC
#define Zero_TC crtZeroTC

namespace hku {

/**
 * 创建交易管理模块，管理帐户的交易记录及资金使用情况
 * @details 考虑的移滑价差需要使用当日的最高或最低价，所以不在该模块内进行处理
 * @ingroup TradeManagerClass
 * @param datetime 账户建立日期, 默认1990-1-1
 * @param initcash 初始现金，默认100000
 * @param costfunc 交易成本算法,默认零成本算法
 * @param name 账户名称，默认“SYS”
 * @see TradeManager
 */
TradeManagerPtr HKU_API crtTM(
        const Datetime& datetime = Datetime(199001010000LL),
        price_t initcash = 100000.0,
        const TradeCostPtr& costfunc = crtZeroTC(),
        const string& name = "SYS");

/**
 * 沪深A股交易成本算法,计算每次买入或卖出的成本
 * @details
 * <pre>
 * 计算规则为：
 *   1）上证交易所
 *      买入：佣金＋过户费
 *      卖出：佣金＋过户费＋印花税
 *   2）深证交易所：
 *      买入：佣金
 *      卖出：佣金＋印花税
 *   其中：佣金和过户费均有最低值，当前佣金比例为千分之1.8（最低5元），印花税为千分之一
 *         上证过户费为交易数量的千分之一，不足1元，按一元计
 * </pre>
 *
 * @param commission 佣金比例，默认千分之1.8，即0.0018
 * @param lowestCommission 最低佣金值，默认5元
 * @param stamptax 印花税，默认千分之一，即0.001
 * @param transferfee 过户费，默认每股千分之一，即0.001
 * @param lowestTransferfee 最低过户费，默认1元
 * @see FixedATradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API crtFixedATC(
        price_t commission = 0.0018,
        price_t lowestCommission = 5.0,
        price_t stamptax = 0.001,
        price_t transferfee = 0.001,
        price_t lowestTransferfee = 1.0);

/*
 * 创建零成本算法实例
 * @see ZeroTradeCost
 * @ingroup TradeCost
 */
//由于TradeManager的构建参数中默认使用了crtZeroTC，所以将其挪入单独的crtZeroTC.h中
//TradeCostPtr HKU_API crtZeroTC();

/* 仅用于测试 */
TradeCostPtr HKU_API TestStub_TC();

} /* namespace hku */

#endif /* TRADE_MANAGE_BUILD_IN_H_ */
