/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-13
 *     Author: fasiondog
 */

#include "TradeManagerBase.h"

namespace hku {

TradeManagerBase::TradeManagerBase() : m_broker_last_datetime(Datetime::now()) {}

}  // namespace hku