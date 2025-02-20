/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-20
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_FIXEDCAPITAL_FUNDS_MM_H_
#define TRADE_SYS_MONEYMANAGER_IMP_FIXEDCAPITAL_FUNDS_MM_H_

#include "../MoneyManagerBase.h"

namespace hku {

class FixedCountTpsMM : public MoneyManagerBase {
public:
    FixedCountTpsMM();
    FixedCountTpsMM(const vector<double>& buy_counts, const vector<double>& sell_counts);
    virtual ~FixedCountTpsMM();

    virtual MoneyManagerPtr _clone() override;
    virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                 price_t risk, SystemPart from) override;
    virtual double _getSellNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                  price_t risk, SystemPart from) override;

private:
    vector<double> m_buy_counts;
    vector<double> m_sell_counts;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MoneyManagerBase);
    }
#endif
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_FIXEDCAPITAL_FUNDS_MM_H_ */
