.. currentmodule:: hikyuu.trade_manage
.. highlight:: python

Performance Indicator Definitions
=================================

This page documents how each statistics item returned by
:py:meth:`TradeManager.get_performance` (a :py:class:`Performance`) is computed.

Common time basis
-----------------

- All time-related statistics (returns, annualization, volatility, drawdown, holding/flat periods,
  etc.) use a **natural calendar day** series and do not depend on the trading calendar of any single
  market (hikyuu targets multi-asset, multi-strategy mixed portfolios).
- The annualization factor is **sqrt(365)** (natural days).
- The period length for annualized returns is in natural days: ``years = number_of_natural_days / 365``.
- Return metrics are based on **net assets**
  ``net_assets = cash + long market value - short market value - borrowed cash`` (borrowed funds are
  not counted in the numerator); the denominator is the accumulated net investment
  ``total_base = cumulative checkin - cumulative checkout + cumulative stock-in - cumulative stock-out``.

Statistics are provided in two layers:

- **Base items**: computed by the core ``Performance.statistics`` and returned by
  ``get_performance()`` or when the extension is disabled.
- **Extended items**: require ``get_performance(ext=True)`` and the extension privilege; they are
  computed by the tmreport plugin (same time basis as the base items; the numerator of the
  risk-adjusted ratios is the time-weighted return, TWR).

Account funds and assets
------------------------

- **Account Initial Capital**: the initial account capital.
- **Total Invested Principal**: accumulated invested principal (cash).
- **Total Invested Assets**: accumulated invested assets (securities).
- **Total Borrowed Cash**: accumulated borrowed cash.
- **Total Borrowed Assets**: accumulated borrowed assets.
- **Total Dividends**: accumulated dividends (sum of the cash of all BONUS records).
- **Cash Balance**: the current cash balance.
- **Open Position Net Value**: the market value of the open long positions.
- **Current Total Assets**: the current total assets (gross basis, including borrowed assets; for
  display only, not used for return computation).

Returns and annualization
-------------------------

- **Open Position Account Return %**: ``100 * (net_assets / accumulated invested principal - 1)``.
- **Closed Trade Account Return %**: ``100 * net profit of closed trades / accumulated invested principal``.
- **Account Avg Annual Return %**: the average annual return (simple), ``100 * ((net_assets / total_base - 1) / years)``.
- **Account CAGR %**: the compound annual growth rate, ``100 * ((net_assets / total_base) ^ (1 / years) - 1)``;
  when ``net_assets <= 0`` (wiped out / deep loss) it returns NaN (Null) instead of entering the report.
- **Account Time-Weighted Return %** (extended): the time-weighted return, excluding cash flows. From the
  daily return ``r_t = (net_assets_t - flow_t) / net_assets_{t-1} - 1`` (``flow_t`` is the net inflow of the
  day) build the index ``E = product(1 + r_t)``, then annualize as ``100 * (E ^ (1 / years) - 1)``.
- **Account Money-Weighted Return %** (extended): the money-weighted return, i.e. the annualized internal
  rate of return (XIRR) of the external cash flows, solving
  ``sum(CF_i / (1 + x) ^ ((t_i - t0) / 365)) = 0``; the cash flows include the initial investment, each
  checkin (negative), each checkout (positive) and the final net assets (positive).

Trade statistics
----------------

- **Total Closed Trades**: the number of closed trades (position records).
- **Total Cost of Closed Trades**: the total cost of closed trades.
- **Total Net Profit of Closed Trades**: the total net profit of closed trades, ``sum(sell money - cost - buy money)``.
- **Total Profit of Winning Trades** / **Total Loss of Losing Trades**: the total profit of winning trades /
  the total loss of losing trades (profit > 0 counts as a win, otherwise a loss; a zero-profit trade counts as a loss).
