/*
 * SubSystemContext.h
 *
 *  递归组合重构：MM L1 上下文（含模式 B 额度）
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

class System;  // 前向声明，避免与 System.h 形成包含环（System.h 已包含 MoneyManagerBase.h）

/**
 * MM L1 上下文（含模式 B 额度）
 * @note 聚合 System 在驱动子系统前，构造该上下文供 MM 进行系统级分配
 */
struct HKU_API SubSystemContext {
    std::shared_ptr<System> sys;
    FundsRecord funds;          // 子系统虚拟账户当前资金
    PriceList profit_curve;     // 至当前的收益曲线（父缓存 + 增量追加）
    double total_return{0.0};
    double current_weight{0.0};  // 该子系统当前在父账户中的实际占比
    price_t quota{0.0};          // 【模式 B】分配给该子系统的额度（写入其虚拟账户）
    size_t suggestion_count{0};
    json ext;
};

typedef std::vector<SubSystemContext> SubSystemContextList;

}  // namespace hku

#endif /* SUBSYSTEMCONTEXT_H_ */
