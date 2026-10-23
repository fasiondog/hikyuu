/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#pragma once
#include "hikyuu/trade_sys/selector/SelectorBase.h"

namespace hku {

//
// start ------ run_start --end
// | train_len | test_len
//
struct RunRanges {
    Datetime start;
    Datetime run_start;
    Datetime end;

    RunRanges(const Datetime& start_, const Datetime& run_start_, const Datetime end_)
    : start(start_), run_start(run_start_), end(end_) {}
};

// The prototype index (mapped back to the running systems by identity) plus the window-local
// trained copy (run directly by SYS_WalkForward)
struct OptimalSelectedItem {
    size_t index{0};
    double weight{0.0};
    SYSPtr trained;
};
using OptimalSelectedList = vector<OptimalSelectedItem>;

class HKU_API OptimalSelectorBase : public SelectorBase {
    CLASS_LOGGER_IMP(SE_Optimal)
    SELECTOR_IMP(OptimalSelectorBase)
    SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    OptimalSelectorBase();
    explicit OptimalSelectorBase(const string& name);
    virtual ~OptimalSelectorBase() = default;

    virtual void _checkParam(const string& name) const override;
    virtual void calculate(const SystemList& pf_realSysList, const KQuery& query) override;
    virtual void bindRealToProto(const SYSPtr& real, const SYSPtr& proto) override;

    virtual void _reset() override;

    virtual string str() const override;

    // So that an inheriting subclass only needs to implement _clone and this interface
    // This interface implements the system performance evaluation; getSelected takes the system
    // with the largest evaluation result std::function cannot be serialized in C++, so inheritance
    // is used When Null<double> is returned it will not be selected by the evaluation
    virtual double evaluate(const SYSPtr& sys, const Datetime& endDate) noexcept {
        return Null<double>();
    }

    const vector<RunRanges>& getRunRanges() const {
        return m_run_ranges;
    }

private:
    void _initParams();

protected:
    /**
     * Validate the candidate list and split the back-test range into the walk-forward train/test
     * windows. Shared by the optimal selectors so that the two implementations cannot drift apart.
     * @param query the back-test query
     * @param dates [out] the trading calendar of the query
     * @param train_ranges [out] the [train_start, train_end) index pairs
     * @return false when the calculation must stop (empty candidate list or a candidate bound to a
     *         security of its own)
     */
    bool _prepareCalculate(const KQuery& query, DatetimeList& dates,
                           vector<std::pair<size_t, size_t>>& train_ranges) const;

    SYSPtr _selectedSystem(size_t candIndex, const SYSPtr& trained) const;

    /**
     * Clone the candidate prototypes for the training so that the parallel back-tests do not share
     * the EV / TM of the prototypes. Shared by the optimal selectors to keep the isolation rule in
     * one place.
     */
    SystemList _cloneProtosForTraining() const;

    /**
     * Fill m_sys_dict / m_run_ranges from the per-range selection result and log the picked system.
     */
    void _applySelected(const vector<std::shared_ptr<OptimalSelectedList>>& sys_list,
                        const DatetimeList& dates,
                        const vector<std::pair<size_t, size_t>>& train_ranges, size_t test_len,
                        bool trace);

    /**
     * Run the parallel training over the train ranges. The subclasses override it only to supply
     * their evaluation input (e.g. the performance key / mode); the range splitting and the result
     * bookkeeping stay in the base class.
     */
    virtual void _calculate_parallel(const vector<std::pair<size_t, size_t>>& train_ranges,
                                     const DatetimeList& dates, size_t test_len, bool trace);

    unordered_map<const System*, SYSPtr> m_proto_to_real;
    unordered_map<Datetime, std::shared_ptr<OptimalSelectedList>> m_sys_dict;
    vector<RunRanges> m_run_ranges;
};

#if HKU_SUPPORT_SERIALIZATION
#define OPTIMAL_SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION       \
private:                                                       \
    friend class boost::serialization::access;                 \
    template <class Archive>                                   \
    void serialize(Archive& ar, const unsigned int version) {  \
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase); \
    }
#else
#define OPTIMAL_SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define OPTIMAL_SELECTOR_IMP(classname)       \
public:                                       \
    virtual SelectorPtr _clone() override {   \
        return std::make_shared<classname>(); \
    }                                         \
    virtual double evaluate(const SYSPtr&, const Datetime& endDate) noexcept override;

}  // namespace hku