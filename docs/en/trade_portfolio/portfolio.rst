.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _portfolio:

Portfolio|PF
============

In Hikyuu, a portfolio is organized with system trading strategies as the units; it can use a portfolio of the same trading strategy with different targets.

.. note::

   Since ``feature/next``, the original independent ``Portfolio`` (``SimplePortfolio`` / ``WithoutAFPortfolio``) has been taken over by
   :class:`MultiSystem`: **PF is essentially a preset configuration of MultiSystem**.
   The ``PF_Simple`` / ``PF_WithoutAF`` described on this page are the **compatibility layer factories**, used to take over the existing master calling style,
   their **return value is** :class:`MultiSystem`, no longer a ``Portfolio`` object.
   New projects are recommended to use :class:`MultiSystem` directly (see :doc:`../trade_sys/system`).

PF part descriptions:

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
                <td>Multi-factor composition (cross-sectional scoring board)</td>
                <td>The multi-factor is essentially scoring the candidates on the cross-section, so it actually needs to be used together with the Selector (the strategy selection algorithm).</td>
            </tr>
            <tr>
                <td>SE_Xxx</td>
                <td>System selection algorithm</td>
                <td>Implements the algorithm for evaluating and selecting the targets and the system strategies.</td>
            </tr>
            <tr>
                <td>AF_Xxx</td>
                <td>Asset allocation algorithm</td>
                <td>Used to allocate the assets to the systems selected on the cross-section.<br>Since ``feature/next``, AF is an independent base class :class:`AllocateFundsBase`, carrying the portfolio-level L1/L2/L3, see :doc:`allocate_funds`.</td>
            </tr>
        </tbody>
    </table>
    <p></p>


Built-in Portfolios
-------------------

.. py:function:: PF_Simple([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True])

    Create a portfolio of multiple targets with a single system strategy (**returns MultiSystem, the semantics is mode B: quota allocation + the next-period quota write-back**).

    Description of the rebalancing mode adjust_mode:
    - In the "query" mode, it follows the ktype in the input parameter query, at this time adjust_cycle determines the cycle interval by the ktype in query;
    - In the "day" mode, adjust_cycle is the rebalancing interval days;
    - In the "week" | "month" | "quarter" | "year" mode, adjust_cycle is the corresponding N-th day of every week, the n-th day of every month, the n-th day of every quarter and the n-th day of every year; when delay_to_trading_day is false and that day is not a trading day, the rebalancing is skipped; when delay_to_trading_day is true and that day is not a trading day, it is postponed to the first trading day within the current cycle, e.g. if the rebalancing is specified on the 1st day of every month but the 1st of that month is not a trading day, it is postponed to the first trading day of that month.

    :param TradeManager tm: the trade management
    :param SelectorBase se: the trading object selection algorithm
    :param AllocateFundsBase af: the fund allocation algorithm (AF, carrying L1/L2/L3, see :doc:`allocate_funds`)
    :param int adjust_cycle: the rebalancing cycle
    :param str adjust_mode: the rebalancing mode "query" | "day" | "week" | "month" | "quarter" | "year"
    :param bool delay_to_trading_day: when that day is not a trading day, it is postponed to the first trading day within the current cycle
    :rtype: MultiSystem

.. py:function:: PF_WithoutAF([tm, se, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sys_use_self_tm=False, sell_at_not_selected=False])

    Create a portfolio without a fund allocation algorithm (**returns MultiSystem, the semantics is mode A: signal aggregation + the parent uniform ordering**).

    The description of the rebalancing mode adjust_mode is the same as above.

    :param TradeManager tm: the trade management
    :param SelectorBase se: the trading object selection algorithm
    :param int adjust_cycle: the rebalancing cycle
    :param str adjust_mode: the rebalancing mode "query" | "day" | "week" | "month" | "quarter" | "year"
    :param bool delay_to_trading_day: when that day is not a trading day, it is postponed to the first trading day within the current cycle
    :param bool trade_on_close: whether the trade is executed at the close
    :param bool sys_use_self_tm: the prototype system uses its own tm for the calculation (**no corresponding semantics in v5, ignored with a warning**)
    :param bool sell_at_not_selected: whether the targets not selected on the rebalancing day are force sold
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
      - ``MultiSystemPtr`` (the compatibility alias ``PortfolioPtr`` points to ``MultiSystemPtr``, the existing ``PortfolioPtr pf = PF_Simple(...)`` keeps compiling)
    * - Class / methods
      - ``Portfolio`` / ``SimplePortfolio`` / ``WithoutAFPortfolio`` and their methods (``run`` / ``getRunningDates`` / ``getCycleEndDates`` / ``lastSuggestion`` ...)
      - No longer exist; use the ``run`` / ``getAdjustDates`` / ``getAdjustTurnover`` / ``toSuggestions`` of :class:`MultiSystem`.
        Among them ``run(query)`` also provides a compatibility overload (equivalent to the master ``Portfolio.run(query)``, with the market trading calendar as the driving axis), the existing ``pf.run(query)`` needs no rewrite
    * - Account hierarchy
      - Real TM + shadow TM + sub-system accounts
      - The parent real TM + the sub-system shadow accounts ``TM_SUB`` (:meth:`MultiSystem.set_sub_init_cash`)
    * - Fund allocation
      - checkout / checkin on the rebalancing day
      - Mode B: the next-period quota write-back (lagging one period behind); mode A: no allocation
    * - Unmapped parameters
      - ``sys_use_self_tm`` takes effect
      - Ignored with ``HKU_WARN``

Migration suggestions:

- The positional argument calls of ``PF_Simple(...)`` / ``PF_WithoutAF(...)`` **need no modification** (the return type alias can take over).
- ``pf.run(query)`` **needs no modification**: :meth:`MultiSystem.run` provides the ``query`` compatibility overload,
  with the market trading calendar (``StockManager.get_trading_calendar``, SH by default) as the driving axis, semantically equivalent to the master ``Portfolio.run(query)``.
- When a custom driving axis is needed, use the explicit writing:
  ``ms.set_axis_mode("calendar")`` + ``ms.set_date_axis(...)`` + ``ms.run(kdata)``.
- The code depending on ``lastSuggestion()`` changes to ``System.to_suggestions()`` (the field structure is different).
- New projects directly use :class:`MultiSystem` + :class:`AllocateFundsBase`.
