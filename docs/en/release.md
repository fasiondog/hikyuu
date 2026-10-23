# Release Notes

## Unreleased

**⚠️ Breaking change: MM / AF responsibility separation (the Portfolio / AllocateFunds rework)**

The former standalone portfolio engine `Portfolio` (`SimplePortfolio` / `WithoutAFPortfolio`) is now carried by `MultiSystem`;
the portfolio-level fund allocation is split out of `MoneyManagerBase` and carried by the standalone base class `AllocateFundsBase` (AF):

* The `PF_Simple` / `PF_WithoutAF` factories are kept (the function names, the parameter names, the parameter order
  and the defaults unchanged), but they **return `MultiSystem`**, with the `af` parameter type changed from
  `MMPtr` to `AFPtr`;
* The `AF_EqualWeight` / `AF_FixedWeight` / `AF_FixedWeightList` / `AF_FixedAmount` / `AF_MultiFactor`
  factories are kept, but they **return `AllocateFundsBase` instances** (`AFPtr`);
* `MoneyManagerBase` is **restricted to the single-system / single-security** form: the portfolio-level interfaces
  are removed (`allocate` / `set_mode` / `_allocate_system_weight` / `_allocate_suggestions` / `_check_risk`
  and the `mode` attribute);
* The `Portfolio` class and its methods are **no longer provided**; the Python `crtAF` is **kept**, its callback
  contract updated from `_allocateWeight(date, se_list)` to `_allocate(date, tm, contexts, query)`, with the
  optional `_to_targets` / `_check_risk` injection.

Migration points:

* Calls of `PF_*` / `AF_*` with positional arguments need no change (the `PortfolioPtr` → `MultiSystemPtr` and
  `AFPtr` → `AllocateFundsPtr` compatibility aliases are provided);
* The `Portfolio` class methods are replaced by the `MultiSystem` equivalents: use `set_axis_mode("calendar")` +
  `set_date_axis(...)` for the driving time axis; `lastSuggestion()` → `last_suggestion` (the trade records
  of the last day plus the delayed requests; an aggregate system also offers `to_suggestions()` returning
  the `TradeSuggestion` list it converted);
  the query interfaces such as `getRunningDates/getCycleEndDates/getRealSystemList` are no longer provided;
* Custom fund allocation: build quickly with `crtAF(allocate_func[, to_targets_func, check_risk_func])`, or subclass
  `AllocateFundsBase` and override `_allocate` (L1) / `_to_targets` (L2) / `_check_risk` (L3);
* The portfolio allocation modes (A/B/C) are owned by the aggregate `MultiSystem` itself (`set_mode` / the
  read-only `mode` attribute) and are passed into the AF hooks as the trailing `mode` argument; serialization
  stays backward compatible (the `m_mode` in the old archives, previously held by the AF, migrates to
  `MultiSystem` automatically);
* The parameters without a corresponding semantic in the new system, such as `sys_use_self_tm`, are ignored with a
  warning; `AF_MultiFactor` provides the `auto_adjust_weight` parameter (default `True`, the scores are normalized
  to sum 1 before allocating, and a score sum below 1 also allocates fully; with `False` the score itself is the
  final weight).

**⚠️ AF risk control is active by default**: `max_single_position` defaults to 1.0, i.e. a single instrument position value <= the net assets (`<=0` disables the limit, `>1` takes effect as 1.0); `max_total_position` defaults to 1.0, in mode A the aggregated target market value of all instruments <= the net assets, in mode B the sum of the sub-system quotas <= the net assets; when exceeded the **weight pricing is scaled down proportionally** while the **fixed amounts are truncated to whole quanta** (with a warning reporting how many were funded; `<=0` disables the cap). For over-allocation or leverage, raise it or set it to `<=0`. The base class also carries the `fixed_amount` parameter (when >0 it overrides the weight pricing with a warning; being an **absolute amount** it is never scaled down, only whole quanta are funded and the rest gets a zero quota) and `adjust_running_sys` (default `True`, mode B only; when `False` the running sub-systems keep their holdings and only the new sub-systems get the remaining cash). The executor also supports `auto_checkin` (default `False`: deposits extra principal for the gap computed from the money plus the commission, which inflates the backtest equity curve and may top up on every bar whose targets change) and margin buying power (`support_borrow_cash` set on the `MultiSystem`; the buying power is then derived from the margin rate **per order**, so a multi-leg batch is funded in order and later legs may be shrunk).

