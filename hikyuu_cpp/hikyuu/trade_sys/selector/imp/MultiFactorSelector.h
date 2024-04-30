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
    using filter_func_t = std::function<bool(const Stock&, const Datetime&)>;

public:
    MultiFactorSelector();
    MultiFactorSelector(const MFPtr& mf, int topn);
    MultiFactorSelector(const MFPtr& mf, int topn, const filter_func_t& filter);
    virtual ~MultiFactorSelector();

    virtual void _checkParam(const string& name) const override;
    virtual void _reset() override;
    virtual SelectorPtr _clone() override;
    virtual SystemWeightList getSelected(Datetime date) override;
    virtual bool isMatchAF(const AFPtr& af) override;
    virtual void _calculate() override;

private:
    MFPtr m_mf;
    unordered_map<Stock, SYSPtr> m_stk_sys_dict;
    filter_func_t m_filter;

    //============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
        ar& BOOST_SERIALIZATION_NVP(m_mf);
    }
#endif
};

}  // namespace hku