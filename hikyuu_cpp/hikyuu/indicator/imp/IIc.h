/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-09
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IIc : public IndicatorImp {
public:
    IIc();
    IIc(const StockList& stks, int n, bool spearman, bool strict);
    virtual ~IIc();

    virtual void _checkParam(const string& name) const override;
    virtual void _calculate(const Indicator& data) override;
    virtual IndicatorImpPtr _clone() override;

    virtual bool selfAlike(const IndicatorImp& other) const noexcept override;

private:
    StockList m_stks;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp);
        ar& BOOST_SERIALIZATION_NVP(m_stks);
    }
#endif
};

}  // namespace hku