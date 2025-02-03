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
 * 动态移动平均
 * 用法：DMA(X,A),求X的动态移动平均。
 * 算法：若Y=DMA(X,A) 则 Y=A*X+(1-A)*Y',其中Y'表示上一周期Y值。
 * 例如：DMA(CLOSE,VOL/CAPITAL)表示求以换手率作平滑因子的平均价
 */
class IDma : public Indicator2InImp {
    INDICATOR2IN_IMP(IDma)
    INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IDma();
    explicit IDma(const Indicator& ref_a, bool fill_null);
    virtual ~IDma();
};

}  // namespace hku