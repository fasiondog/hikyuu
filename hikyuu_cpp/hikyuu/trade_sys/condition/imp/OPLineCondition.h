/*
 * OPCondition.h
 *
 *  Created on: 2016-5-9
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_CONDITION_IMP_OPLINECONDITION_H_
#define TRADE_SYS_CONDITION_IMP_OPLINECONDITION_H_

#include "../ConditionBase.h"
#include "../../../indicator/Indicator.h"

namespace hku {

class OPLineCondition : public ConditionBase {
public:
    OPLineCondition();
    explicit OPLineCondition(const Indicator&);
    virtual ~OPLineCondition();

    virtual void _calculate() override;
    virtual ConditionPtr _clone() override;

private:
    Indicator m_op;

    // Cache of the last embedded-backtest result, keyed by (stock, query, sg). The backtest is the
    // dominant cost of this condition, so identical inputs reuse the result. The key ties to the
    // inputs, so the cached output is identical by construction.
    Stock m_last_stock;
    KQuery m_last_query;
    SGPtr m_last_sg;
    Indicator m_last_x;

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConditionBase);
        ar& BOOST_SERIALIZATION_NVP(m_op);
    }
#endif
};

} /* namespace hku */

#endif /* TRADE_SYS_CONDITION_IMP_OPLINECONDITION_H_ */
