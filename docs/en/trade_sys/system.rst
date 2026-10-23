.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

System Strategy|SYS
===================

A System is a complete trading strategy for a single instrument. It bundles together the market environment, the system validity condition, money management, the stop-loss, the take-profit, the profit goal, and the slippage parts, and is used to run backtests.

For multiple instruments (multiple securities), :class:`MultiSystem` can be used to aggregate multiple System instances (each for a single security, or even a nested :class:`MultiSystem`) into a portfolio that is backtested uniformly under the same trading account (TM). Sub-systems are added to :class:`MultiSystem` through ``add(subsystem)``; it supports **arbitrary nesting** and circular-reference detection. Each sub-system keeps its own SG/MM/EV/CN/ST/TP/PG/SP strategies, and only the parent keeps the books and places orders uniformly.

:class:`MultiSystem` supports three run modes (``set_mode`` / the read-only ``mode`` attribute; the mode is owned by the aggregate system itself and is also configured by the preset PF factories):

- **Mode A "Signal Aggregation" (the default)**: each sub-system holds a signal "shadow account" (the signal cash set via ``set_sub_init_cash``, run continuously and not reset between bars; ``run(reset=True)`` still resets them with the whole system) and runs continuously as an independent signal source; on the rebalancing day the parent reads the **current intent state** (net exposure holdings plus the unfilled requests) of the **selected** sub-systems, synthesizes one **signed net target per instrument** by the AF L2 (weight x net ratio x the parent net assets) and trades the delta on its own account (long and short net out). The unselected sub-systems contribute a zero signal (their instruments drop out of the target and are closed). The shadow bookkeeping never touches the real funds. The sampling folds the **queued (unfilled) delayed requests of the nested subtree leaves** into the net exposure ratio, so the pending intent of a nested C layer is visible.
- **Mode B "Fund Allocation" (quota allocation, FOF/MOM style)**: on every rebalancing day the AF allocates the quota (target total assets) to each selected sub-system first (L1); each selected sub-system is calibrated to its quota BEFORE it is driven (zero-quota liquidation, then the unselected liquidation or the selected cash recycle, then reduce the over-quota part, inject the gap) and trades with the exact quota; the parent mirrors the real instructions of the sub-systems continuously (L2 pass-through). A composite child is reduced pro rata over its leaf holdings (a suspended leaf is valued at its last available close), and the lot-rounding residual together with the unsellable suspended part is covered by selling lots from the largest remaining holdings until the gap is met (below one lot remains). The calibration legs queued by an earlier round count as sold: their value is deducted from the residual base, and when they already cover the whole gap they are trimmed to the gap target (multiple legs of the same instrument keep the front legs greedily), with the lot-rounded residual topped up to the gap by the existing residual loop (below one lot remains). The quota cash is a virtual budget: ``checkin/checkout`` only touch the sub ledger and never move money out of the parent account, and the injections of one rebalancing day are cumulatively capped by the parent's remaining cash, so the mirrored trades stay fundable and the two books (positions / trades) stay matched.
- **The SE convention of mode B**: **without an SE every sub-system is treated as selected** (the mode A/C convention) and the quota is allocated as usual; an SE present but selecting nobody on the rebalancing day means "nothing selected", and ``sell_at_not_selected`` decides whether to clear them.
- **Mode C "Shared Account Compatibility"**: no shadow account is created; the sub-systems trade DIRECTLY on the single real account of the parent (``shared_tm``) and are sized by their own MM, running continuously and exiting on the signal day; the parent only gates the entry (SE) + records + optionally risk-controls, and does no L2 conversion. It keeps the compatibility with the ``Portfolio`` before the refactoring.

.. note::

   Mode C is not a parameter special case of mode B: it cannot be reproduced by any parameter
   setting of mode B (any ``adjust_running_sys`` value, an AF that performs no allocation, turning
   off the parent proportional scaling, and so on). The two are **two different account topologies**
   -- in mode B the sub-systems run in **isolated shadow accounts** and the parent mirrors their
   trades continuously, while in mode C the sub-systems **share the parent real account** directly
   (a deliberate exception). With a shared account the sub-systems **compete for the same cash pool**
   (whoever is driven and fills first takes the cash), which mode B does not exhibit.