- **Number of Winning Trades** / **Number of Losing Trades**: the number of winning / losing trades.
- **Win Rate %**: ``100 * number of winning trades / number of closed trades``.
- **Avg Profit per Winning Trade** / **Avg Loss per Losing Trade**: the average profit / loss per trade.
- **Avg Win / Avg Loss Ratio**: ``avg profit / abs(avg loss)``.
- **Profit Factor**: ``total profit / abs(total loss)``.
- **Profit Expectancy**: the profit expectancy (amount basis),
  ``win rate * avg profit + (1 - win rate) * avg loss``.
- **Largest Single Win** / **Largest Single Loss**: the largest single profit / loss.
- **Largest Single Win %** / **Largest Single Loss %**: the largest single profit / loss percentage,
  ``100 * single profit / (buy amount + cost)``.
- **Max Cash Usage per Trade %** / **Avg Cash Usage per Trade %**: the maximum / average cash usage ratio
  per trade, based on the funds occupied by each buy relative to the total cash at that time.

R-multiple items
----------------

The per-trade R multiple is ``r = profit / risk``, where the risk is
``totalRisk = (buy price - stop-loss price) * number * unit``; when the risk is 0, r is recorded as 0.

- **R-Multiple Expectancy**: ``sum(r) / number of closed trades``.
- **Avg R-Multiple of Winning Trades** / **Avg R-Multiple of Losing Trades**: the average R multiple of
  winning / losing trades.
- **Max Single Win R-Multiple** / **Max Single Loss R-Multiple**: the largest single win / loss R multiple.
- **Trade Opportunities per Year**: ``number of closed trades / years``.
- **Annual Expected R-Multiple**: ``R-multiple expectancy * trade opportunities per year``.

Consecutive trade items
-----------------------

- **Max Consecutive Wins** / **Max Consecutive Losses**: the historical maximum number of consecutive
  winning / losing trades.
- **Max Consecutive Win Amount** / **Max Consecutive Loss Amount**: the summed amount of the maximum
  consecutive winning / losing streak (for equal-length streaks the larger amount wins; for losses the
  larger loss wins).
- **Max Consecutive Win R-Multiple** / **Max Consecutive Loss R-Multiple**: the average R multiple of the
  selected streak above (``sum of r within the streak / streak length``).

Holding and flat periods
------------------------

- **Avg Holding Period of Winning Trades** / **Avg Holding Period of Losing Trades**: the average holding
  days (natural days) of winning / losing trades.
- **Max Holding Period of Winning Trades** / **Max Holding Period of Losing Trades**: the maximum holding
  days of winning / losing trades.
- **Total Time Flat**: the total flat days (natural days).
- **Time Flat / Total Time %**: ``100 * flat days / natural days in range`` (floating point, not truncated).
- **Avg Time Flat**: the average flat days per streak, ``flat days / number of flat streaks`` (floating point).
- **Max Time Flat**: the longest consecutive flat days. The holding test also counts the currently open
  positions (an open position is held through the statistics moment).

Risk-adjusted items (extended)
------------------------------

The following metrics are based on the natural-day daily returns with external cash flows removed
(``r_t = (net_assets_t - flow_t) / net_assets_{t-1} - 1``); the numerator of the annualized return is the
time-weighted return (TWR):

- **Annual Volatility %**: ``std(r_t) * sqrt(365) * 100`` (sample standard deviation).
- **Max Drawdown %**: the maximum peak-to-trough drawdown on the time-weighted index ``E``,
  ``100 * max((peak - E) / peak)``.
- **Sharpe Ratio**: ``(TWR annual * 100 - rf) / annual volatility``, where ``rf`` is the 10Y government
  bond yield (percent).
- **Sortino Ratio**: ``(TWR annual * 100 - rf) / annualized downside deviation``, where the annualized
  downside deviation is ``sqrt(mean(min(r_t, 0)^2)) * sqrt(365) * 100``.
- **Calmar Ratio**: ``TWR annual / max drawdown`` (both as fractions).
- **Avg Per-Trade Return %** / **Std Per-Trade Return %**: the mean and sample standard deviation of the
  per-trade return ``100 * profit / (buy amount + cost)``; records without a cost (e.g. dividend / bonus)
  cannot yield a per-trade return and are skipped.
