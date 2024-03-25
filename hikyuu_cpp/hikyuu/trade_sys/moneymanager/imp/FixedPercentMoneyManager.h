/*
 * PercentRiskMoneyManager.h
 *
 *  Created on: 2015年4月4日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_FIXEDPERCENTMONEYMANAGER_H_
#define TRADE_SYS_MONEYMANAGER_IMP_FIXEDPERCENTMONEYMANAGER_H_

#include "../MoneyManagerBase.h"

namespace hku {

/*
 * 百分比风险模型
 * 参见：《通往财务自由之路》2008年6月 机械工业出版社 范 K.撒普（Van K. Tharp.）P312
 * 公式：P（头寸规模）＝ C（总风险）/ R（每股的风险） ［这里C现金为总风险］
 * 参数：percent：每笔交易总风险占总资产的百分比，如0.02表示总资产的2%
 */
class HKU_API FixedPercentMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(FixedPercentMoneyManager)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedPercentMoneyManager();
    virtual ~FixedPercentMoneyManager();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_FIXEDPERCENTMONEYMANAGER_H_ */
