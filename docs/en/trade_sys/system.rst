.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

System Strategy|SYS
===================

A System is a complete trading strategy for a single instrument. It bundles together the market environment, the system validity condition, money management, the stop-loss, the take-profit, the profit goal, and the slippage parts, and is used to run backtests.

To trade multiple instruments, use a portfolio in Hikyuu; see :ref:`portfolio`.

Common parameters:

    * **buy_delay=True** *(bool)* : Whether buy operations are delayed to be traded at the open of the next bar
    * **sell_delay=True** *(bool)* : Whether sell operations are delayed to be traded at the open of the next bar
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

    The system part enumeration values. The system's buy/sell and other operations can be triggered by these parts, and the enumeration is used to identify the source of an actual trade instruction; see :py:class:`TradeRecord`.
    
    In practice, the shorthand System.ENVIRONMENT can be used in place of System.Part.ENVIRONMENT, and similarly for the others.

    - System.Part.ENVIRONMENT  - Market environment strategy
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
        
    .. py:method:: get_buy_trade_request(self)

        Get the pending buy request; in "delay" mode, use this to check whether a buy operation will occur on the next bar

        :rtype: TradeRequest

    .. py:method:: get_sell_trade_request(self)

        Get the pending sell request; in "delay" mode, use this to check whether a sell operation will occur on the next bar

        :rtype: TradeRequest
                
    .. py:function:: run(self, stock, query[, reset=True])
    
        Run the system and execute the backtest
        
        :param Stock stock: the security to trade
        :param Query query: the bar data query conditions
        :param bool reset: whether to reset according to the shared attributes of the system parts before execution
        :param bool reset_all: force a reset of all parts

    .. py:method:: reset(self)
    
        Reset the system, excluding the existing trading object and the shared parts
        
    .. py:method:: force_reset_all(self)

        Force-reset all parts and clear the existing trading object, ignoring the shared attributes of the parts

    .. py:method:: clone(self)
    
        Clone the system according to the shared attributes of the parts; the shared parts are not actually duplicated and remain shared
        
        
        
Trade Request Records
---------------------

.. py:class:: TradeRequest

    A trade request record. It holds the trade request information registered inside the system when delayed operations are performed. This structure is exposed mainly for use in "delay" mode (where the trade is delayed to the open of the next bar): in this case the system already knows that a trade will happen on the next bar, and :py:meth:`System.getBuyTradeRequest` and :py:meth:`System.getSellTradeRequest` can be used to find out whether the next bar needs a buy or a sell. It is mainly used for alerting on or printing the operations needed on the next bar. It has no effect on the running of the system itself.
    
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
