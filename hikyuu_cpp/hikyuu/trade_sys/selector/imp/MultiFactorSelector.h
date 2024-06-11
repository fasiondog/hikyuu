/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/multifactor/MultiFactorBase.h"
#include "../SelectorBase.h"

namespace hku {

class MultiFactorSelector : public SelectorBase {
public:
    MultiFactorSelector();
    MultiFactorSelector(const MFPtr& mf, int topn);
    virtual ~MultiFactorSelector();

    virtual void _checkParam(const string& name) const override;
    virtual void _reset() override;
    virtual SelectorPtr _clone() override;
    virtual SystemWeightList getSelected(Datetime date) override;
    virtual bool isMatchAF(const AFPtr& af) override;
    virtual void _calculate() override;

    void setIndicators(const IndicatorList& inds) {
        HKU_ASSERT(!inds.empty());
        m_inds = inds;
    }

private:
    IndicatorList m_inds;
    MFPtr m_mf;
    unordered_map<Stock, SYSPtr> m_stk_sys_dict;

    //============================================
    // 序列化支持
    //============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
        ar& BOOST_SERIALIZATION_NVP(m_inds);
        ar& BOOST_SERIALIZATION_NVP(m_mf);
    }
#endif
};

}  // namespace hku