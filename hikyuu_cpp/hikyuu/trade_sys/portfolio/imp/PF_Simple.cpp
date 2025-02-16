/*
 * PF_Simple.cpp
 *
 *  Created on: 2018年1月13日
 *      Author: fasiondog
 */

#include "../crt/PF_Simple.h"

namespace hku {

PortfolioPtr HKU_API PF_Simple(const TMPtr& tm, const SEPtr& st, const AFPtr& af, int adjust_cycle,
                               const string& adjust_mode, bool delay_to_trading_day) {
    auto ret = make_shared<Portfolio>(tm, st, af);
    ret->setParam<int>("adjust_cycle", adjust_cycle);
    ret->setParam<string>("adjust_mode", adjust_mode);
    ret->setParam<bool>("delay_to_trading_day", delay_to_trading_day);
    return ret;
}

} /* namespace hku */
