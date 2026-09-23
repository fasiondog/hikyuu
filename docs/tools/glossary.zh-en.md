# 量化术语中英对照表 / Hikyuu Glossary (Chinese–English)

> 用途：**统一代码注释、docstring、文档与 README 的英文用词**。凡本表收录的词，全项目必须按同一映射使用，
> 不得同义词混用（例如 `money management` 与 `fund management` 不得混指同一组件）。
>
> 对齐目标：**海外量化开发习惯**（sell-side / buy-side / systematic trading 通用术语）。
>
> 本文件是「英文化与双语文档」方案维护规范 §5 所述的仓库内术语表落地位置（CI 可读）；
> 与 obsidian 设计稿 `design_英文化双语文档_术语表.md` 同源，以本文件为仓库内唯一权威版本。

---

## 1. 使用说明

- 本表是代码注释英文化与双语文档（`docs/en/` + `docs/zh/`）的**共用词表**，新增术语需先入表再使用。
- 术语变更走 PR review，改名视为全仓变更（代码注释、docstring、两棵文档树、README 一次性替换）。
- 缩写（§10）在代码与文档中**保持大写**，首次出现给全称。

## 2. 交易系统部件（Trade System Parts）

| 中文 | 英文（标准） | 缩写 | 代码对应 | 备注 |
| --- | --- | --- | --- | --- |
| 系统化交易 | systematic trading | — | — | 不是 "system trading" |
| 交易系统 | trading system | SYS | `trade_sys/system`；类 `System`、`SYS_Simple` / `SYS_WalkForward` | |
| 市场环境（判断） | market environment | EV | `trade_sys/environment`（`crtEV`、`EV_Bool` / `EV_TwoLine`） | |
| 系统有效条件 | condition | CN | `trade_sys/condition`（`crtCN`、`CN_Bool` / `CN_OPLine`） | 系统适用条件 |
| 信号指示器 | signal / signal generator | SG | `trade_sys/signal`（`crtSG`、`SG_Cross` 等） | **不用** "signal indicator" |
| 买卖信号 | buy/sell signal | — | `shouldBuy()` / `shouldSell()` | |
| 止损 | stop-loss | ST | `trade_sys/stoploss`（`crtST`、`ST_FixedPercent` 等） | 保留连字符 |
| 止盈 | take-profit | TP | — | **不用** "stopprofit" 作为正文 |
| 资金管理 | money management | MM | `trade_sys/moneymanager`（`crtMM`、`MM_FixedPercent` 等） | **不用** "money manage"；类名 `MoneyManager`（组件）可用 "money manager" |
| 头寸规模 / 下单数量 | position sizing | — | — | |
| 盈利目标 | profit goal | PG | `trade_sys/profitgoal`（`crtPG`、`PG_FixedPercent` 等） | |
| 滑点 / 移滑价差 | slippage | SP | `trade_sys/slippage`（`crtSP`、`SP_Normal` 等） | |
| 系统选择器 / 标的筛选 | selector | SE | `trade_sys/selector`（`crtSE`、`SE_Fixed` 等） | |
| 资金分配 | fund allocation / allocation | AF | `trade_sys/allocatefunds`（`crtAF`、`TC_FixedA` 等） | |
| 多因子模型 | multi-factor model | MF | `trade_sys/multifactor`（`crtMF`、`MF_ICWeight` 等） | |
| 投资组合 | portfolio | PF | `trade_sys/portfolio` | |
| 组合再平衡 | portfolio rebalancing | — | — | |
| 交易管理 | trade management | TM | `trade_manage/` | |
| 交易管理器 | trade manager | TM | 类名 `TradeManager`（`crtTM`） | |
| 订单执行 / 下单代理 | order broker | OB | 类名 `OrderBrokerBase`（`crtOB`） | |
| 回测 | backtest | — | — | 动词 backtest / 名词 backtest |
| 滚动前推分析 | walk-forward analysis | — | `SYS_WalkForward` | 不是 "forward test" |
| 参数优化 | parameter optimization | — | `Parameter` | |

