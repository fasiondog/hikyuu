.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

System Strategy|SYS
===================

A system is a complete strategy for a single instrument. It bundles together the market environment, the system validity condition, money management, stop-loss, take-profit, profit goal, and slippage parts, and is used to run backtests.

To trade multiple instruments, use a portfolio in Hikyuu; see :ref:`portfolio`.

Common parameters:

    * **buy_delay=True** *(bool)* : Whether buy orders are delayed and executed at the open of the next bar
    * **sell_delay=True** *(bool)* : Whether sell orders are delayed and executed at the open of the next bar
    * **delay_use_current_price=True** *(bool)* : When an order is delayed, whether to compute the new stop-loss, take-profit, and profit-goal prices from the current bar's price, or reuse the values calculated on the previous bar
    * **max_delay_count=3** *(int)* : Maximum number of consecutive times a trade request may be delayed. Must be greater than or equal to 0; a value of 0 allows only one delay
    * **tp_monotonic=True** *(bool)* : Whether the take-profit level is guaranteed to rise monotonically
    * **tp_delay_n=3** *(int)* : Number of days after the actual trade before the take-profit rule begins to take effect
    * **ignore_sell_sg=False** *(bool)* : Whether to ignore sell signals from the signal generator and exit only via stop-loss, take-profit, and other exits
    * **can_trade_when_high_eq_low=False** *(bool)* : Whether trading is allowed when the bar's high equals its low (a single-price limit-up bar cannot be bought, and a single-price limit-down bar cannot be sold)

    * **ev_open_position=False** *(bool)*: Whether to apply the market environment when opening the initial position
    * **cn_open_position=False** *(bool)*: Whether to apply the system validity condition when opening the initial position
    
    * **shared_tm=False** *(bool)*: Whether the tm part is shared across systems
    * **shared_ev=True** *(bool)*: Whether the ev part is shared across systems
    * **shared_cn=False** *(bool)*: Whether the cn part is shared across systems    
    * **shared_mm=False** *(bool)*: Whether the mm part is shared across systems
    * **shared_sg=False** *(bool)*: Whether the sg part is shared across systems
    * **shared_st=False** *(bool)*: Whether the st part is shared across systems
    * **shared_tp=False** *(bool)*: Whether the tp part is shared across systems
    * **shared_pg=False** *(bool)*: Whether the pg part is shared across systems
    * **shared_sp=False** *(bool)*: Whether the sp part is shared across systems


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
                <td>Determines whether the market environment is tradable; a trade is actually executed only when the market is valid. This part can usually be shared across different system instances to reduce computation.</td>
            </tr>
            <tr>
                <td>CN_Xxx</td>
                <td>System validity condition</td>
                <td>Determines whether the system itself is applicable; a trade is actually executed only when this condition holds.</td>
            </tr>
            <tr>
                <td>SG_Xxx</td>
                <td>Signal generator</td>
                <td>Generates buy and sell signals.</td>
            </tr>            
            <tr>
                <td>ST_Xxx</td>
                <td>Stop-loss/take-profit strategy</td>
                <td>Stop-loss: triggers only when the position is at a loss, to terminate the trade<br>Take-profit: triggers only when the position is already profitable; the tp_monotonic system parameter controls whether the level is guaranteed to rise monotonically</td>
            </tr>
            <tr>
                <td>MM_Xxx</td>
                <td>Money management strategy</td>
                <td>Controls trade risk by deciding the buy or sell quantity for each trade</td>
            </tr>            
            <tr>
                <td>PG_Xxx</td>
                <td>Profit goal strategy</td>
                <td>Exits the trade once its profit goal is reached; essentially a specialized take-profit strategy</td>
            </tr>
            <tr>
                <td>SP_Xxx</td>
                <td>Slippage algorithm</td>
                <td>Used only in backtests to model the gap between the intended fill price and the price at which the trade would actually be executed</td>
            </tr>              
        </tbody>
    </table>
    <p></p>

    
Create a System and Run the Backtest
------------------------------------

