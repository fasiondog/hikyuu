/*
 * TwoLineEnviroment.h
 *
 *  Created on: 2016年5月17日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_ENVIRONMENT_IMP_TWOLINEENVIRONMENT_H_
#define TRADE_SYS_ENVIRONMENT_IMP_TWOLINEENVIRONMENT_H_

#include "../../../indicator/Indicator.h"
#include "../EnvironmentBase.h"

namespace hku {

class TwoLineEnvironment : public EnvironmentBase {
public:
    TwoLineEnvironment();
    TwoLineEnvironment(const Indicator& fast, const Indicator& slow);
    virtual ~TwoLineEnvironment();

    virtual void _checkParam(const string& name) const override;
    virtual void _calculate() override;
    virtual EnvironmentPtr _clone() override;

private:
    Indicator m_fast;
    Indicator m_slow;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(EnvironmentBase);
        ar& BOOST_SERIALIZATION_NVP(m_fast);
        ar& BOOST_SERIALIZATION_NVP(m_slow);
    }
#endif
};

} /* namespace hku */

#endif /* TRADE_SYS_ENVIRONMENT_IMP_TWOLINEENVIRONMENT_H_ */