## 3. 交易与订单（Trading & Orders）

| 中文 | 英文 |
| --- | --- |
| 开仓 / 建仓 | open position / entry |
| 平仓 | close position / exit |
| 加仓 | add position / scale in |
| 减仓 | reduce position / scale out |
| 清仓 | liquidate / clear position |
| 持仓 | position / holding |
| 未平仓头寸 | open position |
| 已平仓交易 | closed trade |
| 委托 / 订单 | order |
| 买入委托 | buy order |
| 卖出委托 | sell order |
| 成交 | fill / execution |
| 成交记录 | trade record（`TradeRecord`） |
| 现金 | cash |
| 账户 | account |
| 资产 | assets |
| 总资产 | total assets |
| 净值 | net value / NAV |
| 持仓成本 | position cost |
| 手续费 | commission |
| 印花税 | stamp duty |
| 过户费 | transfer fee |
| 交易成本 | transaction cost |
| 融资 / 借入资金 | margin / borrowed funds |
| 红利 | dividend |
| 权息（复权）数据 | equity/dividend adjustment data |
| 复权 | adjustment (forward / backward)；前复权 qfq (forward adjustment)、后复权 hfq (backward adjustment)；常量 `RECOVER_*`。注：代码枚举 `KQuery::RecoverType`（`NO_RECOVER`/`FORWARD`/`BACKWARD`/`EQUAL_*`）为历史 API 名，正文用 adjustment |
| 除权除息 | ex-rights / ex-dividend |
| 复权因子 | adjustment factor（`IAdjFactor`） |
| 交割 / 结算 | settlement |

## 4. 绩效与风险（Performance & Risk）

| 中文 | 英文 |
| --- | --- |
| 绩效 / 表现 | performance |
| 绩效指标 | performance metrics |
| 资金曲线 | equity curve |
| 收益率 | return |
| 年化收益率 | annualized return |
| 复合年增长率 | CAGR (compound annual growth rate) |
| 最大回撤 | max drawdown |
| 夏普比率 | Sharpe ratio |
| 净利 / 亏损比 | profit factor |
| 胜率 | win rate |
| 盈亏比 | profit/loss ratio |
| 期望值 | expectancy |
| R 乘数 | R-multiple |
| 赢利交易 | winning trade |
| 亏损交易 | losing trade |
| 连续赢利 / 亏损 | consecutive wins / losses (streak) |
| 持仓时间 | holding period |
| 空仓时间 | time flat / out-of-market time |
| 换手率 | turnover |
| 调仓 / 调仓周期 | rebalance / rebalance cycle |
| 风险控制 | risk control |
| 集中度 | concentration |
| 敞口 / 暴露 | exposure |

## 5. 多因子与组合（Factors & Portfolio）

| 中文 | 英文 |
| --- | --- |
| 因子 | factor |
| 因子集 | factor set（`FactorSet`） |
| 因子暴露 | factor exposure |
| 因子中性化 | factor neutralization |
| 行业中性 | industry neutral |
| 打分 / 评分 | score |
| 排序 | ranking |
| 权重 | weight |
| 等权 | equal weight |
| 子系统 | sub-system |
| 聚合 | aggregate |
| 嵌套 | nesting |
| 影子账户 | shadow account |
| 资金划拨 | capital allocation / transfer |
| 额度 | quota |
| 系统权重 | system weight（`SystemWeight`） |
| 指标 | indicator |
| 上下文 | context |

## 6. 数据与行情（Data & Market）

