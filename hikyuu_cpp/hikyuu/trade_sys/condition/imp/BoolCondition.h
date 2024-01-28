/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "../ConditionBase.h"

namespace hku {

class BoolCondition : public ConditionBase {
public:
    BoolCondition();
    BoolCondition(const Indicator&);
    virtual ~BoolCondition();

    virtual void _calculate() override;
    virtual ConditionPtr _clone() override;

private:
    Indicator m_ind;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
        ar& BOOST_SERIALIZATION_NVP(m_ind);
    }
#endif
};

}  // namespace hku