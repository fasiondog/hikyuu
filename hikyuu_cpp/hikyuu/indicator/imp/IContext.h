/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-28
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IContext : public IndicatorImp {
public:
    IContext();
    explicit IContext(const Indicator& ref_ind);
    virtual ~IContext();

    virtual string str() const override;
    virtual string formula() const override;
    virtual void _calculate(const Indicator& data) override;
    virtual IndicatorImpPtr _clone() override;

    KData getContextKdata() const;

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