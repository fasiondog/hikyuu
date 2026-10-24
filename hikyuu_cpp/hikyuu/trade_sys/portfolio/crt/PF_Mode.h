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
 *  alias names.
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

} /* namespace hku */

#endif /* TRADE_SYS_PORTFOLIO_CRT_PF_MODE_H_ */
