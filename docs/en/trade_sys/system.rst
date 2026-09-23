.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

System Strategy|SYS
===================

A system is the complete strategy for a single instrument, including the market environment, the system validity condition, the money management, the stop-loss, the take-profit, the profit goal and the slippage; it is used to simulate the backtest.

For multiple objects, a portfolio needs to be used in Hikyuu; see: :ref:`portfolio`.

Common parameters:

    * **buy_delay=True** *(bool)* : Whether the buy operation is delayed to be traded at the open of the next bar
    * **sell_delay=True** *(bool)* : Whether the sell operation is delayed to be traded at the open of the next bar
    * **delay_use_current_price=True** *(bool)* : In the case of a delayed operation, whether to use the price of the bar at the current trade time to calculate the new stop-loss price/take-profit price/goal price, or to use the result calculated last time
    * **max_delay_count=3** *(int)* : The limit of the number of the consecutive delayed trade requests; it should be greater than or equal to 0, and 0 means only 1 delay is allowed
    * **tp_monotonic=True** *(bool)* : The take-profit increases monotonically
    * **tp_delay_n=3** *(int)* : The number of days when the take-profit starts to be delayed, i.e. the take-profit strategy judgment takes effect from several days after the actual trade
    * **ignore_sell_sg=False** *(bool)* : Ignore the sell signal, and sell only by the stop-loss/take-profit and other ways
    * **can_trade_when_high_eq_low=False** *(bool)* : Whether trading is allowed when the highest price equals the lowest price (a limit-up with a single price cannot be bought, and a limit-down with a single price cannot be sold)

    * **ev_open_position=False** *(bool)*: Whether to use the market environment for the initial position opening
    * **cn_open_position=False** *(bool)*: Whether to use the system validity condition for the initial position opening
    
    * **shared_tm=False** *(bool)*: Whether the tm part is a shared part
    * **shared_ev=True** *(bool)*: Whether the ev part is a shared part
    * **shared_cn=False** *(bool)*: Whether the cv part is a shared part    
    * **shared_mm=False** *(bool)*: Whether the mm part is a shared part
    * **shared_sg=False** *(bool)*: Whether the sg part is a shared part
    * **shared_st=False** *(bool)*: Whether the st part is a shared part
    * **shared_tp=False** *(bool)*: Whether the tp part is a shared part
    * **shared_pg=False** *(bool)*: Whether the pg part is a shared part
    * **shared_sp=False** *(bool)*: Whether the sp part is a shared part


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
                <td>Used to judge the market environment; an actual trade happens only when the market is valid. This strategy can usually be shared among different system strategy instances to reduce the amount of calculation.</td>
            </tr>
            <tr>
                <td>CN_Xxx</td>
                <td>System validity condition</td>
                <td>Used to judge the applicability condition of the system itself; an actual trade happens only when the condition is valid.</td>
            </tr>
            <tr>
                <td>SG_Xxx</td>
                <td>Signal generator</td>
                <td>Responsible for generating the buy and sell signals.</td>
            </tr>            
            <tr>
                <td>ST_Xxx</td>
                <td>Stop-loss/take-profit strategy</td>
                <td>Stop-loss: takes effect only when the trade has a loss, used to terminate the trade<br>Take-profit: takes effect only when the trade is already profitable; whether the increase is guaranteed is controlled by the system common parameter tp_monotonic</td>
            </tr>
            <tr>
                <td>MM_Xxx</td>
                <td>Money management strategy</td>
                <td>Used to control the trade risk, deciding the buy/sell number of each trade</td>
            </tr>            
            <tr>
                <td>PG_Xxx</td>
                <td>Profit goal strategy</td>
                <td>Exits the trade when it reaches the profit goal; it is essentially a special take-profit strategy</td>
            </tr>
            <tr>
                <td>SP_Xxx</td>
                <td>Slippage algorithm</td>
                <td>Used only in the backtest, simulating the difference between the planned price and the actual price that occurs in the actual trading during the backtest</td>
            </tr>              
        </tbody>
    </table>
    <p></p>

    
Create a System and Run the Backtest
------------------------------------

