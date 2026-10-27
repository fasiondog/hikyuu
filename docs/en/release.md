# Release Notes

## 3.0.0 - October 28, 2026

**🚀 New Features**

* feat(portfolio): the new `MultiSystem` portfolio engine (mode A signal aggregation / B fund allocation / C shared account), the `PF_SignalAggregate` factory and the `PF_ModeA` / `PF_ModeB` / `PF_ModeC` aliases
* feat(allocatefunds): `AllocateFundsBase` extended into three replaceable layers L1/L2/L3, with the new `AF_FixedAmount` / `AF_MultiFactor` factories
* feat(allocatefunds): AF risk-control parameters `max_single_position` / `max_total_position` (default 1.0, `<=0` disables; when exceeded the weight pricing is scaled down and the fixed amounts are truncated to whole quanta), `fixed_amount` (an absolute amount taking precedence over the weight pricing) and `adjust_running_sys`; the executor adds `auto_checkin` (funds the gap) and the margin buying power `support_borrow_cash`
* feat(MultiSystem): new `axis_mode` / `adjust_cycle` / `adjust_mode` / `delay_to_trading_day` / `trade_on_close` / `sell_at_not_selected` parameters (snapshotted at `readyForRun`) and arbitrary-depth nesting (the parent/child mode matrix is in `trade_sys/system`); the runtime state is serializable and the aggregate supports pickle, plus the `run(query)` overload and `MomentResult.pendingOnNextOpen`
* feat(MultiSystem): the mode C removed-holding watch switch `keep_watch_force_sell` (default `False`): when on, a removed sub-system that still holds keeps being driven (watch-only, no forced close; its own stop-loss / profit goal / sell signal closes it and it leaves the pool once flat); the forced liquidation is still owned by `sell_at_not_selected` alone. `PF_WithoutAF` / `PF_ModeC` take it as a trailing parameter and the default stays unchanged
* feat(strategy): new `crt_multi_sys_strategy` to mount a `MultiSystem` directly on the strategy runtime; `Strategy` supports shorts: with `support_short` on, the `order` sell part beyond the long holding opens a short by borrow-and-sell (close first, then open), and `buy_short` / `sell_short` place an order directly
* feat(trade_manage): the margin accounting is completed: an under-margined order is rejected and only the shortfall is borrowed (no more silent deposits inflating the return base); new borrow/return cost hooks `get_borrow_cash_cost` / `get_return_cash_cost` / `get_borrow_stock_cost` / `get_return_stock_cost`; the short and borrowed positions follow bonus / consolidation events and pay the lender's cash dividend (new `BORROW_ADJUST` / `SHORT_ADJUST` / `DIVIDEND_COMPENSATION` record types)
* feat(trade_manage): `Performance` supports pickle
* feat(slippage): the four random slippages (`SP_Uniform` / `SP_Normal` / `SP_LogNormal` / `SP_TruncNormal`) gain an `adverse_only` parameter (default `true` keeps the one-sided unfavorable cost; `false` applies the raw signed sample as zero-mean execution noise)
* feat(slippage): asymmetric long/short slippage: `SlippageBase` gains the cover / open-short side hooks (delegating to the long side by default, so an existing implementation keeps its behavior); `SP_FixedPercent` / `SP_FixedValue` gain `short_p` / `short_value`, and the four random models gain `short_mean` / `short_stddev` / `short_min_value` / `short_max_value` (`None` takes the long-side value); `crtSP` gains the matching optional arguments (an older archive carries no `short_*` key, so set them explicitly)
* feat(moneymanager): `MM_Nothing` supports opening shorts, sized by the account net assets (net assets / (price × unit)), consistent with the net-assets based strategies (`MM_FixedCapital` / `MM_FixedCapitalFunds` / `MM_FixedPercent`), so the borrowed proceeds no longer inflate the short size; a new `if_have_short_will_not_sell_short` parameter (default `False`) controls whether an existing short blocks a further short entry (previously the base default forbade it; the behavior changes to silently allowing it)
* feat(signal): new `SG_Manual` manual signal; `SG_Cycle` and custom cycle signals can read the injected rebalancing dates through `set_cycle_dates` / `get_cycle_dates` (modes A/B inject the whole range before the run, mode C declares on the day it is selected); new `get_value` / `get_buy_value` / `get_sell_value` read the signal values
* feat(environment): new `EV_Manual` market environment with `get_value` / `get_values` bound; `_add_valid` accepts an accumulation value and `ConditionBase` follows
* feat(multifactor): new `get_scores_np` / `iter_scores` numpy interfaces for large cross-sections (one structured array per section instead of one Python object per record), `get_scores` accepts a `ScoresFilterBase` directly, and `MultiFactorBase` binds the `reset` / `_reset` hooks
* feat(indicator): `VAR` / `VARP` / `COV` / `CORR` / `BETA` / `SKEW` / `KURT` / `SPEARMAN` and the other window statistics support the `n=0` expanding window; `ROUND` / `ROUNDUP` / `ROUNDDOWN` support a negative `ndigits`; `DATE` / `TIME` / `YEAR` / `MONTH` / `WEEK` / `DAY` / `HOUR` / `MINUTE` accept the date axis of the input indicator when used as a non-leaf; `WEAVE` warns when the result sets exceed the limit
* feat(KData): `operator==` / Python `==` now also compares the last record when the stock, query and size match, so a revised or appended realtime tail of a right-open query can trigger the downstream cache recomputation (still not a full element-wise comparison)
* feat(utilities): `initLogger` is exposed to Python and the logger state is thread-safe

