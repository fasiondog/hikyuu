.. highlight:: python

Multi-Market Data
=================

hikyuu was originally built for the single A-share market; the current version supports
registering multiple markets (futures / US stocks / HK stocks / forex / crypto, etc.)
within the same ``StockManager``, and guarantees correct accounting and reconciliation
for cross-market backtesting.

Registration and Market Info
----------------------------

- The ``Market`` table registers the basic market info, extended with three timezone
  columns: ``tz_offset_min`` (minute offset relative to UTC, may be negative),
  ``dst_rule`` (0=none / 1=US / 2=EU, DST computed by calendar rules without tzdata) and
  ``data_lag`` (the visibility delay in days relative to the base market).
- Stock types (``StockTypeInfo``) are registered per market convention; ``unit`` is the
  lot multiplier (e.g. the futures contract multiplier).
- Each market must have daily K-lines of its representative index (``market`` + the
  market default index code); ``get_trading_calendar`` builds the trading calendar from
  it; holidays are registered independently per market.
- Compatibility: ``BaseInfoDriver::getAllHolidays()`` now returns a per-market grouped map
  instead of a plain date list; custom driver subclasses (including Python ones) must
  update the signature accordingly.
- The Python ``MarketInfo`` supports pickle round-trips preserving the timezone columns.

Unified Time Expression
-----------------------

- **Storage**: each market is persisted with its exchange local wall clock (naive
  ``YYYYMMDDHHMM``), the existing A-share data needs no migration; daily and above keep
  the "trading day label" semantics and never participate in time conversion.
- **Read**: intraday and below are converted bidirectionally at the ``Stock`` layer —
  the query dates of the unified expression are reverse-normalized to the market local
  time on entry, and the bar datetimes are normalized to the unified expression on exit;
  ``MarketInfo.isUnifiedClock()`` (timezone +480 without DST) is the identity fast path;
  ``KQuery.isIntradayKType()`` decides which ktypes participate in the conversion.
- Day-frequency cross-market alignment is by trading-day labels: ``data_lag`` is a
  declarative field (US / forex = 1, A-share / futures / HK = 0) and the library does
  **not** shift bars automatically; the strategy layer must look back and take the last
  bar whose label is ≤ axis day − ``data_lag``. Otherwise the US / forex T-day close
  becomes visible on base-axis day T — one-sided look-ahead (the portfolio-level lag
  shift is tracked as a separate work item).

Dimension Conventions
---------------------

Everything follows the hikyuu API caliber; after importing or replacing a data source,
self-check with the three ratios:

- ``KRecord.amount``: **10K yuan (万元)**
- ``KRecord.volume``: lots (daily) / shares (minute and below)
- ``unit``: the lot multiplier (futures=contract multiplier, forex=contract size,
  stock / crypto=1); accounting = ``price × number × unit``

.. code-block:: python

    day = stk.get_kdata(Query(Datetime(202610270000), Datetime(202610300000)))
    m1 = stk.get_kdata(Query(Datetime(202610300930), Datetime(202610301500), Query.MIN))
    print('amount ratio', day[-1].amount / sum(r.amount for r in m1))   # ≈ 1
    print('count ratio', sum(r.volume for r in m1) / day[-1].volume)    # ≈ 100

Data whose ratios miss the target must not be imported: the minute volume has to be stored in
shares (daily in lots), and the minute amount in the same unit as the daily one. The minute
fixtures of SH600004 / SH000001 / SZ000001 inside the repository ``test_data`` are legacy (the
amount is about 100 times the expected value and the volume is stored in lots); they only serve
the driver read/aggregation tests, so do not assert amounts or magnitudes on them. SZ399001 is
the sample rebuilt to the caliber.

Variety Parameters and Runtime Overrides
----------------------------------------

The type level ``StockTypeInfo`` provides the default tick / tickValue / precision / lot size.
The variety-level differences (the futures contract multiplier, the HK lot size, the minimum
tick) have no dedicated column in the ``stock`` table and are applied at runtime:

- ``tick`` / ``tick_value``: ``unit = tick_value / tick`` follows the override immediately;
  keeping tick at 1 and setting ``tick_value`` to 10 yields the RB contract multiplier of
  10 tons per lot.
- ``min_trade_number`` / ``max_trade_number``: read directly by the lot check of
  ``TradeManager.buy/sell``.
- ``precision``: the rounding digits of the trade cost and the price.
- ``last_datetime``: the expiry date; afterwards ``buy`` / ``sell_short`` reject every new
  position (closing is unaffected).

.. code-block:: python

    rb = sm["SHFRB0000"]
    rb.tick_value = 10.0                         # unit = tick_value / tick = 10
    rb.min_trade_number = 1
    rb.last_datetime = Datetime(202612310000)    # no new position after the expiry

The overrides apply to the current process only and are not persisted: set them again in a new
process or after ``load_hikyuu`` (or write the type-level defaults into ``StockTypeInfo`` to
cover every variety of the same type in that market).

