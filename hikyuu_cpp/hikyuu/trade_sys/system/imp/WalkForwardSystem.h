/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/selector/SelectorBase.h"
#include "hikyuu/trade_sys/selector/imp/optimal/OptimalSelectorBase.h"
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
    virtual MomentResult runMoment(const Datetime& datetime) override;
    virtual MomentResult runMomentOnOpen(const Datetime& datetime) override;
    virtual MomentResult runMomentOnClose(const Datetime& datetime) override;

    virtual void _checkParam(const string& name) const override;
    virtual void _reset() override;
    virtual void _forceResetAll() override;
    virtual SystemPtr _clone() override;

    virtual string str() const override;

public:
    virtual bool hasPythonPart() const override;
    virtual TradeRecord sellForceOnOpen(const Datetime& date, double num, Part from) override;
    virtual TradeRecord sellForceOnClose(const Datetime& date, double num, Part from) override;
    virtual void clearDelayBuyRequest() override;
    virtual TradeRecord pfProcessDelaySellRequest(const Datetime& date) override;

private:
    void initParam();
    void syncDataFromSystem(const SYSPtr&, bool isMoment);
    void syncDataToSystem(const SYSPtr&);

    /** Build the window table (RunRanges) and the lazily-filled window KData from the SE */
    void ensureWindows();
    /** The [start, end) KData of the given window, built on first use */
    KData windowKDataAt(size_t index);
    /** The index of the run window covering datetime, or Null when none */
    size_t windowIndexAt(const Datetime& datetime) const;
    /** Shared driving entry: switch the window / candidate / view TM, then run the candidate */
    MomentResult driveMoment(const Datetime& datetime, bool open, bool close);
    /** The original (unadjusted) open price of the given moment, 0 when there is no data */
    price_t srcOpenPriceAt(const Datetime& datetime);

private:
    SEPtr m_se;        // The optimizing SE
    TMPtr m_train_tm;  // The account used for the optimization evaluation calculation
    SYSPtr m_cur_sys;
    size_t m_cur_window{Null<size_t>()};  // The window index the current candidate was set up for
    vector<KData>
      m_train_kdata_list;  // The [start, end) window KData of each RunRanges entry, built lazily
    vector<RunRanges> m_run_ranges;

//========================================
// Serialization support
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