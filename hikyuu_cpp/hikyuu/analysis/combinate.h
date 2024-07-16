/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/trade_sys/system/System.h"
#include "hikyuu/trade_manage/Performance.h"
#include "hikyuu/utilities/Log.h"

namespace hku {

/**
 * 获取指定序列索引的组合，返回由序列下标索引组成的组合集合
 * @note 组合的总数是 2**n - 1 个，所以inputs的长度需要控制
 * @exception 输入序列的最大长度仅支持15，超过时将抛出异常 HKUException
 * @param inputs 任意类型的序列
 * @return
 */
template <class T>
std::vector<std::vector<size_t>> combinateIndex(const std::vector<T>& inputs) {
    size_t total = inputs.size();
    HKU_CHECK(total <= 15, "The length of inputs exceeds the maximum limit!");

    std::vector<std::vector<size_t>> result;
    std::vector<size_t> current;
    for (size_t i = 0; i < total; i++) {
        for (size_t j = 0, len = result.size(); j < len; j++) {
            current.resize(result[j].size());
            std::copy(result[j].cbegin(), result[j].cend(), current.begin());
            current.push_back(i);
            result.push_back(std::move(current));
        }
        current.clear();
        current.push_back(i);
        result.push_back(std::move(current));
    }

    return result;
}

/**
 * @brief 对输入的指标序列进行组合
 * @details
 * 如输入为 [ind1, ind2], 输出为 [EXIST(ind1,n), EXIST(ind2,n),
 * EXIST(ind1,n)&EXIST(ind2,n)]
 * @param inputs 待组合的指标列表
 * @param n 指标在 n 周期内存在
 * @return std::vector<Indicator>
 */
std::vector<Indicator> HKU_API combinateIndicator(const std::vector<Indicator>& inputs, int n);

/**
 * @brief 对指定的证券及买入、卖出信号指标进行组合测试
 * @param stk 指定证券
 * @param query 指定范围条件
 * @param tm 指定交易管理实例
 * @param sys 指定交易策略实例
 * @param buy_inds 买入信号指标列表
 * @param sell_inds 卖出信号指标列表
 * @param n 组合时间范围参数
 * @return std::map<std::string, Performance>
 */
std::map<std::string, Performance> HKU_API combinateIndicatorAnalysis(
  const Stock& stk, const KQuery& query, TradeManagerPtr tm, SystemPtr sys,
  const std::vector<Indicator>& buy_inds, const std::vector<Indicator>& sell_inds, int n);

/**
 * @brief combinateIndicatorAnalysisWithBlock 输出结果定义
 */
struct HKU_API CombinateAnalysisOutput {
    string combinateName;  ///< 买入、卖出指标组合名称
    string market_code;    ///< 证券代码
    string name;           ///< 证券名称
    PriceList values;      ///< 统计各项指标值

    CombinateAnalysisOutput() = default;
    CombinateAnalysisOutput(const CombinateAnalysisOutput&) = default;
    CombinateAnalysisOutput(CombinateAnalysisOutput&& rv)
    : combinateName(std::move(rv.combinateName)),
      market_code(std::move(rv.market_code)),
      name(std::move(rv.name)),
      values(std::move(rv.values)) {}

    CombinateAnalysisOutput& operator=(const CombinateAnalysisOutput&) = default;
    CombinateAnalysisOutput& operator=(CombinateAnalysisOutput&& rv) {
        HKU_IF_RETURN(this == &rv, *this);
        combinateName = std::move(rv.combinateName);
        market_code = std::move(rv.market_code);
        name = std::move(rv.name);
        values = std::move(rv.values);
        return *this;
    }
};

/**
 * @brief 对指定的证券列表，进行买入、卖出信号指标组合测试
 * @param blk 指定板块
 * @param query 指定范围条件
 * @param tm 指定交易实例
 * @param sys 指定系统策略实例
 * @param buy_inds 买入指标列表
 * @param sell_inds 卖出指标列表
 * @param n 组合时间范围参数
 * @return vector<CombinateAnalysisOutput>
 */
vector<CombinateAnalysisOutput> HKU_API combinateIndicatorAnalysisWithBlock(
  const Block& blk, const KQuery& query, TradeManagerPtr tm, SystemPtr sys,
  const std::vector<Indicator>& buy_inds, const std::vector<Indicator>& sell_inds, int n);

}  // namespace hku