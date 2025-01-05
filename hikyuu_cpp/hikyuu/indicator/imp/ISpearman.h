/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class ISpearman : public IndicatorImp {
public:
    ISpearman();
    ISpearman(const Indicator& ref_ind, int n, bool fill_null);
    virtual ~ISpearman();

    virtual void _checkParam(const string& name) const override;
    virtual void _calculate(const Indicator& data) override;
    virtual IndicatorImpPtr _clone() override;

private:
    Indicator m_ref_ind;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp);
        ar& BOOST_SERIALIZATION_NVP(m_ref_ind);
    }
#endif
};

}  // namespace hku