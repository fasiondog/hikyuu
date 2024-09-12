/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-09
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/*
 * 动态移动平均
 * 用法：DMA(X,A),求X的动态移动平均。
 * 算法：若Y=DMA(X,A) 则 Y=A*X+(1-A)*Y',其中Y'表示上一周期Y值。
 * 例如：DMA(CLOSE,VOL/CAPITAL)表示求以换手率作平滑因子的平均价
 */
class IDma : public IndicatorImp {
public:
    IDma();
    explicit IDma(const Indicator& ref_a);
    virtual ~IDma();

    virtual void _checkParam(const string& name) const override;
    virtual void _calculate(const Indicator& data) override;
    virtual IndicatorImpPtr _clone() override;

private:
    Indicator m_ref_a;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp);
        ar& BOOST_SERIALIZATION_NVP(m_ref_a);
    }
#endif
};

}