/*
 * LOG.h
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_LOG_H_
#define INDICATOR_CRT_LOG_H_

#include "CVAL.h"

namespace hku {

/**
 * 以10为底的对数
 * 用法：LOG(X)取得X的对数
 * @ingroup Indicator
 */
Indicator HKU_API LOG();
Indicator LOG(Indicator::value_t);
Indicator LOG(const Indicator& ind);

inline Indicator LOG(const Indicator& ind) {
    return LOG()(ind);
}

inline Indicator LOG(Indicator::value_t val) {
    return LOG(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_LOG_H_ */