.. py:function:: SYS_Simple([tm=None, mm=None, ev=None, cn=None, sg=None, st=None, tp=None, pg=None, sp=None])

    Create a simple system instance. Each trade is a single in-and-out position: the system never scales into or out of a position, so every buy is later closed by selling the entire holding. To run the instance (by calling the run method), it needs at least a matching trade manager, one money management strategy,
    and one signal generator; these can be assigned after the instance is created. If calling run produces no output and no results, tm, sg, or mm is probably not set. Run a backtest with the run method, for example::
    
        # Create a simulated trading account for the backtest, with 300,000 in initial capital
        my_tm = crtTM(init_cash = 300000)

        # Create the signal generator (the 5-day EMA is the fast line, and its own 10-day EMA is the slow line; buy when the fast line crosses above the slow line, and sell when it crosses below)
        my_sg = SG_Flex(EMA(CLOSE(), n=5), slow_n=10)

        # Buy a fixed 1,000 shares on every entry
        my_mm = MM_FixedCount(1000)

        # Create the trading system and run it
        sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
        sys.run(sm['sz000001'], Query(-150))
    
    :param TradeManager tm: the trade manager instance 
    :param MoneyManager mm: the money management strategy
    :param EnvironmentBase ev: the market environment strategy
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

    Enumerates the system parts. Buy/sell operations and other system actions can be triggered by any of these parts, and the enum identifies the source of an actual trade instruction; see :py:class:`TradeRecord`.
    
    In practice, you can use the shorthand System.ENVIRONMENT instead of System.Part.ENVIRONMENT; the other members work the same way.

    - System.Part.ENVIRONMENT  - Market environment strategy
    - System.Part.CONDITION    - System validity condition
    - System.Part.SIGNAL       - Signal generator
    - System.Part.STOPLOSS     - Stop-loss strategy
    - System.Part.TAKEPROFIT   - Take-profit strategy
    - System.Part.MONEYMANAGER - Money management strategy
    - System.Part.PROFITGOAL   - Profit goal strategy
    - System.Part.SLIPPAGE     - Slippage algorithm
    - System.Part.INVALID      - Boundary of the valid value range; any value greater than or equal to this one denotes an invalid part

    
.. py:function:: get_system_part_name(part)

    Get the short string name of a system part
    
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

     Look up the enumeration value corresponding to a system part's string name

    :param str part_name: the string name of the system part; see: :py:func:`getSystemPartName`
    :rtype: System.Part


    
System Base Class Definition
----------------------------

.. py:class:: System

    Base class for trading systems. Inherit from this class to extend a system or implement more complex systematic trading behavior.
    
    .. py:attribute:: name  
    
        The system name
    
    .. py:attribute:: tm  
    
        The associated trade manager instance
        
    .. py:attribute:: mm  
    
        The money management strategy
        
    .. py:attribute:: ev  
    
        The market environment strategy
        
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
                
    .. py:method:: get_stock(self)
    
        Get the associated security
        
        :rtype: Stock
        
    .. py:method:: get_trade_record_list(self)
    
        Return the records of trades that were actually executed. Unlike the trade manager's records, these do not include records introduced by equity/dividend adjustments
        
        :rtype: TradeRecordList
        
    .. py:method:: get_buy_trade_request(self)
    
        Return the pending buy request. In delay mode, use this to check whether a buy will occur on the next bar
        
        :rtype: TradeRequest

    .. py:method:: get_sell_trade_request(self)
    
        Return the pending sell request. In delay mode, use this to check whether a sell will occur on the next bar
        
        :rtype: TradeRequest
                
    .. py:function:: run(self, stock, query[, reset=True])
    
        Run the system over the given data to execute the backtest
        
        :param Stock stock: the security to trade
        :param Query query: the query defining the KData (candlestick/bar data) range
        :param bool reset: whether to reset parts according to their shared flags before running
        :param bool reset_all: whether to force-reset every part

    .. py:method:: reset(self)
    
        Reset the system without clearing its existing trade object (TO) or the shared parts
        
    .. py:method:: force_reset_all(self)

        Force-reset every part and clear the existing trade object (TO), ignoring each part's shared flag

    .. py:method:: clone(self)
    
        Clone the system according to the parts' shared flags: shared parts are not deep-copied and remain shared
        
        
        
Trade Request Records
---------------------

.. py:class:: TradeRequest

    Records a pending trade request. When the system performs a delayed operation (execution is postponed to the open of the next bar), it registers the request information internally. This structure is exposed mainly so that, in delay mode, you can know in advance that a trade will occur on the next bar: :py:meth:`System.getBuyTradeRequest` and :py:meth:`System.getSellTradeRequest` tell you whether the next bar will trigger a buy or a sell. It is intended for alerts or logging of the upcoming action and has no effect on how the system itself runs.
    
    .. py:attribute:: valid 
        
        Whether this trade request is valid (True | False)
    
    .. py:attribute:: business
    
        The trade business type; see :py:class:`hikyuu.trade_manage.BUSINESS`
    
    .. py:attribute:: datetime
    
        The time at which the trade request was issued
    
    .. py:attribute:: stoploss
    
        The stop-loss price in effect when the trade request was issued
    
    .. py:attribute:: part
    
        The part that issued the trade request; see :py:class:`System.Part`
    
    .. py:attribute:: count
    
        The number of consecutive delays caused by failed order execution
