/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#pragma once

#include "../SelectorBase.h"

namespace hku {

class HKU_API OperatorAddSelector : public SelectorBase {
public:
    OperatorAddSelector();
    OperatorAddSelector(const SelectorPtr& se1, const SelectorPtr& se2);
    virtual ~OperatorAddSelector();

    virtual void _reset() override;
    virtual SelectorPtr _clone() override;
    virtual bool isMatchAF(const AFPtr& af) override;
    virtual void _calculate() override;
    virtual SystemWeightList getSelected(Datetime date) override;

private:
    SelectorPtr m_se1;
    SelectorPtr m_se2;

    //============================================
    // 序列化支持
    //============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
        ar& BOOST_SERIALIZATION_NVP(m_se1);
        ar& BOOST_SERIALIZATION_NVP(m_se2);
    }
#endif
};

}  // namespace hku