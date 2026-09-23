/*
 * AllocateFundsBase.h
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Base class of the portfolio-level fund allocation (AF), it is held by the aggregate form
 *  (MultiSystem) only.
 *
 *  AF is composed of three replaceable algorithm parts, each of them corresponds to one virtual
 *  function (L1/L2/L3):
 *   - L1 `_allocate`  sub-system context -> weight (mode A nominal weight / mode B real quota)
 *   - L2 `_toTargets` weight -> the executable quantity of the parent account (the folding point
 *      from the sub-system space to the individual instrument space)
 *   - L3 `_checkRisk` portfolio risk control clipping (concentration, etc.)
 *
 *  The overall entry is `allocate()`, it is always executed in the L1 -> L2 -> L3 order.
 *  The single-security form uses MoneyManagerBase (MM); the two no longer share the class
 *  hierarchy, the parameter family or the allocation mode.
 *  Created on: 2018-1-30
 *      Author: fasiondog
 */

#pragma once
#ifndef ALLOCATEFUNDSBASE_H_
#define ALLOCATEFUNDSBASE_H_

#include <unordered_map>
#include <vector>

#include "../../utilities/Parameter.h"
#include "../system/SystemPart.h"
#include "../system/TradeSuggestion.h"
#include "../system/SubSystemContext.h"
#include "../../trade_manage/TradeManager.h"

namespace hku {

class System;  // Forward declaration, to avoid a circular include with System.h
using SYSPtr = std::shared_ptr<System>;

class AllocateFundsBase;
typedef shared_ptr<AllocateFundsBase> AllocateFundsPtr;

/**
 * The pointer type that the client programs should use
 * @ingroup AllocateFunds
 */
using AFPtr = AllocateFundsPtr;

/**
 * Base class of the portfolio-level fund allocation (AF). It contains three replaceable algorithm
 * parts L1/L2/L3.
 * @note It is used by the aggregate form (MultiSystem) only; the single-security form never calls
 *       this class.
 * @ingroup AllocateFunds
 */
class HKU_API AllocateFundsBase : public enable_shared_from_this<AllocateFundsBase> {
    PARAMETER_SUPPORT_WITH_CHECK

public:
    /** Default constructor */
    AllocateFundsBase();
    explicit AllocateFundsBase(const string& name);
    AllocateFundsBase(const AllocateFundsBase&) = default;
    virtual ~AllocateFundsBase();

    /** Get the name */
    const string& name() const {
        return m_name;
    }

    /** Set the name */
    void name(const string& name) {
        m_name = name;
    }

    /** Reset */
    void reset();

    /** Clone operation */
    AllocateFundsPtr clone();

    /** Set the trade account (the real account of the parent/aggregate system) */
    void setTM(const TradeManagerPtr& tm) {
        m_tm = tm;
    }

    /** Get the trade account */
    TradeManagerPtr getTM() const {
        return m_tm;
    }

    /** Set the query condition */
    void setQuery(const KQuery& query) {
        m_query = query;
    }

    /** Get the query condition */
    const KQuery& getQuery() const {
        return m_query;
    }

    /** Set the allocation mode: A (signal aggregation, the default) or B (fund allocation /
     *  FOF-MOM).
     *  @note This field is the **only source** of the allocation mode; the aggregate system does
     *        not keep one of its own any more. */
    void setMode(const string& mode) {
        m_mode = (mode == "B" || mode == "b") ? "B" : "A";
    }

    /** Get the allocation mode */
    const string& getMode() const {
        return m_mode;
    }

    /** Weight table: sub-system -> weight (the L1 output and the L2 input) */
    using Weights = std::unordered_map<SYSPtr, double>;

    /**
     * The unified entry of L1/L2/L3, it is called by the aggregate System (MultiSystem).
     * Flow: L1 system-level allocation (nominal weight / real quota) -> L2 behavior-level
     *       conversion (mode A by proportion / mode B pass-through) -> L3 portfolio risk control
     *       clipping.
     * @param date the trade date
     * @param tm the real trade account of the parent (aggregate) system
     * @param suggestions [in/out] the suggestions submitted by every sub-system; L2 rewrites them
     *        in place into the executable quantity of the parent account
     * @param contexts the context of every sub-system (the virtual account funds / the mode B
     *        quota, etc.), used by L1 for the allocation
     * @param query the query condition (the K-line type, etc.)
     */
    void allocate(const Datetime& date, const TradeManagerPtr& tm, TradeSuggestionList& suggestions,
                  SubSystemContextList& contexts, const KQuery& query);