.. py:function:: SYS_Simple([tm=None, mm=None, ev=None, cn=None, sg=None, st=None, tp=None, pg=None, sp=None])

    Create a simple system instance (no multiple position increases or decreases in each trade, i.e. everything is sold when selling after each buy). When the system instance runs (calling the run method), it needs at least one matching trade manager instance, one money management strategy
    and one signal generator), which can be specified after the system instance is created. If nothing is output when calling run, and there is no correct result, it may be that tm, sg, mm are not set. For the backtest, use the run method, e.g.::
    
        # Create a simulated trading account for the backtest, with an initial capital of 300,000
        my_tm = crtTM(init_cash = 300000)

        # Create the signal generator (the 5-day EMA is the fast line, and the 10-day EMA of the 5-day EMA itself is the slow line; buy when the fast line crosses the slow line upward, otherwise sell)
        my_sg = SG_Flex(EMA(CLOSE(), n=5), slow_n=10)

        # Buy a fixed number of 1000 shares each time
        my_mm = MM_FixedCount(1000)

        # Create the trade system and run it
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

    The system part enumeration values; the buy/sell and other operations of the system can be triggered by these parts, used to identify the source of the actual trade instruction; see: :py:class:`TradeRecord`.
    
    In actual use, the simplified way of System.ENVIRONMENT can be used instead of System.Part.ENVIRONMENT, and the others are similar.

    - System.Part.ENVIRONMENT  - Market environment strategy
    - System.Part.CONDITION    - System validity condition
    - System.Part.SIGNAL       - Signal generator
    - System.Part.STOPLOSS     - Stop-loss strategy
    - System.Part.TAKEPROFIT   - Take-profit strategy
    - System.Part.MONEYMANAGER - Money management strategy
    - System.Part.PROFITGOAL   - Profit goal strategy
    - System.Part.SLIPPAGE     - Slippage algorithm
    - System.Part.INVALID      - The invalid value boundary; when it is greater than or equal to this value, the part is invalid

    
.. py:function:: get_system_part_name(part)

    Get the string name of the part
    
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

     Get the corresponding enumeration value by the string name of the system part

    :param str part_name: the string name of the system part; see: :py:func:`getSystemPartName`
    :rtype: System.Part


    
System Base Class Definition
----------------------------

.. py:class:: System

    The system base class. To extend or implement more complex system trading behaviors, inherit from this class.
    
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
    
        Get the actually executed trade records; the difference from TM is that it does not include the trade records brought by the rights/adjustment
        
        :rtype: TradeRecordList
        
    .. py:method:: get_buy_trade_request(self)
    
        Get the buy request; in the "delay" mode, check whether there is a buy operation at the next moment
        
        :rtype: TradeRequest

    .. py:method:: get_sell_trade_request(self)
    
        Get the sell request; in the "delay" mode, check whether there is a sell operation at the next moment
        
        :rtype: TradeRequest
                
    .. py:function:: run(self, stock, query[, reset=True])
    
        Run the system and execute the backtest
        
        :param Stock stock: the traded security
        :param Query query: the K-line data query condition
        :param bool reset: whether to reset according to the sharing attributes of the system parts before executing
        :param bool reset_all: force resetting all the parts

    .. py:method:: reset(self)
    
        Reset, but excluding the existing trading object (TO) and the shared parts
        
    .. py:method:: force_reset_all(self)

        Force resetting all the parts and clearing the existing trading object (TO), ignoring the sharing attributes of the parts

    .. py:method:: clone(self)
    
        The clone operation; it clones according to the sharing attributes of the parts, and the shared parts are not actually cloned, keeping them shared
        
        
        
Trade Request Records
---------------------

.. py:class:: TradeRequest

    The trade request record. The trade request information registered inside the system when implementing the delayed operations. The main purpose of exposing this structure is to be used in the "delay" mode (the trade is delayed to the open of the next bar); in this case the system actually knows that a trade will happen in the next bar, and :py:meth:`System.getBuyTradeRequest` and :py:meth:`System.getSellTradeRequest` can be used to know whether the next BAR needs to be bought/sold. It is mainly used for reminding or printing the operations needed in the next bar. It has no effect on the running of the system itself.
    
    .. py:attribute:: valid 
        
        Whether the trade request record is valid (True | False)
    
    .. py:attribute:: business
    
        The trade business type; see: :py:class:`hikyuu.trade_manage.BUSINESS`
    
    .. py:attribute:: datetime
    
        The moment when the trade request was issued
    
    .. py:attribute:: stoploss
    
        The stop-loss price at the moment when the trade request was issued
    
    .. py:attribute:: part
    
        The source of the trade request; see: :py:class:`System.Part`
    
    .. py:attribute:: count
    
        The number of the consecutive delays due to the operation failures
