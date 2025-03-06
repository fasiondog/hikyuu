/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/environment/EnvironmentBase.h"

namespace hku {

class HKU_API MultiEnvironment : public EnvironmentBase {
public:
    MultiEnvironment();
    MultiEnvironment(const EnvironmentPtr& ev1, const EnvironmentPtr& ev2);
    virtual ~MultiEnvironment();

    virtual void _calculate() override;
    virtual void _reset() override;
    virtual EnvironmentPtr _clone() override;

private:
    EnvironmentPtr m_ev1;
    EnvironmentPtr m_ev2;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
        ar& BOOST_SERIALIZATION_NVP(m_ev1);
        ar& BOOST_SERIALIZATION_NVP(m_ev2);
    }
#endif
};

}  // namespace hku