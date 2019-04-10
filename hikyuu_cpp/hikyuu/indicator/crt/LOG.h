/*
 * LOG.h
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

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
Indicator LOG(price_t);
Indicator LOG(const Indicator& ind);

inline Indicator LOG(const Indicator& ind) {
    return LOG()(ind);
}

Indicator LOG(price_t val) {
    return LOG(CVAL(val));
}

} /* namespace */

#endif /* INDICATOR_CRT_LOG_H_ */
