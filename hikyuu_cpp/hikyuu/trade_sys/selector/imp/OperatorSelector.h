/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#pragma once

#include "../SelectorBase.h"

namespace hku {

class HKU_API OperatorSelector : public SelectorBase {
public:
    OperatorSelector();
    explicit OperatorSelector(const string& name);
    OperatorSelector(const string& name, const SelectorPtr& se1, const SelectorPtr& se2);
    virtual ~OperatorSelector();

    virtual void _reset() override;
    virtual SelectorPtr _clone() override;
    virtual bool isMatchAF(const AFPtr& af) override;
    virtual void _calculate() override;
    virtual SystemWeightList getSelected(Datetime date) override {
        return SystemWeightList();
    }

    virtual void _addSystem(const SYSPtr& sys) override;
    virtual void _removeAll() override;

protected:
    virtual void bindRealToProto(const SYSPtr& real, const SYSPtr& proto) override {
        m_real_to_proto[real] = proto;
    }

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
    std::unordered_set<SYSPtr> m_se1_set;  // se1 的原型系统实例集合
    std::unordered_set<SYSPtr> m_se2_set;  // se2 的原型系统实例集合
    std::unordered_map<SYSPtr, SYSPtr> m_real_to_proto;

private:
    static std::unordered_set<System*> findIntersection(const SelectorPtr& se1,
                                                        const SelectorPtr& se2);

private:
    //============================================
    // 序列化支持
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
#endif
};

#define OPERATOR_SELECTOR_IMP(classname, name)                    \
public:                                                           \
    classname() : OperatorSelector(name) {}                       \
    classname(const SelectorPtr& se1, const SelectorPtr& se2)     \
    : OperatorSelector(name, se1, se2) {}                         \
    virtual ~classname() {}                                       \
                                                                  \
    virtual SystemWeightList getSelected(Datetime date) override; \
                                                                  \
    virtual SelectorPtr _clone() override {                       \
        HKU_THROW("OperatorSelector Could't support clone!");     \
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