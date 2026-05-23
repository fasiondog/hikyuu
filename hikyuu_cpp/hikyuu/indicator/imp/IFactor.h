/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IFACTOE_H_
#define INDICATOR_IMP_IFACTOE_H_

#include "../Indicator.h"
#include "hikyuu/factor/Factor.h"

namespace hku {

class IFactor : public IndicatorImp {
public:
    IFactor();
    explicit IFactor(const Factor& factor);
    virtual ~IFactor() override;

    virtual string formula() const override;
    virtual void _calculate(const Indicator& data) override;
    virtual IndicatorImpPtr _clone() override;

private:
    Factor m_factor;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp);
        ar& BOOST_SERIALIZATION_NVP(m_factor);
    }
#endif
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IFACTOE_H_ */
