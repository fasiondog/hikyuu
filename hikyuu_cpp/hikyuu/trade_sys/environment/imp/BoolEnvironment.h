/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-02-03
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/trade_sys/environment/EnvironmentBase.h"

namespace hku {

class BoolEnvironment : public EnvironmentBase {
public:
    BoolEnvironment();
    explicit BoolEnvironment(const Indicator& ind);
    virtual ~BoolEnvironment();

    virtual void _calculate() override;
    virtual EnvironmentPtr _clone() override;

private:
    Indicator m_ind;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
        ar& BOOST_SERIALIZATION_NVP(m_ind);
    }
#endif
};

}  // namespace hku
