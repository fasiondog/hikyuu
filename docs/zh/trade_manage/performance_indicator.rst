.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

绩效统计指标计算说明
====================

本页说明 :py:meth:`TradeManager.get_performance` 返回的 :py:class:`Performance` 各统计指标的计算口径。

统一时间口径
------------

- 所有与时间相关的统计（收益、年化、波动率、回撤、持仓/空仓时长等）统一采用 **自然日历日** 序列，
  不依赖任何单一市场的交易日历（hikyuu 面向多资产、多策略混合场景）。
- 年化因子统一为 **sqrt(365)**（自然日）。
- 年化收益的时长为自然日：``years = 自然日数 / 365``。
- 收益相关指标以**净资产** ``net_assets = 现金 + 多头市值 - 空头市值 - 借入现金`` 为口径（不把借入资金计入分子），
  分母为累计净投入 ``total_base = 累计入金 - 累计出金 + 累计入券 - 累计出券``。

统计项分两层：

- **基础项**：核心库 ``Performance.statistics`` 计算，``get_performance()`` 或未启用扩展时返回。
- **扩展项**：需 ``get_performance(ext=True)`` 且具备扩展权限，由 tmreport 插件计算（
  时间口径与基础项一致，风险调整指标的年化收益分子统一使用时间加权收益 TWR）。

账户资金与资产项
----------------

- **Account Initial Capital**：账户初始资金。
- **Total Invested Principal**：累计投入本金（现金口径）。
- **Total Invested Assets**：累计投入资产（券口径）。
- **Total Borrowed Cash**：累计借入现金。
- **Total Borrowed Assets**：累计借入资产。
- **Total Dividends**：累计红利（所有 BONUS 记录的现金之和）。
- **Cash Balance**：现金余额。
- **Open Position Net Value**：未平仓头寸市值（多头市值）。
- **Current Total Assets**：当前总资产（毛口径，含借入资产；仅用于展示，不用于收益计算）。

收益与年化项
------------

- **Open Position Account Return %**：未平仓账户收益率，``100 * (net_assets / 累计投入本金 - 1)``。
- **Closed Trade Account Return %**：已平仓账户收益率，``100 * 已平仓净利润 / 累计投入本金``。
- **Account Avg Annual Return %**：账户平均年收益率（单利），``100 * ((net_assets / total_base - 1) / years)``。
- **Account CAGR %**：账户年复合收益率，``100 * ((net_assets / total_base) ^ (1 / years) - 1)``；
  当 ``net_assets <= 0``（爆仓/深度亏损）时返回 NaN（空值），不进入报告。
- **Account Time-Weighted Return %**（扩展）：时间加权收益率，剔除出入金影响。按日收益
  ``r_t = (net_assets_t - flow_t) / net_assets_{t-1} - 1``（``flow_t`` 为当日净入金）累积得到净值指数
  ``E = ∏(1 + r_t)``，年化 ``100 * (E ^ (1 / years) - 1)``。
- **Account Money-Weighted Return %**（扩展）：资金加权收益率，即外部现金流的年化内部收益率 XIRR，
  求解 ``Σ CF_i / (1 + x) ^ ((t_i - t0) / 365) = 0``；现金流含初始投入、每笔入金（负）、出金（正）与期末净资产（正）。

交易统计项
----------

- **Total Closed Trades**：已平仓交易（持仓记录）总数。
- **Total Cost of Closed Trades**：已平仓交易总成本。
- **Total Net Profit of Closed Trades**：已平仓净利润总额，``Σ (卖出金额 - 成本 - 买入金额)``。
- **Total Profit of Winning Trades** / **Total Loss of Losing Trades**：盈利交易盈利总额 / 亏损交易亏损总额（利润 > 0 记盈利，否则记亏损，零利润计入亏损）。
- **Number of Winning Trades** / **Number of Losing Trades**：盈利 / 亏损交易笔数。
- **Win Rate %**：胜率，``100 * 盈利笔数 / 已平仓笔数``。
- **Avg Profit per Winning Trade** / **Avg Loss per Losing Trade**：平均每笔盈利 / 平均每笔亏损。
- **Avg Win / Avg Loss Ratio**：盈亏比，``平均盈利 / |平均亏损|``。
- **Profit Factor**：盈利因子，``盈利总额 / |亏损总额|``。
- **Profit Expectancy**：盈利期望值（金额口径），``胜率 * 平均盈利 + (1 - 胜率) * 平均亏损``。
- **Largest Single Win** / **Largest Single Loss**：最大单笔盈利 / 最大单笔亏损。
- **Largest Single Win %** / **Largest Single Loss %**：最大单笔盈利百分比 / 最大单笔亏损百分比，
  ``100 * 单笔利润 / (买入金额 + 成本)``。