Nested and mixed modes
----------------------

A sub-system may itself be a :class:`MultiSystem` (arbitrary depth). Two rules apply when nesting: the **outer mode decides the account topology** (which account the child gets), and the **inner mode decides how its own sub-systems are scheduled and selected**.

.. list-table:: Outer mode x inner mode
   :header-rows: 1
   :widths: 14 30 30 26

   * - Outer \\ Inner
     - A (Signal Aggregation)
     - B (Fund Allocation)
     - C (Shared Account)
   * - A
     - The child gets a **dedicated shadow account** (initial ``set_sub_init_cash``); the inner layer runs on it as its parent account and executes its L2 net targets there; its leaves build their own shadows
     - The child gets a **dedicated shadow account**; the inner layer runs its L1 quota on it (quota = weight x the account net assets) and its leaves build quota accounts
     - The leaves of the inner C share that **dedicated shadow account** (not the outer real account)
   * - B
     - The child gets a **dedicated quota account** (starting from zero); the outer injects the quota first, then the inner executes its L2 net targets on it
     - The child gets a **dedicated quota account** (starting from zero); after the outer injection the inner runs its own quota allocation for its leaves (nested quotas)
     - The leaves of the inner C share that **dedicated quota account**
   * - C
     - **No dedicated account**: the inner uses the shared account as its parent account and executes "weight x shadow net exposure ratio x the shared net assets" directly on it; the orphan / zero targets are limited to its own subtree instruments
     - quota = weight x the shared net assets; the injection is virtual bookkeeping on the inner leaf quota accounts, only the reduction parent leg acts on the shared account; several children compete for the same cash pool and the holdings are not attributable per child
     - All levels share the same account (a deeper legacy Portfolio)

**Key points**

