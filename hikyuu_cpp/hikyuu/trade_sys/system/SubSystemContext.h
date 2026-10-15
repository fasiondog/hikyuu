/*
 * SubSystemContext.h
 *
 *  Recursive combination refactoring: the L1 context of AF (including the mode B quota)
 */

#pragma once
#ifndef SUBSYSTEMCONTEXT_H_
#define SUBSYSTEMCONTEXT_H_

#include <vector>
#include <string>
#include <memory>
#include "../../DataType.h"
#include "../../trade_manage/FundsRecord.h"
#include <nlohmann/json.hpp>

namespace hku {

using json = nlohmann::json;

class System;  // Forward declaration, to avoid a circular include with System.h (System.h already includes MoneyManagerBase.h)

/**
 * The L1 context of AF (including the mode B quota)
 * @note Before driving the sub-systems, the aggregate System constructs this context for AF to perform the system-level allocation
 */
struct HKU_API SubSystemContext {
    std::shared_ptr<System> sys;
    FundsRecord funds;          // The current funds of the sub-system virtual account
    PriceList profit_curve;     // Reserved for future AF extensions; not filled by MultiSystem
    double total_return{0.0};   // Mode A shadow-account cumulative return over m_sub_init_cash;
                                // 0 in mode B (the quota-injected model has no fixed init cash)
    double current_weight{0.0};  // The scale of this sub-system's account over the parent's real
                                 // total assets
    double score{0.0};           // The SE score on the rebalancing day (0 on the non-rebalancing days / for the unselected), used by AF_MultiFactor etc. to take the scores as the weights
    price_t quota{0.0};          // [Mode B] the quota allocated to this sub-system (written into its virtual account)
    size_t suggestion_count{0};  // Reserved for future AF extensions; not filled by MultiSystem
    json ext;
};

typedef std::vector<SubSystemContext> SubSystemContextList;

}  // namespace hku

#endif /* SUBSYSTEMCONTEXT_H_ */
