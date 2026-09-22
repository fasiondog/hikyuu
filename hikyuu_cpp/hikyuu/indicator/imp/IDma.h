/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-09
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator2InImp.h"

namespace hku {

/*
 * Dynamic moving average
 * Usage: DMA(X,A) gives the dynamic moving average of X.
 * Algorithm: if Y=DMA(X,A) then Y=A*X+(1-A)*Y', where Y' is the Y value of the previous period.
 * For example: DMA(CLOSE,VOL/CAPITAL) gives the average price with the turnover rate as the
 * smoothing factor
 */
class IDma : public Indicator2InImp {
    INDICATOR2IN_IMP(IDma)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IDma();
    explicit IDma(const Indicator& ref_a, bool fill_null);
    virtual ~IDma() override;
    virtual size_t min_increment_start() const override;
};

}  // namespace hku