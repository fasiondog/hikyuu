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
    
    .. note::
        When the stop-loss cannot be represented by the price tick (the price is too low, e.g. ``price=0.01, p=0.5`` rounds to the 0.01 tick and then clamps to 0, or ``price=0.006, p=0.03`` turns negative after the one-tick clamp), the level is invalid and the trade still executes **without stop-loss protection**. ``p=1.0`` means no stop-loss (the level can never be reached).
    
    :param float p: the percentage (0,1]
    :return: the stop-loss/take-profit strategy instance

Technical Indicator Stop-loss
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: ST_Indicator(ind)

    Use a technical indicator as the stop-loss level. For example, use the 10-day EMA::
    
        ST_Indicator(EMA(CLOSE(), n=10))

    .. note::
        During the indicator's warm-up (the first N bars) or on a bar without a valid value (NaN/inf), the level is invalid: used as a stop-loss the trade still executes but has **no stop-loss protection** (the position has no static stop-loss for its whole life, so pair it with a take-profit or a sell signal), used as a take-profit there is no take-profit on that bar.

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
    :param double p: the noise coefficient, defaults to 2; it must be finite and > 0, otherwise the creation throws (p <= 0 puts the stop-loss line above the price, a NaN p filters out every bar and silently disables the stop-loss)
    :return: the stop-loss/take-profit strategy instance
    

Custom Stop-loss/Take-profit Strategy
-------------------------------------

The custom stop-loss/take-profit strategy interface:

* :py:meth:`StoplossBase._get_price` - [Required] Compute the raw long stop-loss price
* :py:meth:`StoplossBase._get_short_price` - [Required] Compute the raw short stop-loss price (default returns 0; a short stop-loss must be above the planned trade price)
* :py:meth:`StoplossBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`StoplossBase._clone` - [Required] The clone interface
* :py:meth:`StoplossBase._reset` - [Optional] Reset the internal member variables

Return Value Contract and System Behaviour
------------------------------------------

How often ``_get_price`` / ``_get_short_price`` run depends on the role the instance is used in:
**as a stop-loss they run on the entry path only** (about once or twice per trade; once the level is
fixed it is stored in the position record and never queried again while the position is held), while
**as a take-profit they run once per bar while the position is held**, because the take-profit level
is trailing. The meaning of the return value and the resulting system behaviour are:

* **> 0**: a valid stop-loss / take-profit level. It must use the same (adjusted) price coordinate
  as the KData passed to ``setTO``; the system maps it back to the original price coordinate
  internally.
* **0**: no stop-loss on this bar (explicitly declared by the strategy). The trade still takes
  place, but without stop-loss protection (the position is sized with the full planned price as
  the risk).
* **negative or non-finite (NaN/inf)**: invalid, which is equivalent to "no stop-loss". The trade
  still takes place (a long buy or a short sell) and the position is sized with the full planned
  price as the risk (a risk-based money manager shrinks the size automatically). Note that the
  stop-loss level is fixed at entry and never updated while the position is held, so such a
  position has no static stop-loss protection for its whole life; it must be exited by the
  take-profit (TP), the profit goal or a sell signal.

In addition, the system rejects two meaningless levels:

* **Long**: when the stop-loss is not lower than the planned buy price, the buy is abandoned (the
  strategy cannot provide effective protection).
* **Short**: when the stop-loss is not above the planned trade price (including equal), the short
  entry is abandoned (otherwise the cover would trigger on the very next bar).

Indicator-based stop-losses (``ST_Indicator`` / ``ST_Saftyloss``) follow the same contract: during
the indicator's warm-up, or on a bar without a valid value (NaN/inf), the level is invalid — used as
a stop-loss the trade still takes place but without stop-loss, used as a take-profit there is simply
no take-profit on that bar.

Both "opening without stop-loss" and "abandoning a meaningless level" are normal handling and do not
emit warnings; set the system parameter ``trace`` to True to see the reason.

**Difference when used as a take-profit**: the take-profit reuses the same base class and contract,
but 0 or an invalid level only means "no take-profit on this bar"; the trade is not skipped.

**Stop-loss is static, take-profit is trailing**: the stop-loss level is fixed at entry and stored in
the position record; it is **not updated with the market** while the position is held (this also
applies to ``ST_Indicator`` / ``ST_Saftyloss`` used as a stop-loss). A trailing exit must use the
take-profit (TP): its level is maintained by a ratchet (only rises for a long, only lowers for a
short) and only exits in profit. After a partial sell/cover that leaves a position open, the ratchet
restarts from the current price.

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
    
        [Public method] Return the planned stop-loss price for an anticipated trade (a buy); return 0 if none. A negative or non-finite (NaN/inf) result of the hook is invalid: Null is returned and the system opens the trade without stop-loss (sized with the full planned price as risk). The system also skips the buy when the level is not lower than the planned buy price. It calls the subclass hook _get_price. Python subclasses should override _get_price instead of this method.
        
        :param Datetime datetime: the trade time
        :param float price: the planned buy price
        :return: the stop-loss price
        :rtype: float
        
    .. py:method:: get_short_price(self, datetime, price)
    
        [Public method] Return the planned stop-loss price for an anticipated trade (a short sell); return 0 if none. A negative or non-finite (NaN/inf) result of the hook is invalid: Null is returned and the system opens the trade without stop-loss (sized with the full planned price as risk). The system also skips the short entry when the level is not above the planned trade price. It calls the subclass hook _get_short_price. Python subclasses should override _get_short_price instead of this method.
        
        :param Datetime datetime: the trade time
        :param float price: the planned trade price
        :return: the short stop-loss price
        :rtype: float
        
    .. py:method:: _get_price(self, datetime, price)
    
        [Override hook] Compute the raw long stop-loss price; the public get_price delegates to this hook. It is called once per bar; the returned level must use the same (adjusted) price coordinate as the KData passed to setTO (0 means no stop-loss). A negative or non-finite (NaN/inf) result is invalid, and the system opens the trade without stop-loss.
        
        .. note::
            In general, stop-loss and take-profit algorithms are interchangeable. The difference is that a stop-loss implementation may use the planned trade price—for example, setting the stop 30% below the buy price—whereas a take-profit ignores the price argument and treats it as 0.0. In practice, even stop-loss implementations are better off ignoring price: a stop based on, say, a level relative to the previous day's low does not need it.
        
        :param Datetime datetime: the trade time
        :param float price: the planned buy price
        :return: the stop-loss price
        :rtype: float
        
    .. py:method:: _get_short_price(self, datetime, price)
    
        [Override hook] Compute the raw short stop-loss price; the public get_short_price delegates to this hook. It is called once per bar; the returned level must use the same (adjusted) price coordinate as the KData passed to setTO. Default returns 0 (no short stop-loss); a negative or non-finite (NaN/inf) result is invalid, and the system opens the trade without stop-loss. Unlike a long stop-loss, a short one must be above the planned trade price (otherwise the system abandons the short entry), and must not fall back to the long-side _get_price result.
        
        :param Datetime datetime: the trade time
        :param float price: the planned trade price
        :return: the short stop-loss price
        :rtype: float
        
    .. py:method:: _calculate(self)
    
        [Override hook] The subclass calculation interface
    
    .. py:method:: _reset(self)
    
        [Override hook] The subclass reset interface; resets the internal private variables
    
    .. py:method:: _clone(self)
    
        [Override hook] The subclass clone interface
