/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-01
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/*
 * 根据上下文或输入参数获取10年期中国国债收益率
 * 1. 上下文日期优先
 * 2. 如果时间早于不存在国债数据的情况下，使用默认值替代
 * 3. 时间如果大于已有国债数据，则取最后的国债数据
 */
class IZhBond10 : public IndicatorImp {
    INDICATOR_IMP(IZhBond10)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IZhBond10();
    explicit IZhBond10(const DatetimeList& dates, double default_val = 4.0);
    virtual ~IZhBond10();
};

}  // namespace hku