**⚡️ Improvements**

* perf(pywrap): release the GIL broadly: whole-series K-data / calendar disk loads, pure C++ multi-factor computation, batch indicator calculation, account queries, factor read/write, analysis and batch helpers; the indicator numpy converters copy contiguous buffers in bulk and the Python sequence to vector path gains a numpy fast path
* perf(indicator): `EXIST` / `EVERY` / `BARSLASTS` replace the repeated rescans with a recurrence, the `IC` cross-section and `SPEARMAN` reuse scratch buffers, `SAFTYLOSS` is fully parallel, the rolling power sums use ring buffers, the `SMA` incremental pass starts at the framework start position, `TIME` drops the duplicated date-list copy and `BLOCKSETNUM` uses binary search
* perf(trade_manage): `updateWithWeight` short-circuits on a per-stock check frontier
* perf(selector): the `SE_Signal` index drops the per-signal weight, the operator selectors intersect in a single pass and reuse the cloned prototypes, `clone` no longer clones the runtime system list, and the multi-factor cross-section is filtered before copying
* perf(signal): the value operators merge-iterate and the logic operators use a merge join, `SG_Bool` aligns only on a size mismatch, and an unchanged cycle-date injection is skipped
* perf(factor): the `FactorSet` slow path becomes single-layer parallel over stocks
* perf(global): the spot records are shared across handler tasks and `getSpotMarketCode` uses string concatenation
* refactor(utilities): introduce the async DB connect base classes and a richer condition builder
* refactor(trade_sys): an invalid `MultiSystem.adjust_cycle` throws or warns at set time and mode B without an SE behaves like A/C (every sub-system counts as selected); `AF_FixedWeightList` rejects empty / all-zero / non-finite weights; the selector default names are corrected (`SE_Signal` / multiplication / operator selectors; only the `name()` / `str()` output is affected and archives still load)
* refactor(trade_sys): the PG / slippage / stop-loss subclass hooks are unified to the `_xxx` style (`_getGoal` / `_getRealBuyPrice` / `_getPrice`, i.e. `_get_goal` / `_get_real_buy_price` / `_get_price` in Python); the built-in components keep their behavior, custom subclasses must rename; a slippage subclass now computes the raw price only and the System validates, rounds and clamps it to the day range; `is_valid` becomes the overridable `_is_valid` hook (EV / CN aligned, default "finite and > 0")
* refactor(trade_sys): the parameter keys use the lowercase-underscore style (`auto_checkin` / `max_stock` / `out_extreme`); the old hyphenated keys throw at `set_param`; the old keys in an old archive are not migrated and fall back to the defaults after deserialization, so call `set_param` explicitly

**🐞 Bug Fixes**

