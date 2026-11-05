/*
 * ByTypeMargin.h
 *
 *  Created on: 2026-11-03
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_IMP_BY_TYPE_MARGIN_H_
#define TRADE_MANAGE_IMP_BY_TYPE_MARGIN_H_

#include <unordered_map>
#include "../MarginBase.h"

namespace hku {

/**
 * Margin model keyed by the security type or the market, for a mixed account
 * @details The rates expression is a list of "key:ratio" items separated by ",", ";" or blanks,
 *          e.g. "12:0.10, SHF:0.12". An all-digit key is a security type, any other key is a
 *          market code (compared upper-cased). A type hit wins over a market hit; when neither
 *          hits, default_rate is used. The long and the short share one ratio; use MG_Fixed or a
 *          Python subclass when they must differ.
 * @param rates the "key:ratio" list
 * @param maintain_rate maintain margin ratio, 0 means the same as the opening margin
 * @param default_rate ratio of a security matched by no key, (0, 1], 0.6 by default
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

    virtual MarginPtr _clone() override;

protected:
    virtual void _checkParam(const string& name) const override;

private:
    /** Parse the rates expression into the lookup maps; called from _checkParam only, so a query
     *  method never parses nor throws and a shared instance stays read-only at run time */
    void parseRates() const;

    /** The ratio of the given security, default_rate when no key matches */
    double rateOf(const Stock& stock) const;

    // The maps are derived from the rates param, hence they are not serialized; they are rebuilt by
    // parseRates on every param check, i.e. on set_param, on clone and on deserialization
    mutable string m_parsed_src;
    mutable std::unordered_map<uint32_t, double> m_type_rates;
    mutable std::unordered_map<string, double> m_market_rates;
};

} /* namespace hku */

#endif /* TRADE_MANAGE_IMP_BY_TYPE_MARGIN_H_ */