| 中文 | 英文 | 代码对应 |
| --- | --- | --- |
| 行情 / 行情数据 | market data / quotes | — |
| 实时行情 | real-time quotes | — |
| K 线 | candlestick / bar；代码中统一用 "K-line"，首次出现带 `(candlestick)` 注记 | `KData` / `KRecord` |
| 日线 | daily bar | `KData`（日线 K 类型） |
| 分钟线 | minute bar | `KData`（1/5/15/30/60 分钟 K 类型） |
| 逐笔成交 | tick / transaction | `TransRecord` |
| 分时数据 | timeline / intraday time series | `TimeLineRecord` |
| 证券 | security / instrument | `Stock` |
| 标的 | underlying / instrument | — |
| 证券代码 | stock code | `Stock::code()` |
| 证券名称 | stock name | `Stock::name()` |
| 市场信息 | market info | `MarketInfo` |
| 证券类型 | stock type | `StockTypeInfo` |
| 板块 | sector | 代码标识符为 `Block`（`Block::category()` 区分板块类别） |
| 行业板块 | industry sector | `Block`（行业类 category） |
| 概念板块 | concept sector | `Block`（概念类 category） |
| 指数 | index | `Query` 索引查询（`KQuery::INDEX`）、`INDEX*` 指标 |
| 时间区间查询 | query | `KQuery` / `Query` |
| 证券管理 | stock manager | `StockManager`（`get_stock_manager()`） |
| 数据驱动 | data driver | `data_driver/` |
| 存储引擎 | storage engine | HDF5 / MySQL / SQLite / TDX 驱动 |
| 数据导入 | data import | `plugin/` 导入器（如 `KDataTo*Importer`） |
| 基础信息 | base info | `BaseInfoDriver` |
| 财务数据 | finance data | `HistoryFinanceInfo` / `FinanceInfo` |
| 共享内存数据服务 | shared-memory data service (IPC) | `ShmServer` / `dataserver` 插件、`KDataShm*` |
| 插件 | plugin | `plugin/` |
| 参数 | parameter | `Parameter` |
| 策略上下文 | strategy context | `StrategyContext` |

## 7. 市场事件（Market Events）

| 中文 | 英文 |
| --- | --- |
| 停牌 | trading suspension / trading halt |
| 复牌 | resume trading |
| 退市 | delisting |
| 涨停 | limit up |
| 跌停 | limit down |
| 一字板 | limit-up/limit-down lock |
| 移仓 | rollover |

## 8. 业绩指标名（Performance.cpp，已落地）

> `hikyuu_cpp/hikyuu/trade_manage/Performance.cpp` 的 53 个指标名**已完成英文化**：英文 key 为正式
> 输出键名，原中文 key 通过 `legacyKeyMap()` 提供向后兼容（旧中文 key → 新英文 key 一一映射），
> 中文名统一由 `chineseNameMap()` 提供。下表即当前实现中的正式英文键名。

