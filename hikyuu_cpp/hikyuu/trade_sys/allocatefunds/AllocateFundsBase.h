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
 *
 *  - **L1 system-level allocation** `_allocate`: decide "how much each sub-system may manage".
 *    Input is the sub-system context list, output is the weight table; in mode B (Fund
 *    Allocation) it also writes the real cash quota into `contexts[i].quota` (weight x the parent
 *    net assets, or the fixed amount). Examples: AF_EqualWeight (1/N), AF_FixedWeight,
 *    AF_MultiFactor (weighted by the SE score).
 *
 *  - **L2 behavior-level conversion** `_toTargets`: convert the sub-system suggestions into the
 *    executable quantity of the parent account (the folding point from the sub-system space to
 *    the individual instrument space). In mode A (Signal Aggregation) the target position market
 *    value = weight x position ratio x the parent net assets, aggregated per instrument and
 *    rebalanced by the delta to the current position; in mode B the real instruction of the
 *    sub-system passes through unchanged.
 *
 *  - **L3 portfolio risk control** `_checkRisk`: clip the suggestions at the portfolio
 *    dimension (e.g. the single instrument concentration cap `max_single_position`); mode B
 *    respects the sub-manager autonomy and skips the clipping.
 *
 *  The overall entry is `allocate()` = L1 -> L2 -> L3. When the quota is needed before driving
 *  the sub-systems (the mode B calibration), use `allocateQuota()` (L1 only) and
 *  `allocateTargets()` (L2 -> L3) separately.
 *  The running mode ("A" Signal Aggregation / "B" Fund Allocation / "C" Shared Account
 *  Compatibility) is **owned by MultiSystem** and is passed explicitly into every L1/L2/L3 hook
 *  as the trailing `mode` argument; AF itself does not hold it. See the MultiSystem class comment
 *  for the three modes.
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
 * parts: L1 system-level allocation / L2 behavior-level conversion / L3 portfolio risk control
 * (see the file comment for the details).
 * @note It is used by the aggregate form (MultiSystem) only; the single-security form never calls
 *       this class. The running mode ("A" Signal Aggregation / "B" Fund Allocation / "C" Shared
 *       Account Compatibility) is owned by MultiSystem and passed into the hooks.
 * @ingroup AllocateFunds
 */
class HKU_API AllocateFundsBase {
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

    // @note The running mode is no longer held here: it is owned by MultiSystem and passed into
    //       every L1/L2/L3 hook through the trailing `mode` argument. In mode C the aggregate
    //       system drives the sub-systems directly on its own real account and never calls
    //       allocate(); the mode checks inside the hooks are only a defensive pass-through for an
    //       AF invoked in mode C.

    /** Weight table: sub-system instance -> weight (the L1 output and the L2 input)
     *  @note It is keyed by the System instance itself, so a suggestion whose system is not the
     *        very instance held by the context (e.g. a clone returned by a Selector) has no weight
     *        and is dropped by L2. */
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
     * @param mode the running mode ("A"/"B"/"C"), owned by MultiSystem and passed to L1/L2/L3
     */
    void allocate(const Datetime& date, const TradeManagerPtr& tm, TradeSuggestionList& suggestions,
                  SubSystemContextList& contexts, const KQuery& query, const string& mode);

    /**
     * L1 only: calculate the weights and (in mode B) write the real quota into contexts[i].quota,
     * without converting any suggestion. It is used by the aggregate System when the quota is
     * needed **before** driving the sub-systems (the mode B quota-first calibration).
     * @return the L1 weight table (the input of allocateTargets)
     */
    Weights allocateQuota(const Datetime& date, const TradeManagerPtr& tm,
                          SubSystemContextList& contexts, const KQuery& query, const string& mode);

    /**
     * L2 + L3 only: convert the suggestions into the executable quantity of the parent account and
     * perform the portfolio risk control. It is used together with allocateQuota by the aggregate
     * System when the suggestions are collected after the quota calibration.
     */
    void allocateTargets(const Datetime& date, const TradeManagerPtr& tm,
                         TradeSuggestionList& suggestions, const Weights& sys_weight,
                         const KQuery& query, const string& mode);