* fix(KData): Adjustment fixed to a fixed-baseline convention: the forward adjustment is anchored at the last data day and the backward one at the data start, so adjusted values no longer depend on the queried range and `Stock::getKData` matches `kdata.getKData`; fix the double adjustment of the backward-adjusted KData sub-queries, the tail offset of the extended-range splicing, the doubled first-day volume/amount in weekly-and-above aggregation, the missed high update when a phase day makes both a new low and a new high, the out-of-bounds read on daily gaps, the extended ktypes wrongly using the daily adjustment formula and the missed first-bar adjustment of minute windows; adjustment is now price-only, the volume and the turnover amount keep their raw values (aligned with mainstream data sources such as Wind / JoinQuant; see the adjustment convention note of Query)
* fix(indicator): The ADJ_FACTOR series now uses the fixed baseline and matches RECOVER_EQUAL_BACKWARD value-by-value on the same range; the ADJ_VOL convention is changed to VOL / ADJ_FACTOR (price-only adjustment keeps the turnover amount unchanged)
* fix(stock/kdata): Fix the crash when fetching K-records through the preloaded buffer with a lowercase ktype, `getMarketValue` falling back to the latest price for queries earlier than the first record (future data leakage; now returns 0), the race between buffer release and queries (the buffer is now held by a shared_ptr so the views stay valid), the security position of the private buffer being computed more than once, the unchecked `blockDriver`, the reload concurrency race, an empty or out-of-range access returning a null record, `KRecord` using a scaled tolerance for a large amount/count, the writable `data()` removed from the shared-memory view, and the `StockManager` preload exception capture with the load-event callbacks dispatched from a lock-held snapshot
* fix(block): Fix the null-pointer dereference when calling getStockList on a null Block; add is now idempotent (adding an already-present security returns true, only a null stock or an unknown code returns false) and the list overloads return an all-of result without short-circuiting
* fix(strategy): Fix the inverted dedup logic of StrategyContext that made the stock code and ktype deduplication ineffective; stock codes are normalized to uppercase before dedup and stored uppercased (stock_list reads back normalized values and dedups correctly against the default must-load codes)
* fix(trade_manage): account reconstruction replays every trade record type (stock in/out, borrow/return, consolidation and the ex-rights adjustments); the trade entries reject a non-positive / non-finite number and an invalid price; `get_profit_cum_change_curve` returns Null when the invested base is 0, and an unclosed `total_profit` returns NaN
* fix(trade_manage): `BrokerTradeManager` prices a market order at the latest close, validates the checkin/checkout cash, keeps the last known state when the asset fetch fails, and revalues the snapshot holdings at the queried date
* fix(trade_sys): `PG_FixedPercent` computes the average cost as buyMoney / totalNumber (a partial sell no longer inflates the goal price and disables the take-profit), rounds to the stock price precision and implements the short goal; `PG_FixedHoldDays` counts the trading days of the traded object itself with a day table built at `setTO` (the O(n²) locked, IO-heavy query over a run becomes two binary searches); `ProfitGoalBase::clone` no longer carries the traded object, so a clone re-calculates at `setTO`
* fix(trade_sys): stop-loss / take-profit: `ST_Indicator` clones only its configuration and rebuilds the result at `setTO`, guards an empty or length-mismatched indicator; `ST_Saftyloss` rejects a non-positive / non-finite `p`
* fix(trade_sys): every trade path sizes the order at the actual fill price (after the slippage); a sell is capped at `maxTradeNumber` before the buy is clamped by the available cash, and a non-finite `maxTradeNumber` is rejected
* fix(trade_sys): signals: the `SG_Single` sell-side noise threshold is symmetric with the buy side (a down bar no longer triggers a sell every day), `Cross` / `CrossGold` / `Band` / `SG_Single` align to the K-data by date on a length mismatch, a combined signal re-calculates its sub-signals at `setTO` to clear stale signals, and the value-operator clone keeps the operand order
* fix(trade_sys): selectors: the operator selectors serialize and reload round-trip and rebuild their prototypes without doubling the weights; the `SE_Signal` prototype binding and clone mapping; the multi-factor lookup and zero-division weights; the multi-factor selector validates the recovery type range
* fix(trade_sys): environment / condition combination operators: `|` (Or) normalizes to 0/1, Multi/Div stay invalid when both sides are invalid, a non-finite value counts as invalid, and `EV_TwoLine` aligns to the K-data before comparing the lengths
* fix(trade_sys): money management: the MM `auto_checkin` deposits once at the actual buy (the quantity query no longer mutates the account), the quantity query uses the query-datetime cash instead of replaying the weight side effects, plus parameter and non-finite guards
* fix(indicator): non-finite values count as missing: `BARSLASTCOUNT`, the condition-family validity, the rolling sums and statistics, the `IIR` filters, the zero-variance family and the `INSUM` cross-stock aggregation are no longer permanently contaminated by NaN/Inf; a zero divisor and a zero variance return Null, `ZSCORE` of a constant series returns NaN, and `WINNER` / `TS_RANK` / `MRR` / the `ROC` family return Null on an invalid input
* fix(indicator): an incremental calculation that no longer fits the new context is rejected and recalculated, completing the discard declarations and the empty / out-of-range guards of `HHV` / `LLV` / `STDP` / `SLOPE` / `CVAL` / `ISLASTBAR` / `SUMBARS`; `HHVBARS` / `LLVBARS` break a tie on the most recent bar, `INTPART` truncates toward zero, the `ATR` first valid index matches `MA(TR)`, `SLOPE` uses relative coordinates, `AMA` fixes the warm-up `ER` lower bound and the rolling window, and `DEVSQ` propagates a window containing a missing value
* fix(indicator): the result buffers are serialized as bit patterns (legacy text archives still load), the indicator imp copy constructor is deleted to avoid a double free, an accessor of an implementation-less indicator returns Null, a cloned subtree keeps its parent links inside the clone, and the result-set index is bounds-checked; `HSL` / `COST` normalize the share count per ktype, `ZONGGUBEN` aligns its leading nulls, `ISLIMITUP` / `ISLIMITDOWN` judge on the backward-adjusted close across ex-rights dates, the period parameters are capped below INT_MAX and a `FINANCE` field-name error reports its context
* fix(multifactor): IC/ICIR output NaN when the weight sum is zero, the degenerate cross-sections and the normalize configuration propagate, `GroupSCFilter` uses quantile boundaries, `getICIR` resolves `ic_n` and validates `ir_n`, and `get_all_factors` behaves like `get_factor` when `save_all_factors` is off
* fix(strategy): an order number beyond the int64 lot conversion range is rejected and `order_value` warns without a current daily bar; a sell that is not an integer multiple of the minimum trade unit escalates to a full clear only when the position itself has an odd lot and the request covers it, otherwise it is truncated and ignored with a warning when the truncation reaches 0; the event loop `stop()` works and a restart no longer registers the timers and the heartbeat twice, single-session markets are supported, and `KeyboardInterrupt` stops gracefully
* fix(analysis): the combination names are deduplicated, `sort_key` is validated to avoid a silent empty result, out-of-range records are skipped, every combination runs on its own account, and the `analysis.py` wrappers validate their arguments and the `Performance.exist` usage
* fix(utilities): a task waiting for its own nested tasks no longer overflows the stack, `ResourcePool` timer timeout and coroutine resume races, `PluginLoader` nulls the handle after `dlclose`, HTTP connection reuse / IPv6 hosts / response limit, MySQL data truncation and column-length checks, `Datetime` and SQL integer handling, and `roundEx` for large and negative digits
* fix(global): the spot agent start/stop is serialized and its init flag atomic, each post-process handler is isolated, defensive JSON and FlatBuffers parsing, `readUUID` no longer posts uninitialized memory, the nng socket is closed on a failed option setup, and the version feedback parsing is hardened
* fix(factor): the `Factor` structural fields are immutable, `FactorSet` supports a negative index and bounds-checks it, removing a factor keeps the insertion order, and the factor name is validated at the save/query/remove entries
* fix(pywrap): callback and conversion safety: the Python-part detection, `boost::any` no longer uses `py::eval`, the `python_list_to_vector` / data-driver owners / numpy converters are exception- and lifetime-safe, pickle rejects a str state, and the inspect cache and the callback arity are checked by position
* fix(pywrap): `get_position_ext_info` returns a single `PositionExtInfo` again, and the overload returning the whole position dictionary is renamed `get_position_ext_info_dict`
* fix(draw): the cash flows are keyed by date, an undefined metric shows `N/A`, the performance is computed before the report text is rendered, and the Sharpe ratio and the return curve match the performance basis
* fix(i18n): the CSV header keys are translatable and the core catalog takes precedence, with the gettext catalog refreshed and the wording corrected
* fix(gui): saving the config preserves the manually tuned `hikyuu.ini` keys

