/*
 * MomentResult.h
 *
 *  递归组合重构：某一时刻、某一系统实例的完整运行结果（建议）
 *  单证券与聚合形态返回【完全相同】的结构，这是嵌套的前提。
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
 * 某一时刻、某一系统实例的完整运行结果（建议）
 * @note 单证券与聚合形态返回【完全相同】的结构，这是嵌套的前提
 * @note 运行期中间数据，不参与序列化
 */
struct HKU_API MomentResult {
    Datetime datetime;                  // 对应时刻
    FundsRecord funds_before_open;      // 【开盘交易前】快照 → tradesOnOpen 比重的分母
    FundsRecord funds_before_close;     // 【收盘交易前】快照（= open 执行后）→ tradesOnClose 比重的分母
    FundsRecord funds;                  // 本时刻最终余额 / 总资产 / 市值（含多空与融资融券）
    std::vector<PositionRecord> positions;  // 当前持仓（聚合形态为多标的；惰性填充）
    TradeRecordList tradesOnOpen;       // 【开盘】阶段执行的交易
    TradeRecordList tradesOnClose;      // 【收盘】阶段执行的交易
    std::vector<TradeRequest> delayOnNextOpen;  // 延迟至下一时刻开盘执行的请求
    TradeSuggestionList suggestions;    // 本时刻产生的建议指令（完整语义）

    json ext;                           // 可扩展区（惰性构造，默认空对象）

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
