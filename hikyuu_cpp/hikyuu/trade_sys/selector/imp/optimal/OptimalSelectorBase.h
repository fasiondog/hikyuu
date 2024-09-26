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

    virtual void _reset() override;

    virtual string str() const override;

    // 以便继承子类只需要实现 _clone 和 该接口即可
    // 该接口实现系统绩效评估，getSelected 时将取评估结果最大的系统
    // 使用 std::function 的话，在 C++ 中无法序列化，所以使用继承
    virtual double evaluate(const SYSPtr& sys, const Datetime& endDate) noexcept {
        return Null<double>();
    }

    const vector<RunRanges>& getRunRanges() const {
        return m_run_ranges;
    }

private:
    void _initParams();
    void _calculate_single(const vector<std::pair<size_t, size_t>>& train_ranges,
                           const DatetimeList& dates, size_t test_len, bool trace);

    void _calculate_parallel(const vector<std::pair<size_t, size_t>>& train_ranges,
                             const DatetimeList& dates, size_t test_len, bool trace);

protected:
    unordered_map<Datetime, std::shared_ptr<SystemWeightList>> m_sys_dict;
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