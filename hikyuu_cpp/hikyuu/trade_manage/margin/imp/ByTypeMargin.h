/*
 * ByTypeMargin.h
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_BY_TYPE_MARGIN_H_
#define TRADE_MANAGE_IMP_BY_TYPE_MARGIN_H_

#include "../../RateTable.h"
#include "../MarginBase.h"

namespace hku {

/**
 * Margin model keyed by security type or market, for a mixed account
 * @details "key:ratio" items separated by ",", ";" or blanks. All-digit keys are security types,
 *          others are market codes (upper-cased). A type hit wins over a market hit; neither uses
 *          default_rate. Long and short share one ratio; use MG_Fixed or a Python subclass to differ.
 * @param rates the "key:ratio" list
 * @param maintain_rate maintain margin ratio, 0 means the same as the opening margin
 * @param default_rate ratio when no key matches, (0, 1], 0.6 by default
 * @param collateral_rates "key:ratio" list of collateral conversion rates, same syntax; empty
 *        counts every holding in full
 * @param default_collateral_rate conversion rate when no collateral key matches, (0, 1], 1.0 by default
 * @ingroup Margin
 */
class HKU_API ByTypeMargin : public MarginBase {
    MARGIN_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ByTypeMargin();
    virtual ~ByTypeMargin();

    virtual bool isMarginTrading(const Stock& stock) const override;

    virtual price_t getOpenMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num, MarginSide side) const override;

    virtual price_t getMaintainMargin(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num, MarginSide side) const override;

    virtual double getMarginRate(const Datetime& datetime, const Stock& stock,
                                 MarginSide side) const override;

    /** The collateral conversion rate of the security, one table for both sides */
    virtual double getCollateralRate(const Datetime& datetime, const Stock& stock,
                                     MarginSide side) const override;

    virtual MarginPtr _clone() override;

protected:
    virtual void _checkParam(const string& name) const override;

private:
    /** Parse both tables from _checkParam only, so queries never parse or throw and a shared
     *  instance stays read-only; each table reparses only when its own expression changes */
    void parseRates() const;

    /** The ratio of the given security, default_rate when no key matches */
    double rateOf(const Stock& stock) const;

    /** The conversion rate of the given security, default_collateral_rate when no key matches */
    double collateralRateOf(const Stock& stock) const;

    // Derived from the rate params, so not serialized; parseRates rebuilds them on every param
    // check: set_param, clone and deserialization
    mutable RateTable m_rates;
    mutable RateTable m_collateral_rates;
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_BY_TYPE_MARGIN_H_ */
