/*
 * PF_Mode.h
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-17
 *      Author: fasiondog
 *
 *  The alphabetical alias factories of the three running modes. Pure forwarders: the default
 *  arguments live only in the semantic-name factories (single source, no per-alias sync), so an
 *  alias accepts exactly the argument list of its real factory, partial calls included. See the
 *  real factory for the parameters; the Python bindings expose the real factories under the same
 *  alias names. Each alias is also exported into the part registry as "core.pf.modea" /
 *  "core.pf.modeb" / "core.pf.modec", registered as a copy of its semantic part's meta so that
 *  the two entries cannot drift apart.
 */

#pragma once
#ifndef TRADE_SYS_PORTFOLIO_CRT_PF_MODE_H_
#define TRADE_SYS_PORTFOLIO_CRT_PF_MODE_H_

#include "PF_SignalAggregate.h"
#include "PF_Simple.h"
#include "PF_WithoutAF.h"

namespace hku {

/** Alias of PF_SignalAggregate: mode A "Signal Aggregation" @ingroup Portfolio */
template <typename... Args>
inline MultiSystemPtr PF_ModeA(Args&&... args) {
    return PF_SignalAggregate(std::forward<Args>(args)...);
}

/** Alias of PF_Simple: mode B "Fund Allocation" @ingroup Portfolio */
template <typename... Args>
inline MultiSystemPtr PF_ModeB(Args&&... args) {
    return PF_Simple(std::forward<Args>(args)...);
}

/** Alias of PF_WithoutAF: mode C "Shared Account Compatibility" @ingroup Portfolio */
template <typename... Args>
inline MultiSystemPtr PF_ModeC(Args&&... args) {
    return PF_WithoutAF(std::forward<Args>(args)...);
}

/** Register the "core.pf.modea" part, a thin alias of "core.pf.signalaggregate" */
void registerPart_PF_ModeA();

/** Register the "core.pf.modeb" part, a thin alias of "core.pf.simple" */
void registerPart_PF_ModeB();

/** Register the "core.pf.modec" part, a thin alias of "core.pf.withoutaf" */
void registerPart_PF_ModeC();

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_CRT_PF_MODE_H_ */
