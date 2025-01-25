/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-22
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ICOST_H_
#define INDICATOR_IMP_ICOST_H_

#include "../Indicator.h"

namespace hku {

/* COST(k, X) 表示X%获利盘的价格是多少 */
class ICost : public IndicatorImp {
    INDICATOR_IMP(ICost)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICost();
    ICost(const KData&, double percent);
    virtual ~ICost();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ICOST_H_ */
