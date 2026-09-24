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
 * Score board with the given indicator weights; the score of a security is the weighted sum of its
 * indicators
 */
class WeightMultiFactor : public MultiFactorBase {
    MULTIFACTOR_IMP(WeightMultiFactor)

public:
    WeightMultiFactor();
    WeightMultiFactor(const PriceList& weights, const StockList& stks, const KQuery& query,
                      const Stock& ref_stk, int ic_n, bool spearman, int mode,
                      bool save_all_factors);
    virtual ~WeightMultiFactor() override = default;

private:
    PriceList m_weights;  // The weight of every indicator, it is as long as the input inds list

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