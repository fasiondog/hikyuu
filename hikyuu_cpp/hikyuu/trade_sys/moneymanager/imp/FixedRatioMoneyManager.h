/*
 * FixedRatioMoneyManager.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_FIXEDRATIOMONEYMANAGER_H_
#define TRADE_SYS_MONEYMANAGER_IMP_FIXEDRATIOMONEYMANAGER_H_

#include "../MoneyManagerBase.h"

namespace hku {

class FixedRatioMoneyManager : public MoneyManagerBase {
public:
    FixedRatioMoneyManager();
    virtual ~FixedRatioMoneyManager();

    virtual void _checkParam(const string& name) const override;
    virtual void _reset() override;
    virtual MoneyManagerPtr _clone() override;
    virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                 price_t risk, SystemPart from) override;

private:
    int m_current_num;
    price_t m_pre_cash;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MoneyManagerBase);
        ar& BOOST_SERIALIZATION_NVP(m_current_num);
        ar& BOOST_SERIALIZATION_NVP(m_pre_cash);
    }

#endif /* HKU_SUPPORT_SERIALIZATION */
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_FIXEDRATIOMONEYMANAGER_H_ */