See the docs: `trade_portfolio/portfolio` and `trade_portfolio/allocate_funds`.

**⚠️ Breaking change: `AF_FixedWeightList` rejects an empty weight list**

`AF_FixedWeightList([])` no longer degrades silently to an `AF_EqualWeight` instance: like an all-zero or non-finite list it throws at both the factory and `set_param("weights", [])` (an empty list cannot price anything). A length that does not match the number of sub-systems still falls back to the equal weight.

**⚠️ Breaking change: profit goal strategy (PG) subclass interface normalization**

`ProfitGoalBase::getGoal` / `getShortGoal` are now non-virtual public interfaces; the subclass hooks are unified
as `_getGoal` / `_getShortGoal` (consistent with the `_xxx` style of `_reset` / `_clone` / `_calculate`):

* C++ custom subclasses: the `PROFITGOAL_IMP` macro is updated accordingly; rename the `getGoal` implementation
  to `_getGoal`; `_getShortGoal` is optional and returns constant.null_price by default (no short goal set);
* Python custom profit goal strategies: the override hook `get_goal(self, datetime, price)` is renamed to
  `_get_goal(self, datetime, price)`, with the optional `_get_short_goal`; the public query methods
  `get_goal` / `get_short_goal` remain;
* `crtPG(get_goal, ...)` keeps its signature and usage; the internal callback contract is renamed from
  `get_goal` to `_get_goal`.

**⚠️ Breaking Changes: Slippage interface reworked (the subclass computes only the raw price; the System part validates and clamps it)**

A custom `SlippageBase` subclass (C++/Python) previously overrode `get_real_buy_price` / `get_real_sell_price` to return the final trade price directly. It must now override `_get_real_buy_price` / `_get_real_sell_price` to compute only the **raw** price. The System part - the only caller in the framework, which already holds the original (unadjusted) record of the bar - then validates the result and clamps it into the bar's range:

* an invalid result (non-finite or non-positive) falls back to the plan price: 0 is accepted by the trade manager as the market-order placeholder, so it must never come from the slippage path;
* the model price is rounded to the market's minimum price tick, so a fill never lands between two ticks (the plan price and the bar bounds are raw market prices and are used as they are);
* both sides are clamped into the bar's `[low, high]` range.

Migration:

* Python custom slippage (`crtSP` subclasses): rename `get_real_buy_price` / `get_real_sell_price` to `_get_real_buy_price` / `_get_real_sell_price` (raw price only); `get_real_buy_price` / `get_real_sell_price` now return the raw price of the model, and the price used by the trade is the fixed-up one.
* C++ custom slippage: move the implementation from `getRealBuyPrice` / `getRealSellPrice` into the protected virtuals `_getRealBuyPrice` / `_getRealSellPrice`.
* If your custom slippage clamped to the day range internally, drop that logic (the System part does it now).
* `SP_FixedPercent` no longer emits the invalid-plan-price warning (`HKU_WARN`), matching the other five built-in slippage implementations.

**⚠️ Breaking Changes: StoplossBase interface reworked (subclass implements only _getPrice / _getShortPrice)**

A custom `StoplossBase` subclass previously overrode the public virtuals `getPrice` / `getShortPrice`. It must now override the protected virtuals `_getPrice` / `_getShortPrice` to compute only the raw stop-loss price; the public `getPrice` / `getShortPrice` become non-virtual wrappers that delegate to the subclass hooks (same form as `reset` / `_reset`).

