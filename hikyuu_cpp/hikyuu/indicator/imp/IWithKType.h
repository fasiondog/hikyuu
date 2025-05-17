/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-28
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IWithKType : public IndicatorImp {
public:
    IWithKType();
    explicit IWithKType(const Indicator& ref_ind, bool fill_null, const KQuery::KType& ktype);
    virtual ~IWithKType();

    virtual string str() const override;
    virtual string formula() const override;
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