/*
 * MomentResult.h
 *
 *  Recursive combination refactoring: the complete running result (suggestion) of a system instance at a certain moment
 *  The single-security and the aggregate forms return the [completely identical] structure, which is the premise of the nesting.
 */

#pragma once
#ifndef MOMENTRESULT_H_
#define MOMENTRESULT_H_

#include <vector>
#include <string>
#include "../../DataType.h"
#include "../../Stock.h"
#include "../../trade_manage/TradeRecord.h"
#include "../../trade_manage/FundsRecord.h"
#include "../../trade_manage/PositionRecord.h"
#include "TradeRequest.h"
#include "TradeSuggestion.h"
#include <nlohmann/json.hpp>

namespace hku {

using json = nlohmann::json;

/**
 * The complete running result (suggestion) of a system instance at a certain moment
 * @note The single-security and the aggregate forms return the [completely identical] structure, which is the premise of the nesting
 * @note It is the runtime intermediate data, not participating in the serialization
 */
struct HKU_API MomentResult {
    Datetime datetime;                  // The corresponding moment
    FundsRecord funds_before_open;      // The [before open trade] snapshot -> the denominator of the tradesOnOpen ratio
    FundsRecord funds_before_close;     // The [before close trade] snapshot (= after the open execution) -> the denominator of the tradesOnClose ratio
    FundsRecord funds;                  // The final balance / total assets / market value at this moment (including the long/short and the margin trading)
    std::vector<PositionRecord> positions;  // The current positions (multiple instruments for the aggregate form; lazily filled)
    TradeRecordList tradesOnOpen;       // The trades executed at the [open] stage
    TradeRecordList tradesOnClose;      // The trades executed at the [close] stage
    std::vector<TradeRequest> delayOnNextOpen;  // The requests delayed to the open of the next moment
    TradeSuggestionList suggestions;    // The suggestion instructions produced at this moment (the complete semantics)

    json ext;                           // The extensible area (lazily constructed, an empty object by default)

    TradeRecordList allTrades() const {
        TradeRecordList r = tradesOnOpen;
        r.insert(r.end(), tradesOnClose.begin(), tradesOnClose.end());
        return r;
    }

    bool empty() const {
        return tradesOnOpen.empty() && tradesOnClose.empty() && suggestions.empty();
    }

    json toJson() const {
        json j;
        j["datetime"] = datetime.str();
        auto append_trade = [](json& arr, const TradeRecord& tr) {
            json r;
            r["datetime"] = tr.datetime.str();
            r["number"] = tr.number;
            r["realPrice"] = tr.realPrice;
            r["from"] = static_cast<int>(tr.from);
            arr.push_back(r);
        };
        json open = json::array();
        for (const auto& tr : tradesOnOpen) {
            append_trade(open, tr);
        }
        j["tradesOnOpen"] = open;
        json close = json::array();
        for (const auto& tr : tradesOnClose) {
            append_trade(close, tr);
        }
        j["tradesOnClose"] = close;
        j["suggestions"] = suggestions.size();
        j["ext"] = ext;
        return j;
    }

    template <typename T>
    T get(const string& key, const T& def) const {
        if (ext.is_object()) {
            auto it = ext.find(key);
            if (it != ext.end()) {
                return it->get<T>();
            }
        }
        return def;
    }

    template <typename T>
    void set(const string& key, const T& value) {
        ext[key] = value;
    }
};

}  // namespace hku

#endif /* MOMENTRESULT_H_ */
