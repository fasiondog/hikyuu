/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-09-01
 *      Author: fasiondog
 */

#pragma once

#include "../MarginRatioBase.h"

namespace hku {

class HKU_API FixedMarginRatio : public MarginRatioBase {
public:
    FixedMarginRatio() : FixedMarginRatio(1.0, 1.0) {}
    FixedMarginRatio(double initRatio, double maintainRatio)
    : FixedMarginRatio(MarginRecord(initRatio, maintainRatio)) {}
    FixedMarginRatio(const MarginRecord& mr);
    virtual ~FixedMarginRatio();

    virtual MarginRecord getMarginRatio(Datetime datetime, const Stock& stk) const override;
    virtual MarginRatioPtr _clone() override;

private:
    MarginRecord m_margin;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MarginRatioBase);
        ar& BOOST_SERIALIZATION_NVP(m_margin);
    }
#endif
};

}  // namespace hku