.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Stop-loss/Take-profit Strategy|ST
=================================

.. Note::

    In Hikyuu, the stop-loss and the take-profit are two separate strategy parts of the trading system. They differ in concept and execution. For example, in a general system, when a following indicator curve is used as the take-profit exit, a lag often occurs: originally it is expected to sell and take the profit when the close price falls below the indicator, but in fact the indicator and the close price both keep falling, so the actual exit happens when the close price crosses the indicator line downward, causing a lagged reaction. In addition, if tracked in real time during the trading session, since the close price keeps changing, the stop-loss indicator line also changes, which causes noise misjudgments, and the results of live trading and backtesting in a general system deviate. In Hikyuu, the stop-loss/take-profit in the current bar is always a fixed value, namely the value of the previous moment; meanwhile, in Hikyuu the system guarantees that the take profit always increases monotonically! For example, for a certain indicator value, the value was 11 the day before yesterday, 9 yesterday, and today's close price is 10; then, as a stop-loss part (today's close price 10 is greater than the stop-loss price 9), it will not trigger an exit, but as a take-profit part, the system will issue a sell instruction, because the current close price is already below 11.
    

Common Stop-loss/Take-profit Strategies
---------------------------------------

The stop-loss means that after buying, the price moves against the expectation, and when the price falls below a certain level, sell to prevent further losses.
The take-profit means that after buying, the price moves as expected, and when the price falls back to a certain level, sell to secure enough profit.
When trading, you can use the same stop-loss and take-profit strategies, or different ones, e.g. a fixed percentage of 3% as the stop-loss and the chandelier safety line as the take-profit.

Fixed Percentage Stop-loss
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: ST_FixedPercent([p=0.03])

    The fixed percentage stop-loss strategy, i.e. stopping the loss when the price falls below a certain percentage of the buy price
    
    :param float p: the percentage (0,1]
    :return: the stop-loss/take-profit strategy instance

Technical Indicator Stop-loss
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: ST_Indicator(op[, kpart="CLOSE"])

    Use a technical indicator as the stop-loss price. E.g. use the 10-day EMA as the stop-loss: ::
    
        ST_Indicator(EMA(CLOSE(), n=10))

    :param Indicator ind: the indicator formula
    :return: the stop-loss/take-profit strategy instance

Alexander Elder Safety Zone Stop-loss
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^    

.. py:function:: ST_Saftyloss([n1=10, n2=3, p=2.0])

    See *Come Into My Trading Room* (2002) by Alexander Elder, p. 202 (Chinese translation published by Seismological Press).
    Calculation description: within the lookback period (generally 10 to 20 days), add up the lengths of all the downward crossings and divide by the number of the downward crossings,
    to get the mean noise (i.e. the length of all the lowest prices below the previous day's lowest price within the lookback period divided by the number), and subtract the previous day's
    mean noise multiplied by a factor from today's lowest price to get the stop-loss line. To offset the fluctuation and guarantee the upward movement of the stop-loss line,
    take the highest value within N days (generally 3 days) based on the above result.

    :param int n1: the lookback time window for calculating the average noise, defaults to 10 days
    :param int n2: take the highest value within n2 days of the preliminary stop-loss line, defaults to 3
    :param double p: the noise coefficient, defaults to 2
    :return: the stop-loss/take-profit strategy instance
    

Custom Stop-loss/Take-profit Strategy
-------------------------------------

The custom stop-loss/take-profit strategy interface:

* :py:meth:`StoplossBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`StoplossBase._clone` - [Required] The clone interface
* :py:meth:`StoplossBase._reset` - [Optional] Reset the internal member variables

Stop-loss/Take-profit Strategy Base Class
-----------------------------------------

.. py:class:: StoplossBase

    The stop-loss/take-profit algorithm base class
    
    .. py:attribute:: name Name
    .. py:attribute:: tm Set or get the trade manager instance
    .. py:attribute:: to Set or get the traded K-line data (TO)
    
    .. py:method:: __init__(self[, name="StoplossBase"])
    
        :param str name: the name
        
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

    .. py:method:: get_price(self, datetime, price)
    
        [Override hook] Get the planned stop-loss price of this expected trade (buy); if there is no stop-loss price, return 0. It is used by the system to query the planned stop-loss price of this trade from the stop-loss strategy module before the trade is executed.
        
        .. note::
            Generally, the stop-loss and take-profit algorithms can be interchanged, but the getPrice of the stop-loss can take the planned trade price, e.g. 30% of the buy price as the stop-loss. The take-profit ignores the passed price parameter, i.e. it assumes price is 0.0. In fact, even for the stop-loss it is not recommended to use the price parameter; e.g. if 30% of the previous day's lowest price can be used as the stop-loss, the price parameter does not need to be considered.
        
        :param Datetime datetime: the trade time
        :param float price: the planned buy price
        :return: the stop-loss price
        :rtype: float
        
    .. py:method:: _calculate(self)
    
        [Override hook] The subclass calculation interface
    
    .. py:method:: _reset(self)
    
        [Override hook] The subclass reset interface, resetting the internal private variables
    
    .. py:method:: _clone(self)
    
        [Override hook] The subclass clone interface