**⚠️ Breaking Changes**

**MM / AF rework (`Portfolio` → `MultiSystem`)**

The former portfolio engine `Portfolio` (`SimplePortfolio` / `WithoutAFPortfolio`) is carried by `MultiSystem`.
The `PF_Simple` / `PF_WithoutAF` factories are kept (the function names, the parameter order and the defaults
unchanged), but they **return `MultiSystemPtr`**:

* The `Portfolio` class and its methods are no longer provided (Python keeps only the import-compatibility
  alias `Portfolio = MultiSystem`): drive the time axis with `set_axis_mode("calendar")` + `set_date_axis(...)`,
  `lastSuggestion()` becomes `last_suggestion`, and the query interfaces `getAdjustDates` / `getCycleEndDates` /
  `getRunningDates` / `getRealSystemList` are no longer provided;
* The `AllocateFundsBase` subclass hook changes from `_allocateWeight(date, se_list)` to
  `_allocate(date, tm, contexts, query, mode)` (L1), with the optional L2 `_to_targets` / L3 `_check_risk`; the
  first callback of the Python `crtAF` becomes `allocate_func(self, date, tm, contexts, query, mode)`, plus the
  optional `to_targets_func` / `check_risk_func`;
* The allocation modes (A/B/C) are owned by `MultiSystem` (`set_mode` / the read-only `mode`) and are passed to
  the AF hooks as the trailing `mode` argument; parameters without a meaning in the new system, such as
  `sys_use_self_tm`, are ignored with a warning.

