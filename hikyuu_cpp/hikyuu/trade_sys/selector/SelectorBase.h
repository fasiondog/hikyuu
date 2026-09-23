/*
 * SelectorBase.h
 *
 *  Created on: 2016-2-21
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SELECTOR_SELECTORBASE_H_
#define TRADE_SYS_SELECTOR_SELECTORBASE_H_

#include "../system/System.h"
#include "../../KData.h"
#include "../../utilities/Parameter.h"

#include "hikyuu/trade_sys/multifactor/MultiFactorBase.h"
#include "SystemWeight.h"

namespace hku {

class HKU_API Portfolio;

/**
 * Trading object selection module
 * @ingroup Selector
 */
class HKU_API SelectorBase : public enable_shared_from_this<SelectorBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    /** Default constructor */
    SelectorBase();
    SelectorBase(const SelectorBase&) = default;

    /**
     * Constructor, it also gives the algorithm name
     * @param name the given name
     */
    explicit SelectorBase(const string& name);

    /** Destructor */
    virtual ~SelectorBase();

    /** Get the algorithm name */
    const string& name() const;

    /** Set the algorithm name */
    void name(const string& name);

    using PFPtr = shared_ptr<Portfolio>;
    PFPtr getPF() const {
        return m_pf.lock();
    }

    void setPF(const PFPtr& pf) {
        m_pf = pf;
    }

    /**
     * Add a candidate stock and its trading strategy prototype
     * @param stock candidate stock
     * @param protoSys trading system strategy prototype
     */
    void addStock(const Stock& stock, const SystemPtr& protoSys);

    /**
     * Add a group of stocks with the same trading strategy
     * @note An invalid stock is ignored automatically, false is not returned
     * @param stkList candidate stock list
     * @param protoSys trading system strategy prototype
     */
    void addStockList(const StockList& stkList, const SystemPtr& protoSys);

    /**
     * Add an existing system strategy instance directly
     * @note The stock should already be bound
     * @param sys
     */
    void addSystem(const SYSPtr& sys);

    /**
     * Add an existing system strategy instance directly
     * @note The stock should already be bound
     * @param sys
     */
    void addSystemList(const SystemList& sys);

    /**
     * @brief Get the prototype system list
     * @return const SystemList&
     */
    const SystemList& getProtoSystemList() const;

    /**
     * @brief Get the system list actually run by PF
     * @return const SystemList&
     */
    const SystemList& getRealSystemList() const;

    /** Get the targets selected at the close of the given moment */
    SystemWeightList getSelected(Datetime date);

    /**
     * @brief Reset
     * @note The reset does not clear the existing prototype systems
     */
    void reset();

    /**
     * Clear the existing system prototypes
     */
    void removeAll();

    typedef shared_ptr<SelectorBase> SelectorPtr;
    SelectorPtr clone();

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Subclass clone interface */
    virtual SelectorPtr _clone() = 0;

    /** Subclass calculation interface */
    virtual void _calculate() = 0;

    /** Subclass interface to get the targets selected at the close of the given moment */
    virtual SystemWeightList _getSelected(Datetime date) = 0;



    /** Add a prototype system in the subclass used for the logical operations; it generally does
     *  not need to be implemented by the subclass */
    virtual void _addSystem(const SYSPtr& sys) {}

    /** Add a prototype system in the subclass used for the logical operations; it generally does
     *  not need to be implemented by the subclass */
    virtual void _removeAll() {}

    /* Called by PF only; PF notifies it of the system list it actually runs and starts the
     * calculation */
    virtual void calculate(const SystemList& pf_realSysList, const KQuery& query);

    /* Called by PF only, it builds the mapping from the actual systems to the prototype systems */
    virtual void bindRealToProto(const SYSPtr& real, const SYSPtr& proto) {}

    void calculate_proto(const KQuery& query);

    virtual string str() const;

public:
    //------------------------------------------------------------------------
    // It is useful for the Selector related to MF only; it is placed here mainly so that SEPtr can
    // get the MF related information directly
    // It is useless for the Selector not related to MF
    //------------------------------------------------------------------------
    MFPtr getMF() const {
        return m_mf;
    }

    void setMF(const MFPtr& mf) {
        m_mf = mf;
        m_calculated = false;
    }

    ScoresFilterPtr getScoresFilter() const {
        return m_sc_filter;
    }

    /** Set the cross-section score record filter, it is used for the MF related Selector only, to
     *  filter when the Score list is got from MF */
    void setScoresFilter(const ScoresFilterPtr& filter);

    /** Append a filter on the basis of the existing filters, it is used for the MF related Selector
     *  only, to filter when the Score list is got from MF */
    void addScoresFilter(const ScoresFilterPtr& filter);

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

private:
    void initParam();

protected:
    ScoresFilterPtr m_sc_filter;
    MFPtr m_mf;

protected:
    string m_name;
    bool m_is_python_object{false};
    bool m_calculated{false};  // Whether it has been calculated
    bool m_proto_calculated{false};
    KQuery m_query;
    KQuery m_proto_query;

    SystemList m_pro_sys_list;   // Prototype system list
    SystemList m_real_sys_list;  // The systems actually run in the PF portfolio, set when PF
                                 // executes, in the same order as the prototype list

    std::weak_ptr<Portfolio> m_pf;  // Stored but not serialized, the reference to PF

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_pro_sys_list);
        ar& BOOST_SERIALIZATION_NVP(m_sc_filter);
        ar& BOOST_SERIALIZATION_NVP(m_mf);
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_pro_sys_list);
        ar& BOOST_SERIALIZATION_NVP(m_sc_filter);
        ar& BOOST_SERIALIZATION_NVP(m_mf);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(SelectorBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public SelectorBase {
 *     SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup Selector
 */
#define SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION               \
private:                                                       \
    friend class boost::serialization::access;                 \
    template <class Archive>                                   \
    void serialize(Archive& ar, const unsigned int version) {  \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase); \
    }
#else
#define SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define SELECTOR_IMP(classname)                                    \
public:                                                            \
    virtual SelectorPtr _clone() override {                        \
        return std::make_shared<classname>();                      \
    }                                                              \
    virtual SystemWeightList _getSelected(Datetime date) override; \
    virtual void _calculate() override;

/**
 * Client programs should all use this pointer type
 * @ingroup Selector
 */
typedef shared_ptr<SelectorBase> SelectorPtr;
typedef shared_ptr<SelectorBase> SEPtr;

HKU_API std::ostream& operator<<(std::ostream&, const SelectorBase&);
HKU_API std::ostream& operator<<(std::ostream&, const SelectorPtr&);

inline const string& SelectorBase::name() const {
    return m_name;
}

inline void SelectorBase::name(const string& name) {
    m_name = name;
}

inline const SystemList& SelectorBase::getRealSystemList() const {
    return m_real_sys_list;
}

inline const SystemList& SelectorBase::getProtoSystemList() const {
    return m_pro_sys_list;
}

inline void SelectorBase::setScoresFilter(const ScoresFilterPtr& filter) {
    m_sc_filter = filter;
    m_calculated = false;
}

inline void SelectorBase::addScoresFilter(const ScoresFilterPtr& filter) {
    m_sc_filter = m_sc_filter | filter;
    m_calculated = false;
}

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::SelectorBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::SelectorPtr> : ostream_formatter {};
#endif

#endif /* TRADE_SYS_SELECTOR_SELECTORBASE_H_ */
