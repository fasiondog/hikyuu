/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#pragma once

#include "../MultiFactorBase.h"

namespace hku {

/**
 * 指定指标权重评分板，证券得分为其个指标的权重之和
 */
class WeightMultiFactor : public MultiFactorBase {
    MULTIFACTOR_IMP(WeightMultiFactor)

public:
    WeightMultiFactor();
    WeightMultiFactor(const vector<Indicator>& inds, const PriceList& weights,
                      const StockList& stks, const KQuery& query, const Stock& ref_stk, int ic_n,
                      bool spearman);
    virtual ~WeightMultiFactor() = default;

private:
    PriceList m_weights;  // 每个指标权重，与输入 inds 列表等长

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MultiFactorBase);
        ar& BOOST_SERIALIZATION_NVP(m_weights);
    }
#endif
};

}  // namespace hku