See the docs: `trade_portfolio/portfolio` and `trade_portfolio/allocate_funds`.

**Same-day signal merging and the single-direction rule**

Multiple signal inputs on the same date are merged into one net value: a positive net keeps only the buy signal
and a negative net only the sell signal, while a net of about zero leaves no signal; with `alternate=true` a
same-direction same-day write only refreshes the value. Among the signal operators `+ - * /` keep the value and
`& |` keep only the direction. The `cycle` parameter now only marks "consumes the rebalancing dates injected by
`MultiSystem`" and no longer changes the calculation range, and it is mutually exclusive with `alternate=true`
(setting `cycle` forces `alternate=false` with a warning). The archive adds the cycle start/end and cycle-date
fields, so old `SG_*` archives (and systems embedding `m_sg`) must be re-saved.

**Behavior change: `TC_FixedA` redefined as the rolling current cost model**

`TC_FixedA` now always matches the current A-share rates: the transfer fee is 0.01 per mille of the turnover on
all markets in both directions, and the sell stamp duty is 0.5 per mille. The former per-share transfer-fee model
is renamed `TC_FixedAPre2015`, and `TC_FixedA2015` / `TC_FixedA2017` are frozen as historical conventions (the
2017 model now also charges the Shenzhen sell-side transfer fee, consistent with the buy side). A backtest using
`TC_FixedA` changes its trading costs accordingly; pick the model of the matching period explicitly for a
historical backtest. An old archive holding `TC_FixedA` interprets the per-share transfer fee as a rate, so switch
to `TC_FixedAPre2015` and re-save it. See `trade_manage/trade_cost`.

**Behavior change: the performance statistics basis is unified**

The return items of `Performance` now use the net assets (cash + long value - short value - borrowed cash) over
the cumulative net invested base (deposits - withdrawals + stocks in - stocks out); the time-related statistics
use natural calendar days with an annualization factor of 365; with less than two days of history, no invested
principal, or non-positive net assets the annualized items follow their definition (an empty value, except
`Account Avg Annual Return` which records 0); an undefined item is NaN (`report()` shows `N/A`) and `reset()`
clears every statistics slot. See `trade_manage/performance_indicator`.

**WalkForward window driving reworked**

