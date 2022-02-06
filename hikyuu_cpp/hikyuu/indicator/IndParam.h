/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-02
 *      Author: fasiondog
 */

#pragma once

#include "IndicatorImp.h"

namespace hku {

class HKU_API Indicator;
class HKU_API IndicatorImp;

/**
 * @brief 动态指标参数，用于作为其他指标的参数
 * @ingroup Indicator
 */
class HKU_API IndParam {
    HKU_API friend std::ostream& operator<<(std::ostream&, const IndParam&);

public:
    IndParam();
    explicit IndParam(const IndicatorImpPtr& ind);
    explicit IndParam(const Indicator& ind);

    IndicatorImpPtr getImp() const {
        return m_ind;
    }

    Indicator get() const;

private:
    IndicatorImpPtr m_ind;
};

}  // namespace hku