- **Account ownership**: an A/B aggregate requires an "exclusive book" -- the outer layer creates a dedicated shadow/quota account for it. Mode C hands the parent account directly to the child (shared semantics). Therefore, with A/B under C the child's "parent account" is the shared account: both the A net asset base and the B quota base are the shared net assets, and several children compete for the same cash pool -- control each size explicitly with the SE weights.
- **The parent only records the mirror**: an A/B outer layer mirrors the **trades** of the inner layer onto its own real account (B also calibrates the quota); the children of a C outer layer trade directly on the shared account, the parent never orders again.
- **Same-batch netting**: the opposite legs of the same instrument in one batch are netted before execution (e.g. an open delayed fill plus a close reversal), so no phantom parent position is left behind; a CLEAR only clears the **attributable part** (the sum of the clearing legs, capped by the parent holding plus the same-side same-batch buys), never swallowing a sibling child's same-batch buy or holding.
- **`trade_on_close=False`**: the mode B quota calibration legs (reduction / liquidation), the quota injections and the signal suggestions are all deferred to the **next open** (sells before the injections), sharing one price basis; the turnover is recorded where the open executions happen. A leg with no quote (a suspension) is **retained and retried**: the deferred calibration legs and the queued suggestions both wait for the next quoted open (a retried suggestion batch is merged into a new rebalancing batch), so nothing fills at a stale price on a suspended day. A deferred injection waits for the deferred sells of the same rebalancing; the continuous-mirror fills on non-rebalancing days are not counted into the turnover. A leg whose holding is already gone (e.g. force-cleared on a delisting) is dropped instead of blocking the injections. A parent-side residual with no sellable quantity left is dropped instead of blocking the injections. With ``trade_on_close=True`` a liquidation/reduction close on a suspended day falls back to the deferred legs of the next open (the executor retries), so nothing fills without a quote.
- **Two-book matching**: the mode B invariant "shadow book = parent book" holds only with a dedicated account; on a shared account (C) the holdings are not attributable per child.
- **Liquidation semantics**: ``sell_at_not_selected`` only applies to its own layer. In B, when unselected: ``True`` clears both the parent and the shadow book together; ``False`` keeps both books running (only the quota calibration of the round is skipped; **the child is not driven at the close of a rebalancing day**, only its open fills are mirrored). A selected sub-system with a zero L1 quota is liquidated by the same rules (gated by the switch); a running sub with ``adjust_running_sys=False`` is exempt -- it keeps running without a budget injection. In C, when a child leaves the pool: ``True`` recursively clears the internal books of the subtree and drops the queued mirror batch; ``False`` keeps the child books frozen (matching the untracked holding that stays on the shared account).
- **Cycle and rebalancing cadence**: the outer pre-injection only covers the own SG of the direct sub-systems (a composite has no SG); the inner A/B layers declare the cycle days at runtime on their own rebalancing days. An inner layer counts **driven days**: when the outer B skips it (``False``) or the outer C does not admit it, its counter does not advance, so an inner ``adjust_cycle=N`` means "every N driven days". The calendar ``adjust_mode`` (week/month/quarter/year) always uses the **runtime period rule** (the N-th calendar day of the period; with ``delay_to_trading_day`` postponed to the first driven day of the period; once per period), shared by the bulk ``run`` and the single-bar live feed; an explicit ``set_adjust_dates`` takes precedence. A nested mode A child already folded its leaf intents into its target on its sampling day: the outer layer folds only its **queued batch**, never its leaves again (no 2x); on a non-sampling day its raw leaf requests are folded.
- **B under B above the quota**: the outer mode B hands the target quota down to the inner mode B, which reduces its own leaves by its AF weights (the budget model), keeping both book levels matched with the outer parent.
- **Unadmitted nested aggregates**: not driven, no cycle day declared, no trading (expected).
- **The shared boundary does not propagate down the chain**: the mode C "holdings are not attributable" rule applies only to its **direct children**; in ``C -> A -> B`` the third layer B gets a dedicated shadow book funded by the A layer's sub_init_cash (attributable again), and only ``C -> C -> *`` stays fully shared. A deferred leg advances **one hop per layer** (toc=false or a suspended close fills at each layer's next open), so a three-level chain lags the leaf by up to 2 trading days.
- **Overlapping instruments across children**: in B an unselected liquidation caps the parent sell by **both** the parent holding and the **child own book**, while a deferred reduction leg is capped by the **parent holding** at the close. On the shared account (C) the holdings are not attributable -- the orphan / zero targets of A are limited to its own subtree instruments, but the **quantity is executed against the shared net position**, so avoid overlapping instruments across children under C.
- **SE and warnings**: every layer has its own SE; without an SE in mode C only the **direct** cycle SGs stay silent with a one-time warning (nested aggregates are handled by their own modes). In mode C a cycle SG cannot be combined with ``SE_Signal`` either: the SE snapshots the already computed buy signals before the run, while the cycle day is declared only on admission, so such a sub-system is never admitted; and because mode C declares the cycle day on admission only, a stop-loss exit inside the same cycle does not re-open by itself.
- **Cascade example**: ``C -> B -> A`` means a quota sub-portfolio B under a shared account, with a signal-aggregation sub-portfolio A inside; A's shadow only serves the B sampling, the B mirror lands directly on the shared account, and the outer C surfaces it without ordering again.

The SE, ``sell_at_not_selected``, rebalancing cadence and cycle-day declarations of each layer are independent and never inherited.

Aggregated backtesting also supports:

