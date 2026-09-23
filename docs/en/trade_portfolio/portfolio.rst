.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _portfolio:

Portfolio|PF
============

In Hikyuu, a Portfolio (PF) is organized around trading systems around trading systems as its core units. Hikyuu supports portfolios that run a single trading strategy across multiple instruments, and it will in the future also support portfolios that combine trading systems differing in both their instruments and their trading logic (provided that the trade manager and the other parts add the required support). This is not exactly the same notion as the portfolio in conventional programmatic trading, where multiple instruments share identical strategy logic.

PF parts:

.. raw:: html

    <table border="1">
        <thead>
            <tr>
                <th>Naming convention</th>
                <th>Component</th>
                <th>Purpose</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td>MF_Xxx</td>
                <td>Multi-factor composition (cross-sectional scoring engine)</td>
                <td>A multi-factor model essentially scores the candidate instruments at each cross-section, so in practice it must be used together with a Selector (strategy selection algorithm).</td>
            </tr>
            <tr>
                <td>SE_Xxx</td>
                <td>System selection algorithm</td>
                <td>Implements the evaluation and selection logic for instruments and trading systems.<br>Note: the optimization selector in the walk-forward optimization system also carries the SE prefix, but it is a different part, not the one described here.</td>
            </tr>
            <tr>
                <td>AF_Xxx</td>
                <td>Fund allocation algorithm</td>
                <td>Allocates funds to the trading systems selected at each cross-section.</td>
            </tr>
        </tbody>
    </table>
    <p></p>


.. py:function:: PF_Simple([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True])

    Create a portfolio that runs a single trading-system strategy across multiple instruments.

    Rebalance mode (``adjust_mode``):

    - ``"query"``: follow the ``ktype`` of the input ``query``; in this mode ``adjust_cycle`` is the interval expressed in that bar period.
    - ``"day"``: ``adjust_cycle`` is the rebalance interval in days.
    - ``"week"`` | ``"month"`` | ``"quarter"`` | ``"year"``: ``adjust_cycle`` specifies the Nth day of the week, month, quarter, or year, respectively. When ``delay_to_trading_day`` is false, a scheduled rebalance date that is not a trading day is skipped. When ``delay_to_trading_day`` is true, the rebalance is postponed to the first trading day within the current period. For example, if the rebalance is scheduled for the 1st of every month and that day is not a trading day, it is moved to the first trading day of that month.

    :param TradeManager tm: trade manager
    :param SelectorBase se: selector that chooses the instruments
    :param AllocateFundsBase af: fund allocation algorithm
    :param int adjust_cycle: rebalance cycle
    :param str adjust_mode: rebalance mode
    :param bool delay_to_trading_day: when the scheduled date is not a trading day, postpone the rebalance to the first trading day within the current period


.. py:function:: PF_WithoutAF([tm, se, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sys_use_self_tm=False,sell_at_not_selected=False])

    Create a portfolio without a fund allocation algorithm; all constituent single-system strategies share one common ``tm`` that manages the account.

    Rebalance mode (``adjust_mode``):

    - ``"query"``: follow the ``ktype`` of the input ``query``; in this mode ``adjust_cycle`` is the interval expressed in that bar period.
    - ``"day"``: ``adjust_cycle`` is the rebalance interval in days.
    - ``"week"`` | ``"month"`` | ``"quarter"`` | ``"year"``: ``adjust_cycle`` specifies the Nth day of the week, month, quarter, or year, respectively. When ``delay_to_trading_day`` is false, a scheduled rebalance date that is not a trading day is skipped. When ``delay_to_trading_day`` is true, the rebalance is postponed to the first trading day within the current period. For example, if the rebalance is scheduled for the 1st of every month and that day is not a trading day, it is moved to the first trading day of that month.

    :param TradeManager tm: trade manager
    :param SelectorBase se: selector that chooses the instruments
    :param int adjust_cycle: rebalance cycle
    :param str adjust_mode: rebalance mode
    :param bool delay_to_trading_day: when the scheduled date is not a trading day, postpone the rebalance to the first trading day within the current period
    :param bool trade_on_close: whether trades are executed at the close
    :param bool sys_use_self_tm: whether each prototype system performs its calculations with its own attached trade manager
    :param bool sell_at_not_selected: whether to force-sell positions in instruments that are not selected on the rebalance day


Portfolio Class Definition
---------------------------

.. py:class:: Portfolio

    Implements a portfolio spanning multiple instruments and multiple strategies.

    .. py:attribute:: name  Portfolio name

    .. py:attribute:: query  Query that defines the run conditions

    .. py:attribute:: tm  Associated trade manager instance

    .. py:attribute:: se  Selector strategy

    .. py:attribute:: af  Fund allocation algorithm

    .. py:attribute:: proto_sys_list  List of prototype systems

    .. py:attribute:: real_sys_list  List of the actual systems instantiated at run time

    .. py:method:: get_param(self, name)

        Return the value of the specified parameter.

        :param str name: parameter name
        :return: parameter value
        :raises out_of_range: raised if no such parameter exists

    .. py:method:: set_param(self, name, value)

        Set the value of a parameter.

        :param str name: parameter name
        :param value: parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! Raised when the parameter value type is not supported.

    .. py:method:: reset(self)

        Reset the portfolio to its initial state.

    .. py:method:: clone(self)

        Return a clone (deep copy) of the portfolio.

    .. py:method:: run(self, query[, force=false])

        Run the portfolio strategy. If the query conditions and all parts are unchanged, a second invocation of the PF does not perform any actual calculation by default. However, the parameters of individual parts may have changed in a way that cannot be detected automatically, so you can pass ``force=True`` to trigger a forced recalculation manually.

        :param Query query: query conditions
        :param bool force: force a full recalculation
