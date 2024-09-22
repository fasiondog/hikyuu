/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-16
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/condition/ConditionBase.h"

namespace hku {

class HKU_API AndCondition : public ConditionBase {
public:
    AndCondition();
    AndCondition(const ConditionPtr& cond1, const ConditionPtr& cond2);
    virtual ~AndCondition();

    virtual void _calculate() override;
    virtual void _reset() override;
    virtual ConditionPtr _clone() override;

private:
    ConditionPtr m_cond1;
    ConditionPtr m_cond2;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
        ar& BOOST_SERIALIZATION_NVP(m_cond1);
        ar& BOOST_SERIALIZATION_NVP(m_cond2);
    }
#endif
};

}  // namespace hku