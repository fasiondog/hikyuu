/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-10-10
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/EXIST.h"
#include "hikyuu/trade_sys/signal/crt/SG_Bool.h"
#include "combinate.h"

namespace hku {

std::vector<Indicator> HKU_API combinateIndicator(const std::vector<Indicator>& inputs, int n) {
    std::vector<Indicator> ret;
    auto indexs = combinateIndex(inputs);
    for (size_t i = 0, len = indexs.size(); i < len; i++) {
        size_t count = indexs[i].size();
        Indicator tmp = EXIST(inputs[indexs[i][0]], n);
        std::string name = inputs[indexs[i][0]].name();
        for (size_t j = 1; j < count; j++) {
            tmp = tmp & EXIST(inputs[indexs[i][j]], n);
            name = fmt::format("{} & {}", name, inputs[indexs[i][j]].name());
        }
        tmp.name(name);
        ret.emplace_back(tmp);
    }
    return ret;
}

std::map<std::string, Performance> HKU_API combinateIndicatorAnalysis(
  const Stock& stk, const KQuery& query, TradeManagerPtr tm, SystemPtr sys,
  const std::vector<Indicator>& buy_inds, const std::vector<Indicator>& sell_inds, int n) {
    auto inds = combinateIndicator(buy_inds, n);
    std::vector<SignalPtr> sgs;
    for (const auto& buy_ind : inds) {
        for (const auto& sell_ind : sell_inds) {
            auto sg = SG_Bool(buy_ind, sell_ind);
            sg->name(fmt::format("{} + {}", buy_ind.name(), sell_ind.name()));
            sgs.emplace_back(sg);
        }
    }

    std::map<std::string, Performance> result;
    for (const auto& sg : sgs) {
        sys->setSG(sg);
        sys->setTM(tm);
        sys->run(stk, query);
        Performance per;
        per.statistics(tm, Datetime::now());
        result[sg->name()] = std::move(per);
    }

    return result;
}

}  // namespace hku