- **AF L1/L2/L3 layering** (see :doc:`../trade_portfolio/allocate_funds` for details): L1 performs system-level allocation (nominal weights in mode A / real quotas in mode B; e.g. equal weight 1/N, fixed weights, weighted by the SE score), L2 performs behavior-level conversion (in A it reads the sub-system's current intent state and synthesizes the **signed net target** per instrument as weight x net ratio x the parent net assets, rebalancing by the delta with the long/short netting out; in B the instructions pass through), and L3 provides portfolio risk control (the concentration limit ``max_single_position``, <=0 means no limit, the default 1.0 caps a single instrument at the total assets; skipped in mode B/C).
- **Rebalancing cycle** (``set_adjust_cycle``): by default, rebalancing happens on every close day; once the number of days is set to a value greater than 1, rebalancing occurs only on the cycle days. In the period modes (week/month/quarter/year) the value is the n-th calendar day within the period (the week starts on Monday, so only 1..5 can hit a driven day): ``0`` is warned and treated as 1 at ``readyForRun`` (a negative value is rejected at set time); a value that can never reach a driven day of the period (week >5 / month >31 / quarter >92 / year >366) is warned but **not clamped**, and that mode then never rebalances. The target is computed from the **start of the current period**, so a non-trading target is postponed only within that period and never drifts into the next one; a value near the period end (e.g. the 31st in a short month, the quarter/year end) may therefore skip a whole period.
- **Parameter aliases**: ``adjust_cycle`` (int, default 1), ``trade_on_close`` (bool, default True) and ``sell_at_not_selected`` (bool, default False) are also exposed as parameters equivalent to their setters (fully illegal values are rejected at set time, e.g. ``adjust_cycle < 0`` and an invalid ``adjust_mode`` / ``axis_mode``; the ``adjust_cycle == 0`` normalization to 1 (a negative value is rejected at set time), the ``adjust_mode`` case normalization, the cycle-date cache reset and the out-of-range warning are all handled at ``readyForRun``).
- **Instrument selection (SE)** (``set_se``): optional; on a rebalance day, only the selected sub-systems are sampled. In modes B/C the unselected ones can be force-liquidated / have their quota recycled (``set_sell_at_not_selected``); in mode A the shadow accounts keep running and the instruments of the unselected ones drop out of the parent target and are closed (regardless of ``sell_at_not_selected``).
- **Turnover rate** (``get_adjust_turnover``): on each rebalance day, records the ratio of the traded amount to the total assets before rebalancing.
- **Delisting/suspension**: the parent uniformly force-sells the positions in delisted instruments during the open stage (in mode C the shared account is sold by the parent too, the sub-ledger does not sell twice); the sub-systems automatically skip the suspended bars. A liquidation/reduction leg whose close is suspended falls back to the deferred legs of the next open (any ``trade_on_close`` mode) and the holding stays unchanged until a quote appears.

- **Archive and clone** (`save`/`load`, `clone`): the shadow/quota account registry is saved by the child slot index and rebuilt after loading; the scheduling counters (close-day counter, last driven day), the fixed time axis, the external rebalancing day table and the unfilled calibration legs are restored too, so a loaded system can continue with ``run(reset=False)``; the mode C running pools are rebuilt on the next rebalancing day. Price fields that may carry NaN (e.g. ``goalPrice``) are written as strings, because the text/XML archives cannot read NaN back as a number.

MultiSystem common parameters:

    * **axis_mode="kdata"** *(str)* : the driving time axis mode. "kdata" uses the dates of the input KData of run(kdata); "calendar" uses the fixed date table injected by set_date_axis (the input KData only provides the query/ktype and the price context). An invalid value is rejected at set time
    * **adjust_cycle=1** *(int)* : the rebalancing cycle. In the query/day modes it is the close-day interval; in the week/month/quarter/year modes it is the n-th calendar day within the period. It must be >= 0 (a negative value is rejected at set time); 0 is warned and treated as 1 at readyForRun; a value that can never reach a driven day of the period is warned but not clamped, and that mode then never rebalances
    * **adjust_mode="query"** *(str)* : the rebalancing mode query / day / week / month / quarter / year. A fully illegal value is rejected at set time; the case normalization is done at readyForRun
    * **delay_to_trading_day=True** *(bool)* : whether a non-trading rebalancing day is postponed to the first trading day within the current period (week/month/quarter/year only)
    * **trade_on_close=True** *(bool)* : whether the rebalancing orders execute at the close; when False the whole batch is deferred to the next open (re-priced at the open, sells before buys, suspended legs retried)
    * **sell_at_not_selected=False** *(bool)* : whether the unselected (or zero L1 quota) sub-systems are force-liquidated and their accounts recycled in modes B/C

    The run-time switches above (adjust_cycle / adjust_mode / delay_to_trading_day / trade_on_close / sell_at_not_selected) are snapshotted at readyForRun (the bulk run refreshes them on every run; a direct runMoment* entry requires readyForRun first). The single-instrument trading parameters of :class:`System` (buy_delay / sell_delay / delay_use_current_price / max_delay_count / tp_* / ignore_sell_sg, ...) belong to the sub-systems and are not used by the aggregate parent; the parent's own trace, shared_* and support_borrow_* still take effect. Mode C is the exception — the parent maps trade_on_close onto the sub-system buy_delay / sell_delay.

System common parameters:

    * **buy_delay=True** *(bool)* : Whether buy operations are delayed to be traded at the open of the next bar; the forced short cover triggered by an invalid environment (ev) or condition (cn) follows the same setting
    * **sell_delay=True** *(bool)* : Whether sell operations are delayed to be traded at the open of the next bar; the forced long liquidation triggered by an invalid environment (ev) or condition (cn) follows the same setting (liquidated at the next open instead of at the invalidating close)
    * **delay_use_current_price=True** *(bool)* : For a delayed operation, whether to use the price of the bar at the current trading time to calculate the new stop-loss/take-profit/goal price, or to reuse the result calculated previously
    * **max_delay_count=3** *(int)* : The maximum number of consecutive delayed trade requests; must be greater than or equal to 0, and 0 means only one delay is allowed
    * **tp_monotonic=True** *(bool)* : Whether the take-profit level increases monotonically
    * **tp_delay_n=3** *(int)* : The number of days by which the take-profit is delayed, i.e. the take-profit judgment takes effect starting a given number of days after the actual trade
    * **ignore_sell_sg=False** *(bool)* : Whether to ignore SG sell signals, so that selling happens only through the stop-loss/take-profit and other such means
    * **can_trade_when_high_eq_low=False** *(bool)* : Whether trading is allowed when the highest price equals the lowest price (a single-price limit-up cannot be bought, and a single-price limit-down cannot be sold)

    * **ev_open_position=False** *(bool)*: Whether to apply the market environment to the initial position opening
    * **cn_open_position=False** *(bool)*: Whether to apply the system validity condition to the initial position opening

    * **shared_tm=False** *(bool)*: Whether the tm part is shared
    * **shared_ev=True** *(bool)*: Whether the ev part is shared
    * **shared_cn=False** *(bool)*: Whether the cn part is shared
    * **shared_mm=False** *(bool)*: Whether the mm part is shared
    * **shared_sg=False** *(bool)*: Whether the sg part is shared
    * **shared_st=False** *(bool)*: Whether the st part is shared
    * **shared_tp=False** *(bool)*: Whether the tp part is shared
    * **shared_pg=False** *(bool)*: Whether the pg part is shared
    * **shared_sp=False** *(bool)*: Whether the sp part is shared

    .. note::
        Currently, except for the ev (market environment) part, all other shared parts (tm, cn, mm, sg,
        st, tp, pg, sp) have no internal lock; sharing them across concurrently-run systems would be a
        data race. Therefore, systems running in parallel should not share these parts — each system
        should use its own clone (i.e. keep the corresponding ``shared_*=False``). This is the current
        implementation and may be extended to support thread-safe sharing in the future.

        The ev part carries an internal read-write lock (see "Sharing and Concurrency" in
        :doc:`environment`): ``is_valid`` takes a shared lock once per bar, while setting ``query``
        and ``reset`` take the exclusive lock. All systems sharing one environment must use the same
        query condition, otherwise every differing ``setQuery`` reruns the full calculation under the
        exclusive lock; in hot multi-threaded call paths each system can own a cloned environment
        (``shared_ev=False``).

    .. note::
        The ``auto_checkin`` parameter of the money management strategy takes effect in the trade stage
        of the System: when the account cash is insufficient to buy the quantity indicated by the money
        management strategy, the System deposits the missing amount once, right before the actual buy,
        based on the real trade price and quantity. A delayed buy sizes the quantity twice (the signal-day
        close and the next-day open) but the deposit still happens only once; a cancelled trade deposits
        nothing. The quantity query itself never changes the account.


.. raw:: html

    <table border="1">
        <thead>
            <tr>
                <th>Part naming convention</th>
                <th>Part description</th>
                <th>Part usage</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td>EV_Xxx</td>
                <td>Market environment strategy</td>
                <td>Assesses the market environment; an actual trade occurs only when the market is valid. This part can usually be shared across different system instances to reduce computation.</td>
            </tr>
            <tr>
                <td>CN_Xxx</td>
                <td>System validity condition</td>
                <td>Defines the applicability conditions of the system itself; an actual trade occurs only when the condition is valid.</td>
            </tr>
            <tr>
                <td>SG_Xxx</td>
                <td>Signal generator</td>
                <td>Generates the buy and sell signals.</td>
            </tr>
            <tr>
                <td>ST_Xxx</td>
                <td>Stop-loss/take-profit strategy</td>
                <td>Stop-loss: takes effect only when the trade is at a loss, and is used to terminate the trade<br>Take-profit: takes effect only when the trade is already profitable; whether the level is guaranteed to move only upward is controlled by the common system parameter tp_monotonic</td>
            </tr>
            <tr>
                <td>MM_Xxx</td>
                <td>Money management strategy</td>
                <td>Used to control trading risk, deciding the buy/sell quantity of each trade</td>
            </tr>
            <tr>
                <td>PG_Xxx</td>
                <td>Profit goal strategy</td>
                <td>Exits the trade when its profit goal is reached; it is essentially a special take-profit strategy</td>
            </tr>
            <tr>
                <td>SP_Xxx</td>
                <td>Slippage algorithm</td>
                <td>Used only in backtests; simulates the difference between the planned price and the actual fill price that would occur in live trading</td>
            </tr>
        </tbody>
    </table>
    <p></p>


Create a System and Run the Backtest
------------------------------------

.. py:function:: SYS_Simple([tm=None, mm=None, ev=None, cn=None, sg=None, st=None, tp=None, pg=None, sp=None])

    Create a simple system instance (a trade is not scaled in or out repeatedly: after each buy, the whole position is sold in one go when selling). When the system instance runs (by calling the run method), it requires at least one matching trade manager instance, one money management strategy
    and one signal generator, which can also be specified after the system instance is created. If calling run produces no output and no correct result, tm, sg, or mm may not have been set. To run a backtest, use the run method, e.g.::

        # Create a simulated trading account for the backtest, with an initial capital of 300,000
        my_tm = crtTM(init_cash = 300000)

        # Create the signal generator (the 5-day EMA is the fast line, and the 10-day EMA of that EMA itself is the slow line; buy when the fast line crosses above the slow line, and sell otherwise)
        my_sg = SG_Flex(EMA(CLOSE(), n=5), slow_n=10)

        # Buy a fixed quantity of 1,000 shares each time
        my_mm = MM_FixedCount(1000)

        # Create the system and run it
        sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
        sys.run(sm['sz000001'], Query(-150))

    :param TradeManager tm: the trade manager instance
    :param MoneyManager mm: the money management strategy
    :param EnvironmentBase ev: the environment judgment strategy
    :param ConditionBase cn: the system validity condition
    :param SignalBase sg: the signal generator
    :param StoplossBase st: the stop-loss strategy
    :param StoplossBase tp: the take-profit strategy
    :param ProfitGoalBase pg: the profit goal strategy
    :param SlippageBase sp: the slippage algorithm
    :return: the system instance


System Part Enum Definitions
----------------------------

.. py:class:: System.Part

    The system part enumeration values. The system's buy/sell and other operations can be triggered by these parts, and the enumeration is used to identify the source of an actual trade instruction; see :py:class:`TradeRecord`.

    In practice, the shorthand System.ENVIRONMENT can be used in place of System.Part.ENVIRONMENT, and similarly for the others.

    - System.Part.ENVIRONMENT  - Environment judgment strategy
    - System.Part.CONDITION    - System validity condition
    - System.Part.SIGNAL       - Signal generator
    - System.Part.STOPLOSS     - Stop-loss strategy
    - System.Part.TAKEPROFIT   - Take-profit strategy
    - System.Part.MONEYMANAGER - Money management strategy
    - System.Part.PROFITGOAL   - Profit goal strategy
    - System.Part.SLIPPAGE     - Slippage algorithm
    - System.Part.INVALID      - The invalid-value boundary; any value greater than or equal to this value denotes an invalid part


.. py:function:: get_system_part_name(part)

    Get the string name of a part

        - System.Part.ENVIRONMENT  - "EV"
        - System.Part.CONDITION    - "CN"
        - System.Part.SIGNAL       - "SG"
        - System.Part.STOPLOSS     - "ST"
        - System.Part.TAKEPROFIT   - "TP"
        - System.Part.MONEYMANAGER - "MM"
        - System.Part.PROFITGOAL   - "PG"
        - System.Part.SLIPPAGE     - "SP"
        - System.Part.INVALID      - "--"

    :param int part: the System.Part enumeration value
    :rtype: str


.. py:function:: get_system_part_enum(part_name)

     Get the enumeration value corresponding to the string name of a system part

    :param str part_name: the string name of the system part; see: :py:func:`getSystemPartName`
    :rtype: System.Part



Semantics of Stoploss, Take-profit and Profit Goal
--------------------------------------------------

All three take part in the "close or not" decision while a position is held, but they differ in state ownership and trigger criteria:

* **Stoploss ST** (``setST``) — a **static** stop price. It is calculated once at entry (including an add-on) and stored in the position record; the per-bar checks during the holding period reuse that value and do not follow the market. A sell recalculates it and writes it back, so the remaining position keeps the stop price taken at the sell. Criteria: a long closes when ``current price <= stoploss``, a short when ``current price >= stoploss``.
* **Take-profit TP** (``setTP``) — a **moving** exit level. It shares the base class with ST (both are ``StoplossBase``), but System owns the ratchet: a long only rises and a short only falls (``tp_monotonic``), and an add-on never moves it backwards; the exit also requires the position to be in profit (net of the fees). ``tp_delay_n`` sets how many bars must pass after the entry before the level may trigger; after a partial exit the ratchet resets to the current price. Criteria: a long closes when ``current price <= take-profit level``, a short when ``current price >= take-profit level``.
* **Profit goal PG** (``setPG``) — a pure target price anchored to the **position cost**, recalculated every bar and holding no state across bars. The cost basis is ``buyMoney / totalNumber`` for a long and ``sellMoney / totalNumber`` for a short, not a snapshot of the entry price. Criteria: a long closes when ``current price >= goal``, a short when ``current price <= goal``.

Within one bar the checks run in a fixed order and **only the first matched action is executed** (it returns at once, the remaining checks are not evaluated, and the next bar starts over):

.. code-block:: text

    environment / condition invalidation -> buy signal -> sell signal -> ST -> PG -> TP

Invalid return values are handled differently:

* ST unset or returning an invalid value: the entry still happens, but the position stop price is 0, i.e. no stop-loss; the risk budget then uses the full planned price (the worst case counts as a total loss).
* PG unset: the goal is ``constant.null_price``, the comparison never holds, and that bar has no PG action.
* PG returning ``0``: for a long it means sell at once (the forced-exit sentinel); it has no effect on a short (``current price <= 0`` never holds) — to close a short regardless of price, return the current price instead.
* PG returning a negative or non-finite value: clamped by the base class to ``constant.null_price``, i.e. treated as unset.
* TP returning ``0``: the bar has no take-profit level, so nothing is closed and the ratchet is not updated.

The return-value contract of the stop price is detailed in the "Return Value Contract and System Behaviour" section of :doc:`stoploss`, and the long/short basis of the profit goal in :doc:`profitgoal`.



System Base Class Definition
----------------------------

.. py:class:: System

    The system base class. To extend it or implement more complex systematic trading behavior, inherit from this class.

    .. py:attribute:: name

        The system name

    .. py:attribute:: tm

        The associated trade manager instance

    .. py:attribute:: mm

        The money management strategy

    .. py:attribute:: ev

        The environment judgment strategy

    .. py:attribute:: cn

        The system validity condition

    .. py:attribute:: sg

        The signal generator

    .. py:attribute:: st

        The stop-loss strategy

    .. py:attribute:: tp

        The take-profit strategy

    .. py:attribute:: pg

        The profit goal strategy

    .. py:attribute:: sp

        The slippage algorithm

    .. py:method:: get_param(self, name)

        Get the value of the specified parameter

        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: no such parameter

    .. py:method:: set_param(self, name, value)

        Set the value of a parameter

        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! The parameter type is not supported

    .. py:method:: get_stock(self)

        Get the associated security

        :rtype: Stock

    .. py:method:: get_trade_record_list(self)

        Get the records of trades actually executed; unlike TM, this does not include the trade records caused by rights/dividend adjustments

        :rtype: TradeRecordList

    .. py:method:: get_buy_trade_request_list(self)

        Get the list of buy requests; in "delay" mode, use this to check whether a buy operation will occur on the next bar

        :rtype: list[TradeRequest]

    .. py:method:: get_sell_trade_request_list(self)

        Get the list of sell requests; in "delay" mode, use this to check whether a sell operation will occur on the next bar

        :rtype: list[TradeRequest]

    .. py:function:: run(self, stock, query[, reset=True])

        Run the system and execute the backtest

        :param Stock stock: the security to trade
        :param Query query: the bar data query conditions
        :param bool reset: whether to reset according to the shared attributes of the system parts before execution
        :param bool reset_all: force a reset of all parts

    .. py:method:: reset(self)

        Reset the system, excluding the current instrument and the shared parts

    .. py:method:: force_reset_all(self)

        Force-reset all parts and clear the existing instrument, ignoring the shared attributes of the parts

    .. py:method:: clone(self)

        Clone the system according to the shared attributes of the parts; the shared parts are not actually duplicated and remain shared



Aggregate System Class Definition
---------------------------------

.. py:class:: MultiSystem([name="MultiSystem"])

    The aggregate trading system (portfolio backtesting). It aggregates multiple :class:`System` instances (single-instrument or nested :class:`MultiSystem`) into one portfolio backtested on a single trading account; every sub-system keeps its own SG/MM/EV/CN/ST/TP/PG/SP and only the parent books the trades and places the orders. Arbitrary nesting and circular-reference detection are supported; see the sections above for the three run modes, the nesting rules and the key notes.

    It inherits ``get_param`` / ``set_param`` from :class:`System`; see "MultiSystem common parameters". Overload ``MultiSystem(sys_list, name="MultiSystem")``: construct directly from a sub-system list.

    .. py:attribute:: mode

        The run mode (read-only): "A" signal aggregation / "B" fund allocation / "C" shared-account compatibility

    .. py:method:: add(self, sys)

        Add a sub-system (with the circular-reference detection; adding the same instance twice is ignored)

    .. py:method:: set_mode(self, mode)

        Set the run mode ("A" / "B" / "C", case-insensitive)

    .. py:method:: run(self, kdata[, reset=True, reset_all=False])

        The portfolio backtesting entry: drive all the sub-systems day by day (the open/close stages) along the given KData and book/place the orders uniformly on the parent account.

        Overload ``run(self, query[, reset=True, reset_all=False])``: drive along the trading calendar (equivalent to the legacy ``Portfolio.run(query)``); see the sections above.

Trade Request Records
---------------------

.. py:class:: TradeRequest

    A trade request record. It holds the trade request information registered inside the system when delayed operations are performed. This structure is exposed mainly for use in "delay" mode (where the trade is delayed to the open of the next bar): in this case the system already knows that a trade will happen on the next bar, and :py:meth:`System.getBuyTradeRequestList` and :py:meth:`System.getSellTradeRequestList` can be used to find out whether the next bar needs a buy or a sell. It is mainly used for alerting on or printing the operations needed on the next bar. It has no effect on the running of the system itself.

    .. py:attribute:: valid

        Whether this trade request record is valid (True | False)

    .. py:attribute:: business

        The trade business type; see :py:class:`hikyuu.trade_manage.BUSINESS`

    .. py:attribute:: datetime

        The time at which the trade request was issued

    .. py:attribute:: stoploss

        The stop-loss price at the time the trade request was issued

    .. py:attribute:: part

        The source part of the trade request; see :py:class:`System.Part`

    .. py:attribute:: count

        The number of consecutive delays caused by failed operations