    /** L1 system-level allocation: mode A returns the nominal weight (suggested weight), mode B
     *  returns the real quota (written into contexts[i].quota).
     *  @note It returns the equal weight (1/N) by default; the other weight sources are the
     *        subclasses overriding this (AF_FixedWeight / AF_FixedWeightList / AF_MultiFactor). */
    virtual Weights _allocate(const Datetime& date, const TradeManagerPtr& tm,
                              SubSystemContextList& contexts, const KQuery& query,
                              const string& mode);

    /** L1 skeleton: fill the L1 result by "sub-system index -> weight", and in mode B write
     *  contexts[i].quota (when fixed_amount>0 the fixed quota takes precedence, otherwise
     *  weight x the parent net assets).
     *  @note **No normalization is performed**, the weight semantics is fully decided by the
     *        subclass; the missing index (weights shorter than contexts) falls back to the equal
     *        weight.
     *  @note The mode B quota sum (N x fixed_amount included) is bounded by
     *        max_total_position x net assets: the weight pricing is a proportion and is scaled
     *        down, while the fixed_amount pricing is an absolute amount and instead truncates the
     *        number of funded sub-systems (the rest get a zero quota). */
    Weights _applyWeights(const Datetime& date, const TradeManagerPtr& tm,
                          SubSystemContextList& contexts, const KQuery& query,
                          const std::vector<double>& weights, const string& mode) const;

    /** L2 behavior-level conversion: in mode A it converts the parent account quantity by the
     *  budget (weight / assets_ratio / fixed_amount, capped by max_total_position) and rewrites
     *  the number of every suggestion (a positive magnitude, the direction expressed by the
     *  type); in mode B the sub-system number is passed through.
     *  @note The number is always positive; a short suggestion is turned into a SELL to reduce
     *        to its target, a SELL/CLEAR closes the current holding.
     *  @note Mode A prices a suggestion only when its system has a weight: a suggestion from a
     *        system absent from the weight table (a missing key, an empty table, or a cloned
     *        instance that is not the context one) is dropped with a warning. The weight table is
     *        produced by the matching L1 call on the same contexts.
     *  @note fixed_amount > 0 skips the weight lookup entirely: every instrument gets that amount,
     *        and the total budget truncates the number of funded instruments in first-appearance
     *        order (the rest are driven flat) instead of shrinking every target. */
    virtual void _toTargets(const Datetime& date, const TradeManagerPtr& tm,
                            TradeSuggestionList& suggestions, const Weights& sys_weight,
                            const KQuery& query, const string& mode);

    /** L3 portfolio risk control clipping: mode A performs the portfolio dimension risk control
     *  (concentration / turnover, etc.); mode B can be disabled or only performs the total amount
     *  check.
     *  @note The single instrument concentration is limited by the parameter max_single_position
     *        (<=0 means no limit, >1 is clamped to 1, so the default 1.0 caps a single instrument
     *        at the net assets).
     *  @note The clipping only shrinks: the budget released from a clipped instrument is not
     *        redistributed to the others, so after the total budget cap and this per-instrument
     *        cap the final gross exposure may stay well below max_total_position. */
    virtual void _checkRisk(const Datetime& date, const TradeManagerPtr& tm,
                            TradeSuggestionList& suggestions, const KQuery& query,
                            const string& mode);

    /** Subclass reset interface */
    virtual void _reset() {}

    /** Interface for the subclass to clone its private variables */
    virtual AllocateFundsPtr _clone() = 0;

    bool isPythonObject() const noexcept {
        return m_is_python_object;
    }

    /** Whether this component or its trade manager is defined in Python; overrides may recurse */
    virtual bool hasPythonPart() const noexcept {
        return m_is_python_object || (m_tm && m_tm->hasPythonPart());
    }

protected:
    string m_name;
    KQuery m_query;
    TradeManagerPtr m_tm;
    bool m_is_python_object{false};

    // The one-shot notices are per instance, not per process: a shared flag would hide the
    // diagnostics of every other portfolio / backtest once the first instance warned
    mutable bool m_warned_fixed_amount{false};
    mutable bool m_warned_short_unsupported{false};

//============================================
// Serialization support
//============================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    // The current schema only; no compatibility with the older archives. m_query and m_tm are set
    // temporarily when the system runs, they do not need to be serialized
    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& BOOST_SERIALIZATION_NVP(m_name);
        ar& BOOST_SERIALIZATION_NVP(m_params);
        ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    }
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

#define ALLOCATE_FUNDS_IMP(classname)            \
public:                                          \
    virtual AllocateFundsPtr _clone() override { \
        return std::make_shared<classname>();    \
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