kdata Configuration
-------------------

The ``[kdata]`` section configures the data files per market with the
``{market}_{ktype}`` keys: keys are split at the first ``_`` (market codes must not
contain underscores, case-insensitive), and the ``time`` key maps to TIMELINE. Markets
or ktypes without configuration return empty data (silent zero output). HDF5 stores
prices fixed at ×1000 (a 3-decimal cap), so high-precision markets (forex / crypto)
should use MySQL / ClickHouse / SQLite instead.

The SQLite backend takes the table name from the code alone (the market is ignored), so with
``type = sqlite3`` each market needs its own database file (``sh_day`` / ``sz_day`` pointing at
their own ``.db``), otherwise varieties sharing a code (e.g. SH000001 and SZ000001) read each
other's rows; inside a file, tables are named after the lowercased code.

Load Scope
----------

- ``load_hikyuu(stock_list=[...])`` or the ``HKU_STOCK_LIST`` environment variable
  (``;``-separated) limits the load scope explicitly: securities not listed are not
  registered, so their K-lines are not loaded and they take no part in the preload; a code
  that can not be resolved is warned about and skipped.
- Security codes are stored in uppercase in both the base info and the K-line storage (the
  import side guarantees it); the internal matching and lookups normalize to uppercase as well,
  so the case in the list never matters and the CRYPTO slash form (``BINANCE/BTCUSDT``) is
  accepted (the SQLite / MySQL K-line table names are lowercased by the driver).
- ``SH000001`` / ``SH000300`` are the default must-load codes and are always registered
  whatever the list says.
- ``load_hikyuu(market_list=[...])`` limits what ``stock_list=["all"]`` loads by market, and
  ``stock_type_list=[...]`` does the same by security type; giving both takes the intersection.
  An empty list means no restriction, and ``ALL`` inside the market list also means no
  restriction. Securities outside the scope are not registered, take no part in the preload,
  and the column-first bulk K-line load falls back to the per-security preload. The
  representative index of every market inside the scope (the index recorded by ``MarketInfo``)
  is always kept, so the trading calendar still works when ``stock_type_list`` leaves the index
  type out; the other block indexes still follow the type filter.
- ``HKU_STOCK_LIST`` is consumed only by the Python ``load_hikyuu`` (the C++ ``hikyuu_init``
  binding always loads the full scope; C++ callers narrow it with the ``load_market_list`` /
  ``load_stock_type_list`` hikyuu parameters).
- When ``stock_list`` is used, the representative indexes of the markets in use must be
  included in the list, otherwise their trading calendars are empty.
- Preloading still takes effect globally per ktype (there is no per-market preload switch),
  but only the registered securities take part in it.

Portfolio Calendar
------------------

``MultiSystem.run(query)`` is driven by the **A-share (SH) trading calendar** by default: a
cross-market portfolio that sets no axis runs on the SH days, so the days unique to the other
market are skipped and the SH-only days run empty (NYSE vs SH over 2005-01-04~2011-12-06 differ
by 115 / 54 days). Inject the target calendar explicitly:

.. code-block:: python

    q = Query(Datetime(20050104), Datetime(20111206), Query.DAY)
    ms.set_axis_mode("calendar")
    ms.set_date_axis(sm.get_trading_calendar(q, "NYSE"))  # "SH" by default
    ms.run(q)

Capability Boundaries
---------------------

- **MySQL upgrade scripts contain DDL and cannot be re-run directly after an
  interruption**: DDL commits implicitly, so re-running ``0032`` / ``0033`` after an
  interruption fails at ADD COLUMN with Duplicate column — recover manually per the
  script header comments (or rebuild the dev database from scratch); the SQLite /
  ClickHouse upgrade scripts are idempotent and re-runnable.
- **The SQLite backend supports K-lines only**: no timeline / trans data (use HDF5 /
  MySQL / ClickHouse); the table name only uses the code (ignoring the market), so
  same-code cross-market shares within one database file would collide — **each market
  and ktype must use an independent database file**; the minute buckets of the
  ``convert`` aggregation are hardcoded to the A-share sessions and apply to A-shares
  only.
- **Night sessions / multiple sessions**: ``isTransactionTime`` supports two sessions
  only and returns false across midnight; it affects the realtime spot filtering only,
  not backtesting.
- **No mark-to-market or forced liquidation in futures backtesting**: no settlement
  price, no occupied-margin snapshot, no forced-liquidation flow; the futures short
  side has no borrow record (``borrow_asset == 0``). Use ``net_assets() / profit()`` as
  the funds caliber; ``total_assets()`` is inflated by ``borrow_asset`` when shorting
  stocks.
- **Multi-currency accounting**: ``cash / market_value`` etc. are summed without a
  currency dimension; cross-currency markets should be pre-converted to a single base
  currency on the data side.
