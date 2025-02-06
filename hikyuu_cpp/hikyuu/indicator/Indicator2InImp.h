/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-03
 *      Author: fasiondog
 */

#pragma once

#include "Indicator.h"

namespace hku {

class Indicator2InImp : public IndicatorImp {
public:
    Indicator2InImp();
    explicit Indicator2InImp(const string& name, size_t result_num = 1);
    Indicator2InImp(const string& name, const Indicator& ref_a, bool fill_null = false,
                    size_t result_num = 1);
    virtual ~Indicator2InImp();

    virtual IndicatorImpPtr _clone() override;

protected:
    Indicator prepare(const Indicator& ind);

protected:
    Indicator m_ref_ind;

//============================================
// 序列化支持
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp);
        ar& BOOST_SERIALIZATION_NVP(m_ref_ind);
    }
#endif
};

#if HKU_SUPPORT_SERIALIZATION
#define INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION          \
private:                                                          \
    friend class boost::serialization::access;                    \
    template <class Archive>                                      \
    void serialize(Archive& ar, const unsigned int version) {     \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(Indicator2InImp); \
    }
#else
#define INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define INDICATOR2IN_IMP(classname)                          \
public:                                                      \
    virtual void _calculate(const Indicator& data) override; \
    virtual IndicatorImpPtr _clone() override {              \
        auto p = make_shared<classname>();                   \
        p->m_ref_ind = m_ref_ind.clone();                    \
        return p;                                            \
    }

}  // namespace hku