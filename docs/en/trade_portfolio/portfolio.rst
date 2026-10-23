.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _portfolio:

Portfolio|PF
============

In Hikyuu, a portfolio is organized around system strategies as its units; for example, it can combine multiple instruments traded under the same system strategy.

.. note::

   ``SimplePortfolio`` / ``WithoutAFPortfolio`` map to :class:`MultiSystem`: **a PF is essentially a
   preset configuration of MultiSystem**.
   The ``PF_Simple`` / ``PF_WithoutAF`` / ``PF_SignalAggregate`` factories described on this page are
   **preset factories**; they **return** :class:`MultiSystem`. Projects may also use
   :class:`MultiSystem` directly (see :doc:`../trade_sys/system`).

   The execution paradigm of a portfolio is decided by the **running mode** owned by the
   :class:`MultiSystem` (set via ``set_mode``), and the three factories map onto the three modes:

   The PF factories only set the **top-level** mode; a sub-system may nest a :class:`MultiSystem`
   of any mode. See "Nested and mixed modes" in :doc:`../trade_sys/system` for the account
   topology, the liquidation semantics and the rebalancing cadence under nesting.

   .. list-table::
      :header-rows: 1

      * - Factory
        - Mode
        - Positioning
      * - ``PF_Simple``
        - B Fund Allocation
        - quota split, autonomous sub-manager, the parent mirrors the orders (FOF/MOM)
      * - ``PF_SignalAggregate``
        - A Signal Aggregation
        - the sub-systems are pure signal sources, the parent converts and orders uniformly
      * - ``PF_WithoutAF``
        - C Shared Account
        - the sub-systems trade on the real account of the parent (the legacy Portfolio behavior)

   Every factory also has a letter-named thin alias (``PF_ModeA`` / ``PF_ModeB`` / ``PF_ModeC``): the
   same implementation, the same parameters and no behavior difference, for lookups and mode comparisons.

   .. list-table:: Defaults of the three modes
      :header-rows: 1

      * - Factory (mode)
        - ``trade_on_close``
        - ``sell_at_not_selected``
      * - ``PF_SignalAggregate`` (A Signal Aggregation)
        - configurable, ``True`` by default
        - configurable, ``False`` by default
      * - ``PF_Simple`` (B Fund Allocation)
        - configurable, ``True`` by default
        - configurable, ``True`` by default (aligned with the legacy ``SimplePortfolio``)
      * - ``PF_WithoutAF`` (C Shared Account Compatibility)
        - configurable, ``True`` by default
        - configurable, ``False`` by default (aligned with the legacy engine)

   Note that the default of ``sell_at_not_selected`` **differs** between the three modes (only mode B
   defaults to ``True``), so switching the mode switches the liquidation behavior; confirm it
   explicitly when migrating.

   There is also an executor switch ``auto_checkin`` (default ``False``): when the parent cash cannot
   fill this batch of suggestions it deposits extra principal to cover the gap computed from the money
   plus the commission (it only keeps the planned quantities tradable, at the cost of an inflated
   backtest equity curve, and it may top up on every bar whose targets change). It only applies to the
   parent executor of modes A/B: mode C lets the sub-systems trade on the shared account, so it has no
   effect there; in mode B the quota injection is already limited by the parent cash before this step,
   so the switch cannot recover a truncated quota. The financing switches ``support_borrow_cash`` /
   ``support_borrow_stock`` must be set on the ``MultiSystem`` and are propagated to the parent TM by
   ``readyForRun``; with financing enabled the executor derives the buying power from the margin rate
   **per order** (a multi-leg batch is funded in order, so later legs may be shrunk). The switch must be
   set before ``readyForRun``; a change during ``run`` takes effect at the next preparation.

   .. warning::

      The name ``PF_WithoutAF`` literally means "without a fund allocation algorithm", which is
      **mode C** (the behavior of the ``Portfolio`` before the refactoring). During the refactoring it
      was briefly mapped onto mode A; it is now restored to mode C, so the existing strategies need
      **no change at all** to reproduce their pre-refactoring results. Code that used ``PF_WithoutAF``
      with the mode A semantics inside that window should switch to ``PF_SignalAggregate``.