* C++ custom stop-loss / take-profit: move the implementation from `getPrice` / `getShortPrice` to `_getPrice` / `_getShortPrice`. Python custom stop-loss / take-profit: rename the override methods from `get_price` / `get_short_price` to `_get_price` / `_get_short_price` (the `crtST` helper is updated accordingly).

**⚠️ Breaking change: the money manager common parameters renamed (unified to the lowercase-underscore style)**

The money management base-class common parameters `auto-checkin` → `auto_checkin` and `max-stock` → `max_stock` are renamed to match the lowercase-underscore naming style of the per-strategy parameters (`max_loss`, `risk`, `capital`, ...):

* Calling `set_param` / `get_param` with the old hyphenated names raises an out-of-range exception; use the new names instead;
* Only these two common parameters are affected, the per-strategy parameter names are unchanged;
* Examples: `mm.set_param("auto_checkin", True)`, `mm.get_param("max_stock")`.

**⚠️ Breaking change: same-day signal merging and the single-direction rule**

Multiple signal inputs on the same date are merged into one net value: a positive net keeps only the buy signal, a negative net only the sell signal (the opposite record of that date is removed), and a net of about zero leaves no signal on that date; with `alternate=true`, a same-day record that already points in the same direction is only refreshed with the net value, not treated as a new entry. The new Python bindings `get_value` / `get_buy_value` / `get_sell_value` expose the signal values.

**⚠️ Breaking change: the indicator parameter `out-extreme` renamed to `out_extreme`**

`ZSCORE` / `NORM_Zscore` and the other indicators/normalizers using `out_extreme` follow the lowercase-underscore style, and `set_param` with the old hyphenated name raises an exception.

**⚠️ Archive format changes**: `SignalBase` adds the cycle start/end and cycle-date fields, and the operator signals no longer serialize the base class twice, so old `SG_*` archives (and systems embedding `m_sg`) must be re-saved; hyphenated parameter keys in old archives (`auto-checkin` / `max-stock` / `out-extreme`) are not migrated automatically and fall back to the defaults after deserialization, call `set_param` explicitly to restore them.

**🚀 New Features**

* feat(portfolio): the `PF_Simple` / `PF_WithoutAF` compatibility factories (passing through to `MultiSystem`, mode B / mode C)
* feat(allocatefunds): the standalone `AllocateFundsBase` base class with the `AF_EqualWeight` / `AF_FixedWeight` / `AF_FixedWeightList` / `AF_FixedAmount` / `AF_MultiFactor` factories (carrying the portfolio-level L1/L2/L3)
* feat(allocatefunds): restore the Python `crtAF` quick construction (L1 `_allocate`, with the optional L2 `_to_targets` / L3 `_check_risk` injection)
* feat(MultiSystem): new `axis_mode` / `adjust_cycle` / `adjust_mode` / `delay_to_trading_day` / `trade_on_close` / `sell_at_not_selected` parameters; the run-time switches are snapshotted at `readyForRun` and fully illegal values are rejected at set time, supporting internalizing the adjust mode into an adjust-date table
* feat(slippage): the four random slippages (`SP_Uniform` / `SP_Normal` / `SP_LogNormal` / `SP_TruncNormal`) gain an `adverse_only` parameter (default `true` keeps the one-sided unfavorable cost; `false` applies the raw signed sample as zero-mean execution noise)
* feat(slippage): asymmetric long/short slippage is now supported: `SlippageBase` gains `get_real_buy_short_price` (covering a short) and `get_real_sell_short_price` (opening a short) with their overridable hooks, which delegate to the long-side hooks by default so an existing implementation keeps its behavior; `SP_FixedPercent` / `SP_FixedValue` gain `short_p` / `short_value`, the four random models gain `short_mean` / `short_stddev` / `short_min_value` / `short_max_value` (each taking its long-side counterpart value at construction when left `None`); `crtSP` gains the optional `get_real_buy_short_price` / `get_real_sell_short_price` arguments (note: an archive serialized by an older version carries no `short_*` key, so after deserialization the short side takes the class default instead of the matching long-side value; call `set_param` explicitly to restore it)
* feat(moneymanager): `MM_Nothing` now supports opening shorts, sized by the account net assets (the net assets ÷ (price × unit)), consistent with the net-assets based strategies (`MM_FixedCapital` / `MM_FixedCapitalFunds` / `MM_FixedPercent`) in using the net assets as the short base, so the borrowed proceeds no longer inflate the short size; a new `if_have_short_will_not_sell_short` parameter (default `False`) controls whether an open short blocks a further short entry (previously the base default forbade short entry; the behavior changes to silently allowing it)

