/*
 * IndicatorStoploss.h
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATORSTOPLOSS_H_
#define INDICATORSTOPLOSS_H_

#include "../../../indicator/Indicator.h"
#include "../StoplossBase.h"

namespace hku {

class IndicatorStoploss : public StoplossBase {
public:
    IndicatorStoploss();  // 仅用于序列化默认构造函数
    IndicatorStoploss(const Indicator& op, const string& kdata_part);
    virtual ~IndicatorStoploss();
    virtual void _checkParam(const string& name) const;

    virtual price_t getPrice(const Datetime& datetime, price_t price) override;
    virtual void _reset() override;
    virtual StoplossPtr _clone() override;
    virtual void _calculate() override;

private:
    Indicator m_op;
    map<Datetime, price_t> m_result;

//========================================
// 序列化支持
//========================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(StoplossBase);
        ar& BOOST_SERIALIZATION_NVP(m_op);
        // m_result 每次系统运行时都要重新计算，不保存
        // ar & BOOST_SERIALIZATION_NVP(m_result);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

} /* namespace hku */
#endif /* INDICATORSTOPLOSS_H_ */
