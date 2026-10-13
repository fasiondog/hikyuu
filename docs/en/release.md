# Release Notes

## Unreleased

**🐞 Bug Fixes**

* fix(KData): Adjustment fixed to a fixed-baseline convention: the forward adjustment is anchored at the last data day and the backward one at the data start, so adjusted values no longer depend on the queried range and `Stock::getKData` matches `kdata.getKData`; fix the double adjustment of the backward-adjusted KData sub-queries, the tail offset of the extended-range splicing, the doubled first-day volume/amount in weekly-and-above aggregation, the missed high update when a phase day makes both a new low and a new high, the out-of-bounds read on daily gaps, the extended ktypes wrongly using the daily adjustment formula, the missed first-bar adjustment of minute windows and the crash of front()/back() on an empty KData; adjustment is now price-only, the volume and the turnover amount keep their raw values (aligned with mainstream data sources such as Wind / JoinQuant; see the adjustment convention note of Query)
* fix(indicator): The ADJ_FACTOR series now uses the fixed baseline and matches RECOVER_EQUAL_BACKWARD value-by-value on the same range; the ADJ_VOL convention is changed to VOL / ADJ_FACTOR (price-only adjustment keeps the turnover amount unchanged)
* fix(stock): Fix the crash when fetching K-records through the preloaded buffer with a lowercase ktype, getMarketValue falling back to the latest price for queries earlier than the first record (future data leakage; now returns 0), the race between buffer release and queries, the unchecked blockDriver, and the reload concurrency race
* fix(block): Fix the null-pointer dereference when calling getStockList on a null Block; add is now idempotent (adding an already-present security returns true, only a null stock or an unknown code returns false) and the list overloads return an all-of result
* fix(strategy): Fix the inverted dedup logic of StrategyContext that made the stock code and ktype deduplication ineffective; stock codes are normalized to uppercase before dedup and stored uppercased (stock_list reads back normalized values and dedups correctly against the default must-load codes)

**⚠️ AF risk control now active by default**: `max-single-position` defaults to 1.0, i.e. a single instrument position value <= the total assets (`<=0` disables the limit, `>1` takes effect as 1.0); the new `max-total-position` defaults to 1.0, i.e. in mode A the aggregated target market value of all instruments <= the total assets, scaled down proportionally with a warning when exceeded (`<=0` disables the cap). For over-allocation or leverage, lower the parameter or set it to `<=0`.

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
* fix(draw): The Avg Win/Avg Loss key of the payoff panel
* fix(stock): Improve the readability of parameter validation logs
