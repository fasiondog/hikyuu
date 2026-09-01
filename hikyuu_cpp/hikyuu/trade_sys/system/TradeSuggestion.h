/*
 * TradeSuggestion.h
 *
 *  递归组合重构：完整语义表达的建议指令（不做归一化）
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

class System;  // 前向声明，避免与 System.h 形成包含环

/** 建议类型 */
enum class SuggestionType { HOLD = 0, BUY, SELL, CLEAR };

/**
 * 完整语义表达的建议指令（不做归一化）
 * @note 携带子系统虚拟账户上 MM 算出的原始绝对数量，并标注三比重
 */
struct HKU_API TradeSuggestion {
    Stock stock;
    std::shared_ptr<System> sys;  // 来源子系统（可嵌套，聚合形态下为直接子系统）
    SuggestionType type{SuggestionType::HOLD};

    double number{0.0};       // 原始数量（不缩放）；MAX_DOUBLE 表示全平
    price_t plan_price{0.0};
    price_t plan_cash{0.0};   // = number * plan_price

    double cash_ratio{0.0};            // plan_cash / 本阶段交易前现金余额
    double assets_ratio{0.0};          // plan_cash / 本阶段交易前总资产
    double target_position_ratio{0.0}; // 交易后目标持仓市值 / 总资产

    price_t stoploss{0.0};
    price_t goalPrice{0.0};
    SystemPart from{PART_SIGNAL};  // 来自子系统时为 PART_SYSTEM(=11)
    int urgency{0};                // 0-当前收盘执行；1-下一开盘立即执行
    double score{0.0};             // 信号强度，供 SE / MM 排序
    string remark;
    json ext;
};

typedef vector<TradeSuggestion> TradeSuggestionList;

}  // namespace hku

#endif /* TRADESUGGESTION_H_ */