**🐞 Bug Fixes**

* fix(KData): Adjustment fixed to a fixed-baseline convention: the forward adjustment is anchored at the last data day and the backward one at the data start, so adjusted values no longer depend on the queried range and `Stock::getKData` matches `kdata.getKData`; fix the double adjustment of the backward-adjusted KData sub-queries, the tail offset of the extended-range splicing, the doubled first-day volume/amount in weekly-and-above aggregation, the missed high update when a phase day makes both a new low and a new high, the out-of-bounds read on daily gaps, the extended ktypes wrongly using the daily adjustment formula, the missed first-bar adjustment of minute windows and the crash of front()/back() on an empty KData; adjustment is now price-only, the volume and the turnover amount keep their raw values (aligned with mainstream data sources such as Wind / JoinQuant; see the adjustment convention note of Query)
* fix(indicator): The ADJ_FACTOR series now uses the fixed baseline and matches RECOVER_EQUAL_BACKWARD value-by-value on the same range; the ADJ_VOL convention is changed to VOL / ADJ_FACTOR (price-only adjustment keeps the turnover amount unchanged)
* fix(stock): Fix the crash when fetching K-records through the preloaded buffer with a lowercase ktype, getMarketValue falling back to the latest price for queries earlier than the first record (future data leakage; now returns 0), the race between buffer release and queries, the unchecked blockDriver, and the reload concurrency race
* fix(block): Fix the null-pointer dereference when calling getStockList on a null Block; add is now idempotent (adding an already-present security returns true, only a null stock or an unknown code returns false) and the list overloads return an all-of result
* fix(strategy): Fix the inverted dedup logic of StrategyContext that made the stock code and ktype deduplication ineffective; stock codes are normalized to uppercase before dedup and stored uppercased (stock_list reads back normalized values and dedups correctly against the default must-load codes)
* fix(trade_sys): PG_FixedPercent now computes the average cost as buyMoney / totalNumber, so a partial sell no longer inflates the goal price and disables the take-profit; the goal price is rounded to the stock price precision (up for a long, down for a short, after sanitizing the float noise) instead of returning an intransactable price; implement the short goal; return no goal instead of dereferencing a null trade manager
* fix(trade_sys): PG_FixedHoldDays now counts the held trading days of the traded object itself with a day table built at setTO, replacing the per-bar daily driver query (O(n²) over a run, with locks and IO) by two binary searches; it no longer fails silently when the stock has no daily data
* fix(trade_sys): ProfitGoalBase::clone no longer carries the traded object, so a clone always re-calculates at setTO (a pre-computing custom subclass used to stay silently stale on the clone); the "no short goal" sentinel of _get_short_goal is unified to constant.null_price, matching the long side
* fix(signal): same-day signal inputs are merged into one net value with a single direction per date: a zero net no longer leaves a stale signal behind (previously `shouldBuy`/`shouldSell` could both hold), and with `alternate=true` a repeated same-day write refreshes the value instead of dropping it
* fix(KData): `operator==` / Python `==` now also compares the last record when the stock, query and size match, so a revised or appended realtime tail of a right-open query can trigger the downstream cache recomputation (still not a full element-wise comparison)

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