- **Max Cash Usage per Trade %** / **Avg Cash Usage per Trade %**：单笔最大/平均占用现金比例，
  基于每笔买入的占用资金占当时总现金比例。

R 乘数项
--------

单笔 R 乘数 ``r = 利润 / 风险``，风险 ``totalRisk = (买入价 - 止损价) * 数量 * 每手乘数``；风险为 0 时记 0。

- **R-Multiple Expectancy**：R 乘数期望值，``Σ r / 已平仓笔数``。
- **Avg R-Multiple of Winning Trades** / **Avg R-Multiple of Losing Trades**：盈利 / 亏损交易平均 R 乘数。
- **Max Single Win R-Multiple** / **Max Single Loss R-Multiple**：最大单笔盈利 / 亏损 R 乘数。
- **Trade Opportunities per Year**：年交易机会频率，``已平仓笔数 / years``。
- **Annual Expected R-Multiple**：年度期望 R 乘数，``R 乘数期望值 * 年交易机会频率``。

连续交易项
----------

- **Max Consecutive Wins** / **Max Consecutive Losses**：历史最大连续盈利 / 连续亏损笔数。
- **Max Consecutive Win Amount** / **Max Consecutive Loss Amount**：最大连续盈利 / 连续亏损段的金额之和
  （等长段时取金额更大者；亏损段取亏损更大者）。
- **Max Consecutive Win R-Multiple** / **Max Consecutive Loss R-Multiple**：上述被选中连击段的平均 R 乘数
  （``段内 R 之和 / 段长``）。

持仓与空仓时长项
----------------

- **Avg Holding Period of Winning Trades** / **Avg Holding Period of Losing Trades**：盈利 / 亏损交易平均持仓天数（自然日）。
- **Max Holding Period of Winning Trades** / **Max Holding Period of Losing Trades**：盈利 / 亏损交易最大持仓天数。
- **Total Time Flat**：空仓总天数（自然日）。
- **Time Flat / Total Time %**：空仓时间占比，``100 * 空仓天数 / 区间自然日数``（浮点，不截断）。
- **Avg Time Flat**：平均每段空仓天数，``空仓天数 / 空仓段数``（浮点）。
- **Max Time Flat**：最长连续空仓天数。空仓判定同时计入当前未平仓持仓（未平仓头寸持有至统计时刻）。

风险调整项（扩展）
------------------

以下指标基于自然日、剔除出入金现金流后的日收益序列（``r_t = (net_assets_t - flow_t)/net_assets_{t-1} - 1``），
年化收益分子统一使用时间加权收益 TWR：

- **Annual Volatility %**：年化波动率，``std(r_t) * sqrt(365) * 100``（样本标准差）。
- **Max Drawdown %**：最大回撤，在时间加权净值指数 ``E`` 上的最大峰谷跌幅，``100 * max((peak - E) / peak)``。
- **Sharpe Ratio**：夏普比率，``(TWR年化 * 100 - rf) / (年化波动率)``，``rf`` 为 10 年期国债收益率（%）。
- **Sortino Ratio**：索提诺比率，``(TWR年化 * 100 - rf) / (下行年化波动)``，
  下行年化波动 = ``sqrt(mean(min(r_t, 0)^2)) * sqrt(365) * 100``。
- **Calmar Ratio**：卡玛比率，``TWR年化 / 最大回撤``（两者同为小数）。
- **Avg Per-Trade Return %** / **Std Per-Trade Return %**：单笔交易收益率（``100 * 利润 / (买入金额 + 成本)``）的
  均值与样本标准差；无成本记录（如分红送转）无法计算单笔收益率，予以剔除。
