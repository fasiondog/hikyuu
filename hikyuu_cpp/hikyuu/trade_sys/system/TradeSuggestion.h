/*
 * TradeSuggestion.h
 *
 *  Recursive combination refactoring: the suggestion instruction with the complete semantic expression (without normalization)
 */

#pragma once
#ifndef TRADESUGGESTION_H_
#define TRADESUGGESTION_H_

#include <vector>
#include <string>
#include "../../DataType.h"
#include "SystemPart.h"
#include "../../Stock.h"
#include <nlohmann/json.hpp>

namespace hku {

using json = nlohmann::json;

class System;  // Forward declaration, to avoid a circular include with System.h

/** The suggestion type */
enum class SuggestionType { HOLD = 0, BUY, SELL, CLEAR };

/**
 * The suggestion instruction with the complete semantic expression (without normalization)
 * @note It carries the raw absolute quantity calculated by MM on the sub-system virtual account, and marks the three ratios
 */
struct HKU_API TradeSuggestion {
    Stock stock;
    std::shared_ptr<System> sys;  // The source sub-system (nestable, a direct sub-system in the aggregate form)
    SuggestionType type{SuggestionType::HOLD};

    double number{0.0};       // The raw quantity (not scaled); MAX_DOUBLE means full close
    price_t plan_price{0.0};
    price_t plan_cash{0.0};   // = number * plan_price

    // false: the long side (BUY opens, SELL reduces, CLEAR closes all); true: the short side
    // (SELL opens a short, BUY covers, CLEAR closes the whole short). The number stays positive
    // and the type carries the direction.
    bool short_side{false};

    double cash_ratio{0.0};            // plan_cash / the cash balance before this stage's trade (magnitude)
    double assets_ratio{0.0};          // The increment ratio: plan_cash / the net assets before this
                                       // stage's trade; the sign encodes direction (> 0 buy/cover,
                                       // < 0 sell/open-short)
    double target_position_ratio{0.0}; // The signed net target level: the target net position market
                                       // value after the trade / the net assets; defined for all types
                                       // (0 means flat), the sign encodes long/short

    price_t stoploss{0.0};
    price_t goalPrice{0.0};
    SystemPart from{PART_SIGNAL};  // PART_SYSTEM when it comes from a sub-system
    int urgency{0};                // 0 - executed at the current close; 1 - executed immediately at the next open
    double score{0.0};             // The signal strength, used for the SE / MM sorting
    string remark;
    json ext;
};

typedef vector<TradeSuggestion> TradeSuggestionList;

}  // namespace hku

#endif /* TRADESUGGESTION_H_ */