The PF parts:

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
                <td>MF_Xxx</td>
                <td>Multi-factor composition (cross-sectional scoring and ranking)</td>
                <td>A multi-factor model essentially scores the candidate instruments on each cross-section, so in practice it must be used together with a Selector (the strategy selection algorithm).</td>
            </tr>
            <tr>
                <td>SE_Xxx</td>
                <td>System selection algorithm</td>
                <td>Implements the algorithm for evaluating and selecting instruments and system strategies.<br>Note: the optimization selector in the walk-forward optimization system also carries the SE prefix, but it is a different part, not the one described here.</td>
            </tr>
            <tr>
                <td>AF_Xxx</td>
                <td>Asset allocation algorithm</td>
                <td>Allocates assets to the systems selected on each cross-section.<br>AF is a standalone base class, :class:`AllocateFundsBase`, that carries the portfolio-level L1/L2/L3; see :doc:`allocate_funds`.</td>
            </tr>
        </tbody>
    </table>
    <p></p>


Built-in Portfolios
-------------------

.. py:function:: PF_Simple([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sell_at_not_selected=True])

    Create a multi-instrument portfolio with a single system strategy (**returns MultiSystem running in mode B "Fund Allocation"**).

    Mode B semantics (quota allocation, FOF/MOM style): on every rebalancing day the AF allocates the
    quota to each selected sub-system (L1); each selected sub-system is calibrated to its quota BEFORE
    it is driven (zero-quota liquidation, then the unselected liquidation or the quota cash recycle, then reduce the over-quota part, inject the gap) and trades with
    the exact quota; the parent mirrors the real instructions of the sub-systems (L2 pass-through) on
    its own account; the L3 portfolio risk control is skipped (the sub-manager autonomy is respected).
    ``PF_Simple`` defaults to the legacy ``SimplePortfolio``: ``trade_on_close=True`` and
    ``sell_at_not_selected=True``, so the unselected ones are cleared together with the parent
    holdings on the rebalancing day; set ``sell_at_not_selected=False`` to keep them running (they
    only skip the quota calibration). The sub-system shadow accounts start from zero and follow the
    cost function of the parent account.

    The quota cash is a **virtual budget**: ``checkin/checkout`` only touch the sub ledger, the parent
    account is neither debited nor credited by them; the parent real cash only changes with the
    mirrored trades. The injections of one rebalancing day are cumulatively capped by the parent's
    remaining cash, so the mirrored buys are always fundable and the two books (positions / trades)
    stay matched; an over-allocation (e.g. ``fixed_amount`` beyond the parent cash) is truncated at
    the injection with a one-time warning instead of creating hidden leverage. The cash of the two
    ledgers is **not** subject to a conservation constraint (the quota cash is not reconciled against
    the parent cash); a hard per-sub budget / auditable commitment would be a separate "fund
    commitment ledger" design.

    The rebalancing mode adjust_mode:
    - In "query" mode, rebalancing follows the ktype of the input query, and adjust_cycle determines the interval in units of that ktype;
    - In "day" mode, adjust_cycle is the number of days between rebalances;
    - In "week" | "month" | "quarter" | "year" mode, adjust_cycle specifies the N-th day of the corresponding week, month, quarter, or year; when delay_to_trading_day is false and that day is not a trading day, the rebalance is skipped; when delay_to_trading_day is true and that day is not a trading day, it is deferred to the first trading day within the current cycle — e.g. if rebalancing is scheduled for the 1st of every month and the 1st of that month is not a trading day, it is deferred to the first trading day of that month.

    :param TradeManager tm: the trade manager
    :param SelectorBase se: the selector (the instrument selection algorithm)
    :param AllocateFundsBase af: the fund allocation algorithm (AF; carries L1/L2/L3, see :doc:`allocate_funds`)
    :param int adjust_cycle: the rebalancing cycle
    :param str adjust_mode: the rebalancing mode "query" | "day" | "week" | "month" | "quarter" | "year"
    :param bool delay_to_trading_day: if the scheduled day is not a trading day, defer the rebalance to the first trading day within the current cycle
    :param bool trade_on_close: whether the trades are executed at the close
    :param bool sell_at_not_selected: whether instruments not selected on the rebalancing day are force-sold, ``True`` by default to match the legacy ``SimplePortfolio``
    :rtype: MultiSystem

