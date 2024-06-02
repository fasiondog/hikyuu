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

    virtual void _addSystem(const SYSPtr& sys) override;
    virtual void _removeAll() override;

protected:
    virtual void bindRealToProto(const SYSPtr& real, const SYSPtr& proto) override {
        m_real_to_proto[real] = proto;
    }

private:
    SelectorPtr m_se1;
    SelectorPtr m_se2;
    std::unordered_set<SYSPtr> m_se1_set;
    std::unordered_set<SYSPtr> m_se2_set;
    std::unordered_map<SYSPtr, SYSPtr> m_real_to_proto;

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