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
 * Get the yield of the 10-year Chinese treasury bond according to the context or the input
 * parameters
 * 1. The context date has priority
 * 2. If the time is earlier than the case where no treasury bond data exists, the default value is
 * used instead
 * 3. If the time is later than the existing treasury bond data, the last treasury bond data is used
 */
class IZhBond10 : public IndicatorImp {
    INDICATOR_IMP(IZhBond10)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IZhBond10();
    explicit IZhBond10(const DatetimeList& dates, double default_val = 4.0);
    virtual ~IZhBond10() override;
};

}  // namespace hku
