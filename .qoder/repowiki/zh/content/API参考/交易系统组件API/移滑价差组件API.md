# 移滑价差组件API

<cite>
**本文档引用文件**  
- [SP_FixedPercent.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_FixedPercent.h)
- [SP_FixedValue.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_FixedValue.h)
- [SP_Normal.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_Normal.h)
- [SP_LogNormal.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_LogNormal.h)
- [SP_TruncNormal.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_TruncNormal.h)
- [SP_Uniform.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_Uniform.h)
- [FixedPercentSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/FixedPercentSlippage.cpp)
- [FixedValueSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/FixedValueSlippage.cpp)
- [NormalSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/NormalSlippage.cpp)
- [LogNormalSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/LogNormalSlippage.cpp)
- [TruncNormalSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/TruncNormalSlippage.cpp)
- [UniformSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/UniformSlippage.cpp)
</cite>

## 目录
1. [简介](#简介)
2. [核心移滑价差模型](#核心移滑价差模型)
3. [固定百分比移滑价差 SP_FixedPercent](#固定百分比移滑价差-sp_fixedpercent)
4. [固定数值移滑价差 SP_FixedValue](#固定数值移滑价差-sp_fixedvalue)
5. [正态分布移滑价差 SP_Normal](#正态分布移滑价差-sp_normal)
6. [对数正态分布移滑价差 SP_LogNormal](#对数正态分布移滑价差-sp_lognormal)
7. [截断正态分布移滑价差 SP_TruncNormal](#截断正态分布移滑价差-sp_truncnormal)
8. [均匀分布移滑价差 SP_Uniform](#均匀分布移滑价差-sp_uniform)
9. [使用场景与参数校准](#使用场景与参数校准)
10. [对策略绩效的影响分析](#对策略绩效的影响分析)

## 简介
移滑价差（Slippage）是量化交易中模拟实际交易价格与计划交易价格之间差异的重要组件。在回测和实盘交易中，由于市场流动性、订单执行速度等因素，实际成交价格往往偏离预期价格。Hikyuu 提供了多种移滑价差模型，允许用户根据不同的市场条件和交易策略选择合适的滑点模型，以更真实地模拟交易过程。

本文档系统性地介绍 Hikyuu 中所有可用的移滑价差创建函数，包括 `SP_FixedPercent`、`SP_FixedValue`、`SP_Normal`、`SP_LogNormal`、`SP_TruncNormal` 和 `SP_Uniform`，详细说明每个构造函数的参数、使用场景及其实现机制。

## 核心移滑价差模型
Hikyuu 的移滑价差组件基于 `SlippageBase` 基类实现，所有具体的滑点模型均继承自该基类，并通过工厂函数创建实例。这些模型主要用于计算买入和卖出的实际成交价格，从而影响交易成本和策略绩效。

**本文档引用文件**  
- [SP_FixedPercent.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_FixedPercent.h)
- [SP_FixedValue.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_FixedValue.h)
- [SP_Normal.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_Normal.h)
- [SP_LogNormal.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_LogNormal.h)
- [SP_TruncNormal.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_TruncNormal.h)
- [SP_Uniform.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_Uniform.h)

## 固定百分比移滑价差 SP_FixedPercent

### 功能说明
`SP_FixedPercent` 实现了固定百分比的移滑价差算法。该模型假设每次交易的价格偏差为一个固定的百分比，适用于模拟手续费或固定比例的市场冲击。

### 参数说明
- **p**: 偏移的固定百分比，默认值为 0.001（即 0.1%）
  - 必须满足：`p >= 0.0` 且 `p < 1.0`

### 价格计算逻辑
- 买入实际价格 = 计划买入价格 × (1 + p)
- 卖出实际价格 = 计划卖出价格 × (1 - p)

### 使用场景
适用于需要模拟固定比例交易成本的场景，如高频交易中的手续费估算，或在流动性较好的市场中假设滑点比例恒定。

**节来源**  
- [SP_FixedPercent.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_FixedPercent.h#L16-L22)
- [FixedPercentSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/FixedPercentSlippage.cpp#L29-L34)

## 固定数值移滑价差 SP_FixedValue

### 功能说明
`SP_FixedValue` 实现了固定数值的移滑价差算法。该模型假设每次交易的价格偏差为一个固定的金额，适用于模拟最小价格变动单位（tick）或固定金额的市场滑点。

### 参数说明
- **value**: 偏移的固定价格，默认值为 0.01
  - 必须满足：`value >= 0.0`

### 价格计算逻辑
- 买入实际价格 = 计划买入价格 + value
- 卖出实际价格 = 计划卖出价格 - value

### 使用场景
适用于价格变动以固定金额为单位的市场，如某些期货合约或外汇市场，其中滑点通常以固定的点数表示。

**节来源**  
- [SP_FixedValue.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_FixedValue.h#L16-L22)
- [FixedValueSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/FixedValueSlippage.cpp#L28-L33)

## 正态分布移滑价差 SP_Normal

### 功能说明
`SP_Normal` 实现了基于正态分布的随机价格移滑价差算法。该模型通过正态分布生成随机偏移量，模拟市场价格波动带来的不确定性滑点。

### 参数说明
- **mean**: 正态分布的均值，默认值为 0.0
- **stddev**: 正态分布的标准差，默认值为 0.05
  - 必须满足：`stddev >= 0.0`

### 价格计算逻辑
- 使用 `std::normal_distribution` 生成随机值
- 买入实际价格 = 计划买入价格 + |随机值|
- 卖出实际价格 = 计划卖出价格 - |随机值|

### 使用场景
适用于需要模拟随机滑点的场景，尤其是在流动性波动较大的市场中，滑点可能呈现正态分布特征。

**节来源**  
- [SP_Normal.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_Normal.h#L14-L20)
- [NormalSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/NormalSlippage.cpp#L33-L50)

## 对数正态分布移滑价差 SP_LogNormal

### 功能说明
`SP_LogNormal` 实现了基于对数正态分布的随机价格移滑价差算法。该模型适用于模拟价格偏移呈对数正态分布的市场行为，常用于金融资产价格变动的建模。

### 参数说明
- **mean**: 对数正态分布的均值参数，默认值为 0.0
- **stddev**: 对数正态分布的标准差参数，默认值为 0.05
  - 必须满足：`stddev >= 0.0`

### 价格计算逻辑
- 使用 `std::lognormal_distribution` 生成随机值
- 对生成的值进行中心化处理：`centered_value = value - exp(mean + stddev²/2)`
- 买入实际价格 = 计划买入价格 + |centered_value|
- 卖出实际价格 = 计划卖出价格 - |centered_value|

### 使用场景
适用于模拟价格偏移具有右偏特征的市场，如某些高波动性股票或期权市场的滑点行为。

**节来源**  
- [SP_LogNormal.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_LogNormal.h#L14-L20)
- [LogNormalSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/LogNormalSlippage.cpp#L35-L58)

## 截断正态分布移滑价差 SP_TruncNormal

### 功能说明
`SP_TruncNormal` 实现了基于截断正态分布的随机价格移滑价差算法。该模型在正态分布的基础上增加了上下限约束，确保生成的滑点值在合理范围内。

### 参数说明
- **mean**: 正态分布均值，默认值为 0.0
- **stddev**: 正态分布标准差，默认值为 0.05
- **min_value**: 截断最小值，默认值为 -0.1
- **max_value**: 截断最大值，默认值为 0.1
  - 必须满足：`stddev >= 0.0`，`min_value <= max_value`

### 价格计算逻辑
- 使用 `std::normal_distribution` 生成随机值
- 通过循环重采样确保值在 `[min_value, max_value]` 范围内
- 买入实际价格 = 计划买入价格 + |截断后的值|
- 卖出实际价格 = 计划卖出价格 - |截断后的值|

### 使用场景
适用于需要限制滑点范围的场景，防止极端值对回测结果造成不合理影响，尤其适合在风险控制严格的策略中使用。

**节来源**  
- [SP_TruncNormal.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_TruncNormal.h#L14-L23)
- [TruncNormalSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/TruncNormalSlippage.cpp#L47-L80)

## 均匀分布移滑价差 SP_Uniform

### 功能说明
`SP_Uniform` 实现了基于均匀分布的随机价格移滑价差算法。该模型在指定范围内均匀随机生成价格偏移，适用于模拟滑点在一定区间内均匀分布的情况。

### 参数说明
- **min_value**: 偏移价格下限，默认值为 -0.05
- **max_value**: 偏移价格上限，默认值为 0.05
  - 必须满足：`min_value <= max_value`

### 价格计算逻辑
- 在 `[min_value, max_value]` 范围内均匀随机生成偏移值
- 买入实际价格 = 计划买入价格 + |随机值|
- 卖出实际价格 = 计划卖出价格 - |随机值|

### 使用场景
适用于滑点分布较为均匀的市场环境，或当缺乏具体滑点分布数据时作为默认的随机滑点模型。

**节来源**  
- [SP_Uniform.h](file://hikyuu_cpp/hikyuu/trade_sys/slippage/crt/SP_Uniform.h#L14-L21)
- [UniformSlippage.cpp](file://hikyuu_cpp/hikyuu/trade_sys/slippage/imp/UniformSlippage.cpp#L37-L48)

## 使用场景与参数校准
选择合适的移滑价差模型对回测结果的真实性至关重要。以下是一些常见的使用建议：

- **固定百分比模型**：适用于手续费主导的滑点模拟，参数 `p` 可根据实际交易佣金率设定。
- **固定数值模型**：适用于以 tick 为单位的市场，`value` 可设为 1-2 个 tick 值。
- **正态分布模型**：适用于流动性较好的市场，`stddev` 可根据历史滑点数据的标准差校准。
- **截断正态分布模型**：在正态分布基础上增加安全性，避免极端滑点影响，`min_value` 和 `max_value` 可根据最大可接受滑点设定。
- **对数正态分布模型**：适用于高波动性资产，`mean` 和 `stddev` 需通过历史数据拟合。
- **均匀分布模型**：作为保守估计，`min_value` 和 `max_value` 可根据历史滑点的最小最大值设定。

参数校准应基于实际交易数据或市场微观结构分析，以确保回测结果的可靠性。

## 对策略绩效的影响分析
移滑价差直接影响交易成本，进而影响策略的收益和风险指标：

- **收益影响**：滑点增加了买入成本，降低了卖出收入，直接减少策略收益。
- **风险影响**：随机滑点模型引入了额外的不确定性，可能增加策略的收益波动性。
- **策略稳定性**：过大的滑点可能导致止损单提前触发或无法成交，影响策略的稳定性。
- **参数敏感性**：滑点参数的选择对高频交易策略影响尤为显著，需进行敏感性分析。

在实盘交易前，应通过不同滑点模型和参数的对比测试，评估策略在各种市场条件下的鲁棒性。