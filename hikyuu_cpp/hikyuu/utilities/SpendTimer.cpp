/*
 * SpendTimer.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-8-3
 *      Author: fasiondog
 */

#include "SpendTimer.h"

namespace hku {

#if !HIKYUU_CLOSE_SPEND_TIME
bool SpendTimer::m_closed = false;
#endif

} /* namespace hku */