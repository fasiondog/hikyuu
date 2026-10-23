/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/selector/SelectorBase.h"

namespace hku {

class HKU_API OperatorSelector : public SelectorBase {
public:
    OperatorSelector();
    explicit OperatorSelector(const string& name);
    OperatorSelector(const string& name, const SelectorPtr& se1, const SelectorPtr& se2);
    virtual ~OperatorSelector();

    virtual void _reset() override;
    virtual SelectorPtr _clone() override;

    virtual void _calculate() override;
    virtual SystemWeightList _getSelected(Datetime date) override {
        return SystemWeightList();
    }

    virtual void _addSystem(const SYSPtr& sys) override;
    virtual void _removeAll() override;

protected:
    virtual void bindRealToProto(const SYSPtr& real, const SYSPtr& proto) override {
        m_real_to_proto[real] = proto;
        // The operand may be a logical selector itself (e.g. (a + b) + c), the binding must reach
        // it, otherwise the nested level would never learn its prototypes and select nothing
        if (m_se1) {
            m_se1->bindRealToProto(real, proto);
        }
        if (m_se2) {
            m_se2->bindRealToProto(real, proto);
        }
    }

    // The weight is the raw func result and may be negative (e.g. 0 - w in SE_Sub): it is an
    // intermediate value for the downstream filter to flip or screen, not an invalid state. The
    // list is sorted by weight only; the tie order stays undefined on purpose (getSelected runs
    // per bar by default, a market_code tie-break would cost more than it is worth).
    SystemWeightList getUnionSelected(Datetime date,
                                      const std::function<double(double, double)>&& func);

    SystemWeightList getIntersectionSelected(Datetime date,
                                             const std::function<double(double, double)>&& func);

protected:
    static void sortSystemWeightList(SystemWeightList& swlist);

    void build();
    void cloneRebuild(const SelectorPtr& se1, const SelectorPtr& se2);

protected:
    SelectorPtr m_se1;
    SelectorPtr m_se2;
    std::unordered_set<SYSPtr> m_se1_set;  // The prototype system instance set of se1
    std::unordered_set<SYSPtr> m_se2_set;  // The prototype system instance set of se2
    std::unordered_map<SYSPtr, SYSPtr> m_real_to_proto;

private:
    static std::unordered_set<System*> findIntersection(const SelectorPtr& se1,
                                                        const SelectorPtr& se2);

private:
    //============================================
    // Serialization support
    //============================================
#if HKU_SUPPORT_SERIALIZATION
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
        ar& BOOST_SERIALIZATION_NVP(m_se1);
        ar& BOOST_SERIALIZATION_NVP(m_se2);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
        ar& BOOST_SERIALIZATION_NVP(m_se1);
        ar& BOOST_SERIALIZATION_NVP(m_se2);
        build();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif
};

#define OPERATOR_SELECTOR_IMP(classname, name)                     \
public:                                                            \
    classname() : OperatorSelector(name) {}                        \
    classname(const SelectorPtr& se1, const SelectorPtr& se2)      \
    : OperatorSelector(name, se1, se2) {}                          \
    virtual ~classname() override {}                               \
                                                                   \
    virtual SystemWeightList _getSelected(Datetime date) override; \
                                                                   \
    virtual SelectorPtr _clone() override {                        \
        auto p = std::make_shared<classname>();                    \
        p->cloneRebuild(m_se1, m_se2);                             \
        return p;                                                  \
    }

#if HKU_SUPPORT_SERIALIZATION
#define OPERATOR_SELECTOR_SERIALIZATION                            \
private:                                                           \
    friend class boost::serialization::access;                     \
    template <class Archive>                                       \
    void serialize(Archive& ar, const unsigned int version) {      \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(OperatorSelector); \
    }
#else
#define OPERATOR_SELECTOR_SERIALIZATION
#endif

}  // namespace hku