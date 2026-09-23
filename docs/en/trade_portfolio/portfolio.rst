.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

.. _portfolio:

Portfolio|PF
============

In Hikyuu, a Portfolio (PF) is organized with system trading strategies as the units; Hikyuu can use a portfolio of the same trading strategy with different targets, and in the future it can also use a portfolio of system strategies with different targets and different trading logic (requiring tm and the components to support it), which is not exactly the same as the portfolios with multiple targets but the same strategy logic in common programmatic trading.

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
                <td>The multi-factor is essentially scoring the candidates on the cross-section, so it actually needs to be used together with the Selector (strategy selection algorithm).</td>
            </tr>
            <tr>
                <td>SE_Xxx</td>
                <td>Strategy selection algorithm</td>
                <td>Implements the algorithm for evaluating and selecting the targets and the system strategies.<br>Note: the optimization selector in the walk-forward optimization system is also named with the SE prefix, but it is not this one.</td>
            </tr>
            <tr>
                <td>AF_Xxx</td>
                <td>Asset allocation algorithm</td>
                <td>Used to allocate the assets to the systems selected on the cross-section.</td>
            </tr>            
        </tbody>
    </table>
    <p></p>


.. py:function:: PF_Simple([tm, se, af, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True])

    Create a portfolio of multiple targets with a single system strategy

    Description of the position adjustment mode adjust_mode:
    - "query" mode, follows the ktype in the input parameter query; in this case adjust_cycle is the period interval determined by the ktype in the query;
    - "day" mode, adjust_cycle is the position adjustment interval in days;
    - for the "week" | "month" | "quarter" | "year" modes, adjust_cycle

      is the corresponding Nth day of the week, the Nth day of the month, the Nth day of the quarter, or the Nth day of the year; when delay_to_trading_day is false,
      if the day is not a trading day the position adjustment will be skipped; when delay_to_trading_day is true, if the day is not a trading day
      it will be postponed to the first trading day in the current period; e.g. if the position adjustment is specified on the 1st day of each month, but the 1st day of the month is not a trading day, it will be postponed to the first trading day of that month.    

    :param TradeManager tm: the trade management
    :param SelectorBase se: the trading object selection algorithm
    :param AllocateFundsBase af: the asset allocation algorithm
    :param int adjust_cycle: the position adjustment period
    :param str adjust_mode: the position adjustment mode
    :param bool delay_to_trading_day: if the day is not a trading day, it will be postponed to the first trading day in the current period


.. py:function:: PF_WithoutAF([tm, se, adjust_cycle=1, adjust_mode="query", delay_to_trading_day=True, trade_on_close=True, sys_use_self_tm=False,sell_at_not_selected=False])
    
    Create a portfolio without an asset allocation algorithm; all the single-system strategies use the common tm to manage the account

    Description of the position adjustment mode adjust_mode:
    - "query" mode, follows the ktype in the input parameter query; in this case adjust_cycle is the period interval determined by the ktype in the query;
    - "day" mode, adjust_cycle is the position adjustment interval in days;
    - for the "week" | "month" | "quarter" | "year" modes, adjust_cycle
    
      is the corresponding Nth day of the week, the Nth day of the month, the Nth day of the quarter, or the Nth day of the year; when delay_to_trading_day is false,
      if the day is not a trading day the position adjustment will be skipped; when delay_to_trading_day is true, if the day is not a trading day
      it will be postponed to the first trading day in the current period; e.g. if the position adjustment is specified on the 1st day of each month, but the 1st day of the month is not a trading day, it will be postponed to the first trading day of that month.    

    :param TradeManager tm: the trade management
    :param SelectorBase se: the trading object selection algorithm
    :param int adjust_cycle: the position adjustment period
    :param str adjust_mode: the position adjustment mode
    :param bool delay_to_trading_day: if the day is not a trading day, it will be postponed to the first trading day in the current period
    :param bool trade_on_close: whether the trade is executed at the close
    :param bool sys_use_self_tm: the prototype systems use their own tm to calculate
    :param bool sell_at_not_selected: whether the stocks not selected on the position adjustment day are forcibly sold
    
    
Portfolio Class Definition
--------------------------

.. py:class:: Portfolio

    Implements a portfolio of multiple targets and multiple strategies
    
    .. py:attribute:: name  Name
    
    .. py:attribute:: query Running condition

    .. py:attribute:: tm The associated trade manager instance
        
    .. py:attribute:: se The selector strategy
        
    .. py:attribute:: af The asset allocation algorithm

    .. py:attribute:: proto_sys_list The prototype system list

    .. py:attribute:: real_sys_list The actual system list at runtime

    .. py:method:: get_param(self, name)

        Get the specified parameter
        
        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: no such parameter
        
    .. py:method:: set_param(self, name, value)
    
        Set the parameter
        
        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! The parameter type is not supported  

    .. py:method:: reset(self)
    
        The reset operation
    
    .. py:method:: clone(self)
    
        The clone operation

    .. py:method:: run(self, query[, force=false])
    
        Run the portfolio strategy. When the query condition and the components have not changed, the PF will not actually calculate by default when it is executed the second time.
        However, since the parameters of the components may change, whether a recalculation is needed cannot be judged automatically; you can specify a forced calculation manually.

        :param Query query: the query condition
        :param bool force: force recalculating        
