.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Stop-loss/Take-profit Strategy|ST
=================================

.. Note::

    In Hikyuu, stop-loss and take-profit are implemented as two separate strategy parts of the trading system, differing in both concept and execution. A typical system uses a trailing indicator curve as its take-profit exit, which introduces lag: you intend to sell and lock in profit as soon as the close falls below the indicator, but in a downturn the indicator and the close fall together, so the exit is only signaled when the close crosses below the indicator line. Tracking the curve in real time during the trading session makes the problem worse, because the close keeps moving and drags the stop-loss line with it, producing noise and false signals; as a result, live and backtested results diverge in typical systems. Hikyuu avoids this by keeping the stop-loss and take-profit levels fixed within the current bar—the level is always the value from the previous moment—and by guaranteeing that the take-profit level increases monotonically. For example, suppose an indicator reads 11 two days ago and 9 yesterday, while today's close is 10. Used as a stop-loss, it does not trigger an exit because today's close 10 is above the stop price 9; used as a take-profit, the system issues a sell instruction because the current close 10 is already below 11.
    

Common Stop-loss/Take-profit Strategies
---------------------------------------

A stop-loss protects a position after entry: if the price moves against expectations and falls below a predefined level, the position is sold to prevent further losses.
A take-profit works the other way around: after the price moves in the expected direction, if it pulls back to a predefined level, the position is sold to lock in a sufficient gain.
You may assign the same strategy to both roles or use different ones—for example, a fixed 3% stop-loss together with the chandelier safety line as the take-profit.

Fixed Percentage Stop-loss
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: ST_FixedPercent([p=0.03])

    Fixed-percentage stop-loss: exit once the price falls more than a given percentage below the buy price
    
    :param float p: the percentage (0,1]
    :return: the stop-loss/take-profit strategy instance

Technical Indicator Stop-loss
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: ST_Indicator(op[, kpart="CLOSE"])

    Use a technical indicator as the stop-loss level. For example, use the 10-day EMA::
    
        ST_Indicator(EMA(CLOSE(), n=10))

    :param Indicator ind: the indicator formula
    :return: the stop-loss/take-profit strategy instance

Alexander Elder Safety Zone Stop-loss
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^    

.. py:function:: ST_Saftyloss([n1=10, n2=3, p=2.0])

    See Alexander Elder, *Come Into My Trading Room* (2002), p. 202 (Chinese translation published by Seismological Press).
    Calculation: over the lookback window (typically 10 to 20 days), sum the sizes of all downward penetrations and divide by their count to obtain the average noise. A downward penetration is the amount by which a day's low falls below the previous day's low. The preliminary stop-loss line is today's low minus the previous day's
    average noise multiplied by a coefficient. To dampen fluctuations and ensure that the line only moves upward, take the highest preliminary value over the past n2 days (typically 3 days).

    :param int n1: the lookback window used to calculate the average noise, defaults to 10 days
    :param int n2: number of days over which the highest preliminary stop-loss value is taken, defaults to 3
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

    Base class for stop-loss/take-profit strategies
    
    .. py:attribute:: name The strategy name
    .. py:attribute:: tm Set or get the trade manager instance
    .. py:attribute:: to Set or get the traded KData (trade object, TO)
    
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
    
        [Override hook] Return the planned stop-loss price for an anticipated trade (a buy); return 0 if no stop-loss price applies. The system calls this method before execution to query the strategy for this trade's planned stop-loss level.
        
        .. note::
            In general, stop-loss and take-profit algorithms are interchangeable. The difference is that a stop-loss implementation may use the planned trade price—for example, setting the stop 30% below the buy price—whereas a take-profit ignores the price argument and treats it as 0.0. In practice, even stop-loss implementations are better off ignoring price: a stop based on, say, a level relative to the previous day's low does not need it.
        
        :param Datetime datetime: the trade time
        :param float price: the planned buy price
        :return: the stop-loss price
        :rtype: float
        
    .. py:method:: _calculate(self)
    
        [Override hook] The subclass calculation interface
    
    .. py:method:: _reset(self)
    
        [Override hook] The subclass reset interface; resets the internal private variables
    
    .. py:method:: _clone(self)
    
        [Override hook] The subclass clone interface
