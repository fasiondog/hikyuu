.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _portfolio:

Portfolio|PF
============

In Hikyuu, a portfolio is organized around system strategies as its units; for example, it can combine multiple instruments traded under the same system strategy.

.. note::

   Since ``feature/next``, the former standalone ``Portfolio`` (``SimplePortfolio`` / ``WithoutAFPortfolio``) has been taken over by
   :class:`MultiSystem`: **PF is essentially a preset configuration of MultiSystem**.
   The ``PF_Simple`` / ``PF_WithoutAF`` factories described on this page are **compatibility-layer factories** that support the existing master calling style;
   they **return** :class:`MultiSystem`, rather than a ``Portfolio`` object.
   For new projects, prefer using :class:`MultiSystem` directly (see :doc:`../trade_sys/system`).

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
                <td>Implements the algorithm for evaluating and selecting instruments and system strategies.</td>
            </tr>
            <tr>
                <td>AF_Xxx</td>
                <td>Asset allocation algorithm</td>
                <td>Allocates assets to the systems selected on each cross-section.<br>Since ``feature/next``, AF is a standalone base class, :class:`AllocateFundsBase`, that carries the portfolio-level L1/L2/L3; see :doc:`allocate_funds`.</td>
            </tr>
        </tbody>
    </table>
    <p></p>


Built-in Portfolios
-------------------

.. py:function:: PF_Simple([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True])

    Create a multi-instrument portfolio with a single system strategy (**returns MultiSystem; semantics: mode B — quota allocation + next-period quota write-back**).

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
    :rtype: MultiSystem

.. py:function:: PF_WithoutAF([tm, se, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sys_use_self_tm=False, sell_at_not_selected=False])

    Create a portfolio without a fund allocation algorithm (**returns MultiSystem; semantics: mode A — signal aggregation + uniform ordering by the parent**).

    The rebalancing mode adjust_mode is the same as described above.

    :param TradeManager tm: the trade manager
    :param SelectorBase se: the selector (the instrument selection algorithm)
    :param int adjust_cycle: the rebalancing cycle
    :param str adjust_mode: the rebalancing mode "query" | "day" | "week" | "month" | "quarter" | "year"
    :param bool delay_to_trading_day: if the scheduled day is not a trading day, defer the rebalance to the first trading day within the current cycle
    :param bool trade_on_close: whether the trades are executed at the close
    :param bool sys_use_self_tm: whether the prototype system uses its own tm for the calculation (**has no corresponding semantics in v5; ignored with a warning**)
    :param bool sell_at_not_selected: whether instruments not selected on the rebalancing day are force-sold
    :rtype: MultiSystem


Differences from master and the migration
-----------------------------------------

.. list-table::
    :header-rows: 1

    * - Dimension
      - master
      - v5 (factory pass-through)
    * - Return type
      - ``PortfolioPtr``
      - ``MultiSystemPtr`` (``PortfolioPtr`` is a compatibility alias for ``MultiSystemPtr``, so the existing ``PortfolioPtr pf = PF_Simple(...)`` still compiles)
    * - Class / methods
      - ``Portfolio`` / ``SimplePortfolio`` / ``WithoutAFPortfolio`` and their methods (``run`` / ``getRunningDates`` / ``getCycleEndDates`` / ``lastSuggestion`` ...)
      - No longer exist; use the ``run`` / ``getAdjustDates`` / ``getAdjustTurnover`` / ``toSuggestions`` methods of :class:`MultiSystem`.
        Among them, ``run(query)`` also provides a compatibility overload (equivalent to the master ``Portfolio.run(query)`` and driven by the market trading calendar), so the existing ``pf.run(query)`` calls need no rewrite
    * - Account hierarchy
      - Real TM + shadow TM + sub-system accounts
      - The parent's real TM + the sub-system shadow accounts ``TM_SUB`` (:meth:`MultiSystem.set_sub_init_cash`)
    * - Fund allocation
      - checkout / checkin on the rebalancing day
      - Mode B: the next-period quota write-back (lagging one period behind); mode A: no allocation
    * - Unmapped parameters
      - ``sys_use_self_tm`` takes effect
      - Ignored with ``HKU_WARN``

Migration suggestions:

- Calls using positional arguments to ``PF_Simple(...)`` / ``PF_WithoutAF(...)`` **need no modification** (the return-type alias takes care of them).
- ``pf.run(query)`` **needs no modification**: :meth:`MultiSystem.run` provides the ``query`` compatibility overload,
  driven by the market trading calendar (``StockManager.get_trading_calendar``, SH by default) and semantically equivalent to the master ``Portfolio.run(query)``.
- When a custom driving axis is needed, use the explicit form:
  ``ms.set_axis_mode("calendar")`` + ``ms.set_date_axis(...)`` + ``ms.run(kdata)``.
- Code relying on ``lastSuggestion()`` should switch to ``System.to_suggestions()`` (the field structure is different).
- For new projects, use :class:`MultiSystem` together with :class:`AllocateFundsBase` directly.