.. py:function:: PF_WithoutAF([tm, se, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sys_use_self_tm=False, sell_at_not_selected=False])

    Create a portfolio without a fund allocation algorithm (**returns MultiSystem running in mode C
    "Shared Account Compatibility"**), i.e. the behavior of the ``WithoutAFPortfolio`` from before the
    refactoring, used to reproduce the previous results.

    Mode C semantics (shared account): no shadow account is created, every sub-system trades **directly
    on the real account of the parent** (``shared_tm``) and is sized by its own MM, so the sub-systems
    compete for the same cash (in the pool admission order). The parent only gates the entry with the SE
    and keeps the pool driven; it performs **no L2 conversion and never orders again**, and the mode is
    held by the aggregate system itself -- the AF only provides the L1/L2/L3 no-ops here (L1 takes no
    quota, L2 needs no conversion, L3 is off by default).

    The driven set follows the SE semantics (the SE is freely replaceable): a sub-system admitted by the
    SE on the rebalancing day enters the running pool, and it **leaves as soon as it is not selected
    again** (no matter whether it still holds). For a leaving system that still holds: with
    ``sell_at_not_selected=True`` the parent liquidates it at once, while with the default ``False``
    (aligned with the legacy engine) it gets one last drive on that day and is then handed to the
    ``force_sell`` pool, which is only driven again on the non-rebalancing days and only when
    ``sell_at_not_selected`` is on. Under the default the residual position is therefore no longer
    followed (the established behavior of the legacy engine).

    The rebalancing mode adjust_mode is the same as described above.

    :param TradeManager tm: the trade manager
    :param SelectorBase se: the selector (the instrument selection algorithm)
    :param int adjust_cycle: the rebalancing cycle
    :param str adjust_mode: the rebalancing mode "query" | "day" | "week" | "month" | "quarter" | "year"
    :param bool delay_to_trading_day: if the scheduled day is not a trading day, defer the rebalance to the first trading day within the current cycle
    :param bool trade_on_close: whether the trades are executed at the close (also mapped onto the sub-system buy_delay / sell_delay)
    :param bool sys_use_self_tm: kept for the signature compatibility only (mode C always shares the real account of the parent; **ignored with a warning**)
    :param bool sell_at_not_selected: whether instruments not selected on the rebalancing day are force-sold, ``False`` by default to match the legacy engine
    :rtype: MultiSystem

.. py:function:: PF_SignalAggregate([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sell_at_not_selected=False, sub_init_cash=100000.0])

    Create a portfolio running in **mode A "Signal Aggregation"** (returns MultiSystem).

    Mode A semantics (signal aggregation): the sub-systems are pure signal sources on their shadow
    accounts (the signal cash is never reset, the shadows run continuously); the parent converts the
    suggestions into the target positions of the parent account by the AF L2 (target market value =
    weight x net ratio x the parent net assets, aggregated per instrument and rebalanced by the delta
    against the current position) and the L3 portfolio risk control applies (e.g. max_single_position).
    The unselected sub-systems are excluded from the parent target sampling, so their instruments drop out of the target and are closed; ``sell_at_not_selected`` does not take part in that behavior in mode A (the parent closes its own holdings and never touches the shadow accounts).

    This is the preset that ``PF_WithoutAF`` carried at the beginning of the refactoring; since
    ``PF_WithoutAF`` now stands for mode C, the mode A preset is named after what it does.

    The rebalancing mode adjust_mode is the same as described above.

    :param TradeManager tm: the trade manager
    :param SelectorBase se: the selector (the instrument selection algorithm)
    :param AllocateFundsBase af: the fund allocation algorithm (AF, carrying L1/L2/L3, equal weight by default, see :doc:`allocate_funds`)
    :param int adjust_cycle: the rebalancing cycle
    :param str adjust_mode: the rebalancing mode "query" | "day" | "week" | "month" | "quarter" | "year"
    :param bool delay_to_trading_day: if the scheduled day is not a trading day, defer the rebalance to the first trading day within the current cycle
    :param bool trade_on_close: whether the trades are executed at the close
    :param bool sell_at_not_selected: whether instruments not selected on the rebalancing day are force-sold (ignored in mode A, see above)
    :param float sub_init_cash: the signal cash of every sub-system shadow account (never reset)
    :rtype: MultiSystem
