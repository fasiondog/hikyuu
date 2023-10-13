/*
 * BandSignal.h
 *
 *   Created on: 2023年09月23日
 *       Author: yangrq1018
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_IMP_BANDSIGNAL_H_
#define TRADE_SYS_SIGNAL_IMP_BANDSIGNAL_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

class BandSignal : public SignalBase {
public:
    BandSignal();
    BandSignal(const Indicator& sig, price_t lower, price_t upper);
    virtual ~BandSignal();

    virtual SignalPtr _clone() override;
    virtual void _calculate() override;

private:
    Indicator m_ind;
    price_t m_lower, m_upper;
};
}  // namespace hku

#endif