| 序号 | 中文 | 英文键名（现行） |
| --- | --- | --- |
| 1 | 帐户初始金额 | Account Initial Capital |
| 2 | 累计投入本金 | Total Invested Principal |
| 3 | 累计投入资产 | Total Invested Assets |
| 4 | 累计借入现金 | Total Borrowed Cash |
| 5 | 累计借入资产 | Total Borrowed Assets |
| 6 | 累计红利 | Total Dividends |
| 7 | 现金余额 | Cash Balance |
| 8 | 未平仓头寸净值 | Open Position Net Value |
| 9 | 当前总资产 | Current Total Assets |
| 10 | 已平仓交易总成本 | Total Cost of Closed Trades |
| 11 | 已平仓净利润总额 | Total Net Profit of Closed Trades |
| 12 | 单笔交易最大占用现金比例% | Max Cash Usage per Trade % |
| 13 | 交易平均占用现金比例% | Avg Cash Usage per Trade % |
| 14 | 未平仓帐户收益率% | Open Position Account Return % |
| 15 | 已平仓帐户收益率% | Closed Trade Account Return % |
| 16 | 帐户年复合收益率% | Account CAGR % |
| 17 | 帐户平均年收益率% | Account Avg Annual Return % |
| 18 | 赢利交易赢利总额 | Total Profit of Winning Trades |
| 19 | 亏损交易亏损总额 | Total Loss of Losing Trades |
| 20 | 已平仓交易总数 | Total Closed Trades |
| 21 | 赢利交易数 | Number of Winning Trades |
| 22 | 亏损交易数 | Number of Losing Trades |
| 23 | 赢利交易比例% | Win Rate % |
| 24 | 赢利期望值 | Profit Expectancy |
| 25 | 赢利交易平均赢利 | Avg Profit per Winning Trade |
| 26 | 亏损交易平均亏损 | Avg Loss per Losing Trade |
| 27 | 平均赢利/平均亏损比例（每笔） | Avg Win / Avg Loss Ratio（**不要**与 payoff ratio 或 Profit Factor 混用：本项 = 平均单笔盈利 / 平均单笔亏损；Profit Factor = 总盈利 / 总亏损） |
| 28 | 净赢利/亏损比例 | Profit Factor |
| 29 | 最大单笔赢利 | Largest Single Win |
| 30 | 最大单笔盈利百分比% | Largest Single Win % |
| 31 | 最大单笔亏损 | Largest Single Loss |
| 32 | 最大单笔亏损百分比% | Largest Single Loss % |
| 33 | 赢利交易平均持仓时间 | Avg Holding Period of Winning Trades |
| 34 | 赢利交易最大持仓时间 | Max Holding Period of Winning Trades |
| 35 | 亏损交易平均持仓时间 | Avg Holding Period of Losing Trades |
| 36 | 亏损交易最大持仓时间 | Max Holding Period of Losing Trades |
| 37 | 空仓总时间 | Total Time Flat |
| 38 | 空仓时间/总时间% | Time Flat / Total Time % |
| 39 | 平均空仓时间 | Avg Time Flat |
| 40 | 最长空仓时间 | Max Time Flat |
| 41 | 最大连续赢利笔数 | Max Consecutive Wins |
| 42 | 最大连续亏损笔数 | Max Consecutive Losses |
| 43 | 最大连续赢利金额 | Max Consecutive Win Amount |
| 44 | 最大连续亏损金额 | Max Consecutive Loss Amount |
| 45 | R乘数期望值 | R-Multiple Expectancy |
| 46 | 交易机会频率/年 | Trade Opportunities per Year |
| 47 | 年度期望R乘数 | Annual Expected R-Multiple |
| 48 | 赢利交易平均R乘数 | Avg R-Multiple of Winning Trades |
| 49 | 亏损交易平均R乘数 | Avg R-Multiple of Losing Trades |
| 50 | 最大单笔赢利R乘数 | Max Single Win R-Multiple |
| 51 | 最大单笔亏损R乘数 | Max Single Loss R-Multiple |
| 52 | 最大连续赢利R乘数 | Max Consecutive Win R-Multiple |
| 53 | 最大连续亏损R乘数 | Max Consecutive Loss R-Multiple |

## 9. 绘图文案（hikyuu/draw，计划译名，阶段 6 落地）

> 用途：`hikyuu/draw/**` 的**图上显示文案**改为英文 msgid + `htr()` 时，按下表取词
> （方案见 `design_英文化双语文档_代码注释.md` §5.1；**当前实现仍为中文硬编码，本表为落地基准**）。
> **业绩类文案必须直接引用 §8 的既有键名**，不得自造同义写法
> （v7 曾因 `Account average annual return %` 与 §8 的 `Account Avg Annual Return %` 不一致导致单测失败）。

