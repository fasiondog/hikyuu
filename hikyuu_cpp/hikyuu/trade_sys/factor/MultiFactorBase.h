/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/KData.h"
#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/utilities/Parameter.h"

namespace hku {

/**
 * 合成多因子
 * @ingroup MultiFactor
 */
class HKU_API MultiFactorBase : public enable_shared_from_this<MultiFactorBase> {
    PARAMETER_SUPPORT

public:
    typedef Indicator::value_t value_t;

public:
    MultiFactorBase();
    MultiFactorBase(const string& name);
    MultiFactorBase(const IndicatorList& inds, const StockList& stks, const KQuery& query,
                    const Stock& ref_stk, const string& name);

    void calculate();

    /** 获取指定证券合成因子 */
    const Indicator& get(const Stock&) const;

    /** 获取指定日期截面的所有因子值 */
    const vector<std::pair<Stock, value_t>>& get(const Datetime&) const;

    /** 获取合成因子的IC */
    Indicator getIC(int ndays) const;

    /** 获取合成因子的 ICIR */
    Indicator getICIR(int ic_n, int ir_n) const;

    typedef std::shared_ptr<MultiFactorBase> MultiFactorPtr;
    MultiFactorPtr clone();

    virtual IndicatorList _calculate(const vector<IndicatorList>&) = 0;

    virtual MultiFactorPtr _clone() = 0;

protected:
    vector<IndicatorList> _alignAllInds();
    IndicatorList _getAllReturns(int ndays) const;

protected:
    string m_name;

    std::mutex m_mutex;
    IndicatorList m_inds;
    StockList m_stks;
    Stock m_ref_stk;
    KQuery m_query;
    unordered_map<Stock, size_t> m_stk_map;
    IndicatorList m_all_factors;

    unordered_map<Datetime, size_t> m_date_index;
    vector<vector<std::pair<Stock, value_t>>> m_stk_factor_by_date;

    DatetimeList m_ref_dates;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_inds);
        ar& BOOST_SERIALIZATION_NVP(m_stks);
        ar& BOOST_SERIALIZATION_NVP(m_ref_stk);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_stk_map);
        ar& BOOST_SERIALIZATION_NVP(m_all_factors);
        ar& BOOST_SERIALIZATION_NVP(m_date_index);
        ar& BOOST_SERIALIZATION_NVP(m_stk_factor_by_date);
        ar& BOOST_SERIALIZATION_NVP(m_ref_dates);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_inds);
        ar& BOOST_SERIALIZATION_NVP(m_stks);
        ar& BOOST_SERIALIZATION_NVP(m_ref_stk);
        ar& BOOST_SERIALIZATION_NVP(m_query);
        ar& BOOST_SERIALIZATION_NVP(m_stk_map);
        ar& BOOST_SERIALIZATION_NVP(m_all_factors);
        ar& BOOST_SERIALIZATION_NVP(m_date_index);
        ar& BOOST_SERIALIZATION_NVP(m_stk_factor_by_date);
        ar& BOOST_SERIALIZATION_NVP(m_ref_dates);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MultiFactorBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * 对于没有私有变量的继承子类，可直接使用该宏定义序列化
 * @code
 * class Drived: public MultiFactorBase {
 *     MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MultiFactor
 */
#define MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                          \
    friend class boost::serialization::access;                    \
    template <class Archive>                                      \
    void serialize(Archive& ar, const unsigned int version) {     \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MultiFactorBase); \
    }
#else
#define MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

typedef std::shared_ptr<MultiFactorBase> FactorPtr;
typedef std::shared_ptr<MultiFactorBase> MultiFactorPtr;
typedef std::shared_ptr<MultiFactorBase> MFPtr;

#define MULTIFACTOR_IMP(classname)             \
public:                                        \
    virtual MultiFactorPtr _clone() override { \
        return std::make_shared<classname>();  \
    }                                          \
    virtual IndicatorList _calculate(const vector<IndicatorList>&) override;

}  // namespace hku