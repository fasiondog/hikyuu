/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-xx-xx
 *      Author: fasiondog
 */

#pragma once

#include "../SelectorBase.h"

namespace hku {

class MultiFactorSelector2 : public SelectorBase {
public:
    MultiFactorSelector2();
    MultiFactorSelector2(const MFPtr& mf);
    virtual ~MultiFactorSelector2();

    virtual void _checkParam(const string& name) const override;
    virtual void _reset() override;
    virtual SelectorPtr _clone() override;
    virtual SystemWeightList _getSelected(Datetime date) override;
    virtual bool isMatchAF(const AFPtr& af) override;
    virtual void _calculate() override;

    void setIndicators(const IndicatorList& inds) {
        HKU_ASSERT(!inds.empty());
        m_inds = inds;
        m_calculated = false;
    }

private:
    IndicatorList m_inds;
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
    }
#endif
};

}  // namespace hku