`SYS_WalkForward` now locates the covering run window per datetime through a single driving entry: the window data
is built lazily and the candidate is re-anchored only on a window switch, so the first window is no longer skipped
(existing backtest results change), and moment driving without a prior calculation fails loudly; intraday driving
is available through `runMomentOnOpen` / `runMomentOnClose`, and a window switch honors
`clean_hold_when_select_changed`. `WalkForwardTradeManager` is a runtime-only view and no longer serializable,
so an old `SYS_WalkForward` archive **cannot be loaded and must be regenerated**. See `trade_sys/walkforward`.

## 2.8.3 - October 1, 2026

**🚀 New Features**

* feat(shm): Add a plugin-based shared memory data service; add the client/standalone shared-memory zero-copy K-line view, the K-line shared memory cache, IPC client-mode real-time update forwarding, and the base-info shared memory snapshot; in client mode, stock weights and historical finance data are now loaded lazily with local caching; the main process can pull K-lines directly from the quote cache server
* feat(gui): Add UI language selection with dynamic switching, and complete the English translation
* feat(i18n): Translate core C++/Python comments, docstrings and docs into English; add the English README, the Quick Start guide, the AI development guide (AGENTS.md) and a Chinese-English quant terminology glossary
* feat(slippage): Add the `seed` parameter for reproducible backtests; boundary handling for truncated normal rejection sampling; NaN validation for the NormalSlippage mean
* feat(plugin): ClickHouse/MySQL/HDF5 importers now support market and stock type registration; remove the market registration start-date parameter
* feat(data): Add a standalone auto-negotiated KData data service; `db_connect` adds `sqlStringLiteral` for safe SQL escaping; hkuextra client mode adds the `clearKDataCache` interface
* feat(hub): In non-Chinese environments, the default hub is cloned from GitHub by default
* refactor: Boost dependency requirement raised to 1.92.0


**⚡️ Improvements**

* perf(trade_manage): `getFundsList` now replays the trade list in one pass; add `getRefTradeList` to avoid copying the whole table on read-only access; fix margin/borrowing/asset accounting and duplicated dividend counting (#511)
* perf(trade_sys): Buyable quantity now uses binary search instead of per-lot decrement
* fix(trade_sys): Fix take-profit price mapping to unadjusted coordinates, the short stop-loss direction and exit check, and stop-loss/take-profit cache reset
* perf(indicator): The dynamic-parameter path uses scalar recursion instead of SLICE + rebuild
* fix(indicator): Unified fix for incremental calculation boundaries and out-of-bounds reads (ADJ_FACTOR/BACKSET/BARSSINCEN/ALIGN/SAFTYLOSS/ISLIMIT/QUANTILE_TRUNC/ATR/DIFF/REF/WMA/MACD/DMA/EXIST, etc.)
* fix(strategy): Fix order reference price, the sign and units of totalRisk, long-only continuous trading count, unified reserved-cash deduction in `can_allocate_cash`, per-training-window EV isolation in WalkForward, etc.
* fix(utilities): GlobalStealThreadPool switched to thread-local storage; fix move semantics and loading guards of MQStealQueue/ResourcePool/DllLoader/PluginLoader

**🐞 Bug Fixes**

* fix(data_driver): Fix cross-boundary aggregation of derived-period K-lines in the SQL backend, the Python custom KDataDriver `getIndexRangeByDate` returning `end` wrongly replaced by `start`, and the HDF5 date-index fallback logic; remove the redundant TMPCSV temporary mechanism
* fix(stock): Fix real-time update rejecting invalid K-line records and client-mode buffer gating; the shm client's daily auto-reload is delayed by 5 minutes
* fix(global): Fix the FlatBuffers validation range and the exception safety of parseFlatSpot/StockManager initialization
* fix(analysis): Fix null-pointer dereference for null tm/sys in the portfolio analysis entry, the date range and ending at the last trading day of the query; fix the Portfolio delayed sell-order removal condition and self-tm systems being retained in the WithoutAF selector pool
* refactor(clone): Simplify the clone implementation and strengthen subclass exception handling; System/Portfolio/WalkForward are now exception-safe
* fix(trade): Short broker orders now synchronize with the actual executed quantity
