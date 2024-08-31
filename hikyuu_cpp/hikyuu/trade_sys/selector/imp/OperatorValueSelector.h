/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#pragma once

#include "../SelectorBase.h"

namespace hku {

class HKU_API OperatorValueSelector : public SelectorBase {
public:
    OperatorValueSelector();
    explicit OperatorValueSelector(const string& name);
    OperatorValueSelector(const string& name, const SelectorPtr& se, double value);
    virtual ~OperatorValueSelector();

    virtual void _reset() override;
    virtual SelectorPtr _clone() override;
    virtual bool isMatchAF(const AFPtr& af) override;
    virtual void _calculate() override;
    virtual SystemWeightList getSelected(Datetime date) override {
        return SystemWeightList();
    }

protected:
    SelectorPtr m_se;
    double m_value{0.0};

private:
    //============================================
    // 序列化支持
    //============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    // template <class Archive>
    // void serialize(Archive& ar, const unsigned int version) {
    //     ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
    //     ar& BOOST_SERIALIZATION_NVP(m_se);
    //     ar& BOOST_SERIALIZATION_NVP(m_value);
    // }
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_value);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_value);
        if (m_se) {
            m_pro_sys_list = m_se->getProtoSystemList();
        }
    }

#endif
};

#define OPERATOR_VALUE_SELECTOR_IMP(classname, name)                                           \
public:                                                                                        \
    classname() : OperatorValueSelector(name) {}                                               \
    classname(const SelectorPtr& se, double value) : OperatorValueSelector(name, se, value) {} \
    virtual ~classname() {}                                                                    \
                                                                                               \
    virtual SystemWeightList getSelected(Datetime date) override;                              \
                                                                                               \
    virtual SelectorPtr _clone() override {                                                    \
        auto p = std::make_shared<classname>();                                                \
        if (m_se) {                                                                            \
            p->m_se = m_se->clone();                                                           \
        }                                                                                      \
        p->m_value = m_value;                                                                  \
        return p;                                                                              \
    }

#if HKU_SUPPORT_SERIALIZATION
#define OPERATOR_VALUE_SELECTOR_SERIALIZATION                           \
private:                                                                \
    friend class boost::serialization::access;                          \
    template <class Archive>                                            \
    void serialize(Archive& ar, const unsigned int version) {           \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(OperatorValueSelector); \
    }
#else
#define OPERATOR_VALUE_SELECTOR_SERIALIZATION
#endif

}  // namespace hku