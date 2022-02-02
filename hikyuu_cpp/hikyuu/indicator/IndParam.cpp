/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-02
 *      Author: fasiondog
 */

#include "Indicator.h"
#include "IndParam.h"

namespace hku {

IndParam::IndParam() {}

IndParam::IndParam(const IndicatorImpPtr& ind) : m_ind(ind){};

}