| 中文 | 英文 msgid | 备注 |
| --- | --- | --- |
| 开 / 高 / 低 / 收 | `Open` / `High` / `Low` / `Close` | K 线提示 |
| 涨幅 | `Change %` | K 线提示 |
| 成交量 | `Volume` | |
| 收益率(%) | `Return (%)` | 轴标签 |
| 累积收益率 | `Cumulative Return` | 图例/标题 |
| 年度收益 | `Annual Return` | |
| 年化收益率 | `Annualized Return` | 业绩表语境用 §8 的 `Account Avg Annual Return %` |
| 最大回撤 | `Max Drawdown` | 同 §8 |
| 当前距历史最高点回撤 | `Current Drawdown from Peak` | |
| 系统胜率 | `Win Rate %`（图上标签 `Win Rate`） | 同 §8 |
| 盈/亏比（图上 `1 : x`） | `Avg Win/Avg Loss`，取 Performance key `Avg Win / Avg Loss Ratio`（均盈/均亏） | **不是** `Profit Factor`（总盈/总亏，见 §8-28） |
| 夏普比率 | `Sharpe Ratio` | |
| 投入总资产 / 当前总资产 / 当前盈利 | `Total Invested Assets` / `Current Total Assets` / `Current Profit` | 同 §8 |
| 月度 / 年份 | `Month` / `Year` | 热力图轴 |
| 年-月度收益率(%)热力图 | `Year-Month Return (%) Heatmap` | 标题 |
| 最高收益范围 / 最低收益范围 | `Highest Return Range` / `Lowest Return Range` | |
| 数据不足 | `Insufficient data` | 告警 |
| 没有数据，请检查日期范围 | `No data, please check the date range` | 告警 |
| （日线）/（周线）/（月线）/（季线）/（半年线）/（年线） | `(Daily)` / `(Weekly)` / `(Monthly)` / `(Quarterly)` / `(Half-Yearly)` / `(Yearly)` | 标题后缀 |
| （1/3/5/15/30/60分钟线） | `(1-min)` / `(3-min)` / `(5-min)` / `(15-min)` / `(30-min)` / `(60-min)` | 标题后缀 |
| （2/4/6/12小时线） | `(2-hour)` / `(4-hour)` / `(6-hour)` / `(12-hour)` | 标题后缀 |
| 收益曲线 | `Return Curve` | 图例 |

## 10. 缩写约定（代码与文档保持一致）

> **用词规则**：交易体系中的各单元（EV/CN/SG/ST/TP/MM/PG/SP/SE/AF/MF/PF/TM）统一称
> **part**（对应代码 `SystemPart` / `System.Part`，中文"部件"），**不要**用 component；
> component 仅用于内部软件组合语境（如 GUI 组件、软件架构组件）。板块成分股用 constituent。

| 缩写 | 全称 | 中文 |
| --- | --- | --- |
| — | **part**（交易体系部件，禁用 component） | 部件 |
| SG | signal | 信号指示器 |
| MM | money management | 资金管理 |
| ST | stop-loss | 止损 |
| TP | take-profit | 止盈 |
| PG | profit goal | 盈利目标 |
| SP | slippage | 滑点 |
| EV | market environment | 市场环境 |
| CN | condition | 系统有效条件 |
| SE | selector | 选择器 |
| AF | allocate funds | 资金分配 |
| MF | multi-factor | 多因子 |
| PF | portfolio | 投资组合 |
| SYS | system | 交易系统 |
| TM | trade manager | 交易管理 |
| OB | order broker | 订单执行 |
| SM | stock manager | 证券管理 |
| KD | k-data | K 线数据 |

## 11. 风格禁忌（避免中式英语）

| ✗ 中式英语 | ✓ 推荐 |
| --- | --- |
| signal indicator | signal / signal generator |
| money manage manager | money manager / money management |
| shift slippery price difference | slippage |
| stop profit | take-profit |
| system environment judge | market environment |
| back test | backtest |
| win/loss ratio (含义模糊) | win rate / profit factor（按语义二选一） |
| profit making target | profit goal |
| funds distribute | fund allocation |
| choose stock | stock selection / selector |
| multi factor model | multi-factor model |
| the K line | candlestick / bar |
| paper trade (歧义) | simulated trading / backtest |