    /** L1 system-level allocation: mode A returns the nominal weight (suggested weight), mode B
     *  returns the real quota (written into contexts[i].quota).
     *  @note It returns the equal weight (1/N) by default; when the parameter weight-list is not
     *        empty the fixed weights are used instead (migrated from AF_FixedWeight /
     *        AF_FixedWeightList). */
    virtual Weights _allocate(const Datetime& date, const TradeManagerPtr& tm,
                              SubSystemContextList& contexts, const KQuery& query);

    /** Parse the parameter weight-list (the comma separated fixed weights) into a normalized weight
     *  vector; an empty vector is returned when it is empty, when its size does not match the
     *  sub-systems or when the sum is <= 0 (the caller falls back to the equal weight). Migrated
     *  from AF_FixedWeight / AF_FixedWeightList. */
    std::vector<double> _parseWeightList(size_t expect_n) const;

    /** L1 skeleton: fill the L1 result by "sub-system index -> weight", and in mode B write
     *  contexts[i].quota (when fixed-amount>0 the fixed quota takes precedence, otherwise
     *  weight x the parent total assets).
     *  @note **No normalization is performed**, the weight semantics is fully decided by the
     *        subclass; the missing index (weights shorter than contexts) falls back to the equal
     *        weight. */
    Weights _applyWeights(const Datetime& date, const TradeManagerPtr& tm,
                          SubSystemContextList& contexts, const KQuery& query,
                          const std::vector<double>& weights) const;

    /** L2 behavior-level conversion: in mode A it is converted into the parent account quantity by
     *  assets_ratio / weight; in mode B the sub-system number is passed through.
     *  @note The default implementation is mode A (equal weight to position): the number of every
     *        suggestion is rewritten into the target share quantity "weight x the parent total
     *        assets / the planned price"; the SELL/CLEAR flags are fully closed (MAX_DOUBLE). */
    virtual void _toTargets(const Datetime& date, const TradeManagerPtr& tm,
                            TradeSuggestionList& suggestions, const Weights& sys_weight,
                            const KQuery& query);

    /** L3 portfolio risk control clipping: mode A performs the portfolio dimension risk control
     *  (concentration / turnover, etc.); mode B can be disabled or only performs the total amount
     *  check.
     *  @note The single instrument concentration is limited by the parameter max-single-position by
     *        default. */
    virtual void _checkRisk(const Datetime& date, const TradeManagerPtr& tm,
                            TradeSuggestionList& suggestions, const KQuery& query);

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Interface for the subclass to clone its private variables */
    virtual AllocateFundsPtr _clone() = 0;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

protected:
    string m_name;
    string m_mode{"A"};  // The allocation mode: A=signal aggregation (the default) / B=fund allocation (FOF-MOM)
    KQuery m_query;
    TradeManagerPtr m_tm;
    bool m_is_python_object{false};

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_mode);
        // m_query and m_tm are set temporarily when the system runs, they do not need to be serialized
    }

    template <class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
        ar& BOOST_SERIALIZATION_NVP(m_mode);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HKU_SUPPORT_SERIALIZATION */
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(AllocateFundsBase)
#endif

#if HKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used directly for the
 * serialization
 * @code
 * class Drived: public AllocateFundsBase {
 *     ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup AllocateFunds
 */
#define ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION              \
private:                                                            \
    friend class boost::serialization::access;                      \
    template <class Archive>                                        \
    void serialize(Archive& ar, const unsigned int version) {       \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(AllocateFundsBase); \
    }
#else
#define ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define ALLOCATE_FUNDS_IMP(classname)                \
public:                                              \
    virtual AllocateFundsPtr _clone() override {     \
        return std::make_shared<classname>();        \
    }

HKU_API std::ostream& operator<<(std::ostream&, const AllocateFundsBase&);
HKU_API std::ostream& operator<<(std::ostream&, const AllocateFundsPtr&);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::AllocateFundsBase> : ostream_formatter {};

template <>
struct fmt::formatter<hku::AllocateFundsPtr> : ostream_formatter {};
#endif

#endif /* ALLOCATEFUNDSBASE_H_ */
