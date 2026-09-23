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
    PriceList profit_curve;     // The profit curve up to now (the parent cache + the incremental append)
    double total_return{0.0};
    double current_weight{0.0};  // The actual proportion of this sub-system in the parent account
    double score{0.0};           // The SE score on the rebalancing day (0 on the non-rebalancing days / for the unselected), used by AF_MultiFactor etc. to take the scores as the weights
    price_t quota{0.0};          // [Mode B] the quota allocated to this sub-system (written into its virtual account)
    size_t suggestion_count{0};
    json ext;
};

typedef std::vector<SubSystemContext> SubSystemContextList;

}  // namespace hku

#endif /* SUBSYSTEMCONTEXT_H_ */
