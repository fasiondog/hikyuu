/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/selector/SelectorBase.h"
#include "../System.h"

namespace hku {

class HKU_API WalkForwardSystem : public System {
    CLASS_LOGGER_IMP(SYS_WalkForward)

public:
    WalkForwardSystem();
    WalkForwardSystem(const SystemList& candidate_sys_list, const SelectorPtr& se,
                      const TradeManagerPtr& train_tm);
    virtual ~WalkForwardSystem() = default;

    virtual void readyForRun() override;
    virtual void run(const KData& kdata, bool reset = true, bool resetAll = false) override;
    virtual TradeRecord runMoment(const Datetime& datetime) override;

    virtual void _checkParam(const string& name) const override;
    virtual void _reset() override;
    virtual void _forceResetAll() override;
    virtual SystemPtr _clone() override;

    virtual string str() const override;

public:
    virtual TradeRecord sellForceOnOpen(const Datetime& date, double num, Part from) override;
    virtual TradeRecord sellForceOnClose(const Datetime& date, double num, Part from) override;
    virtual void clearDelayBuyRequest() override;
    virtual bool haveDelaySellRequest() const override;
    virtual TradeRecord pfProcessDelaySellRequest(const Datetime& date) override;

private:
    void initParam();
    void syncDataFromSystem(const SYSPtr&, bool isMoment);
    void syncDataToSystem(const SYSPtr&);

private:
    SEPtr m_se;        // 寻优SE
    TMPtr m_train_tm;  // 用于优化评估计算的账户
    SYSPtr m_cur_sys;
    size_t m_cur_kdata{0};
    vector<KData> m_train_kdata_list;
    vector<RunRanges> m_run_ranges;

//========================================
// 序列化支持
//========================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(System);
        ar& BOOST_SERIALIZATION_NVP(m_se);
        ar& BOOST_SERIALIZATION_NVP(m_train_tm);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

}  // namespace hku