/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#pragma once
#include "../System.h"

namespace hku {

class HKU_API DelegateSystem : public System {
public:
    DelegateSystem() = default;
    explicit DelegateSystem(const string& name) : System(name) {}
    explicit DelegateSystem(const SystemPtr& sys) : m_sys(sys) {}
    virtual ~DelegateSystem() = default;

    virtual void run(const KData& kdata, bool reset = true, bool resetAll = false) override;
    virtual TradeRecord runMoment(const Datetime& datetime) override;

    virtual void _reset() override;
    virtual void _forceResetAll() override;
    virtual SystemPtr _clone() override;

public:
    virtual TradeRecord sellForceOnOpen(const Datetime& date, double num, Part from) override;
    virtual TradeRecord sellForceOnClose(const Datetime& date, double num, Part from) override;
    virtual void clearDelayBuyRequest() override;
    virtual bool haveDelaySellRequest() const override;
    virtual TradeRecord pfProcessDelaySellRequest(const Datetime& date) override;

private:
    SystemPtr m_sys;

//========================================
// 序列化支持
//========================================
#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(System);
        ar& BOOST_SERIALIZATION_NVP(m_sys);
    }
#endif /* HKU_SUPPORT_SERIALIZATION */
};

}  // namespace hku