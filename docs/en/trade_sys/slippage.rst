.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Slippage Algorithm|SP
=====================

Built-in Slippage Algorithms
----------------------------

.. py:function:: SP_FixedPercent([p=0.001])

    Fixed-percentage slippage: the actual buy price equals the planned buy price multiplied by (1 + p), and the actual sell price equals the planned sell price multiplied by (1 - p). p must be in [0, 0.1).

    :param float p: the fixed percentage offset
    :return: the slippage algorithm instance


.. py:function:: SP_FixedValue([value=0.01])

    Fixed absolute-price slippage: the actual buy price equals the planned buy price plus the offset, and the actual sell price equals the planned sell price minus the offset.

    :param float value: the absolute price offset
    :return: the slippage algorithm instance

.. py:function:: SP_Uniform([min_value=-0.05, max_value=0.05])

    Random slippage with a uniform distribution. [min_value, max_value] is the offset *amplitude range*: the sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction (added on buys, subtracted on sells), so it is a one-sided offset rather than a symmetric +/- offset.

    :param float min_value: the lower bound of the offset amplitude range
    :param float max_value: the upper bound of the offset amplitude range
    :return: the slippage algorithm instance

.. py:function:: SP_Normal([mean=0.0, stddev=0.05])

    Random slippage with a normal distribution. The sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction (added on buys, subtracted on sells).

    :param float mean: the mean of the normal distribution
    :param float stddev: the standard deviation of the normal distribution
    :return: the slippage algorithm instance

.. py:function:: SP_LogNormal([mean=0.0, stddev=0.05])

    Random slippage with a log-normal distribution. The sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction (added on buys, subtracted on sells).

    :param float mean: the mean of the log-normal distribution
    :param float stddev: the standard deviation of the log-normal distribution
    :return: the slippage algorithm instance

.. py:function:: SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1])

    Random slippage with a truncated normal distribution. The sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction, and the sample is drawn analytically (inverse-CDF) from N(mean, stddev^2) truncated to [min_value, max_value].

    :param float mean: the mean of the underlying normal distribution
    :param float stddev: the standard deviation of the underlying normal distribution
    :param float min_value: the lower truncation bound
    :param float max_value: the upper truncation bound
    :return: the slippage algorithm instance

.. note::

    The random slippage algorithms (SP_Uniform / SP_Normal / SP_LogNormal / SP_TruncNormal) hold an internal random engine that is not thread-safe. For concurrent backtests (batch backtests, WalkForward, etc.) each instance must be cloned; do not set ``shared_sp`` to ``True`` to share one instance across multiple Systems.


Custom Slippage Algorithm
-------------------------

A custom slippage algorithm implements the following interface:

* :py:meth:`SlippageBase._getRealBuyPrice` - [Required] Compute the raw buy price (the base class clamps it to not exceed the day's high)
* :py:meth:`SlippageBase._getRealSellPrice` - [Required] Compute the raw sell price (the base class clamps it to not fall below the day's low)
* :py:meth:`SlippageBase._calculate` - [Required] Subclass calculation hook
* :py:meth:`SlippageBase._clone` - [Required] Subclass clone hook
* :py:meth:`SlippageBase._reset` - [Optional] Reset internal member variables


Slippage Algorithm Base Class
-----------------------------

.. py:class:: SlippageBase

    Base class for slippage algorithms.

    .. py:attribute:: name Name

    .. py:method:: __init__(self[, name="SlippageBase"])

        Constructor.

        :param str name: the name

    .. py:method:: get_param(self, name)

        Get the value of the specified parameter.

        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: raised if no such parameter exists

    .. py:method:: set_param(self, name, value)

        Set the value of a parameter.

        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! Raised when the parameter type is not supported

    .. py:method:: get_real_buy_price(self, datetime, price)

        [Public method] Return the actual buy price. It calls the subclass hook _get_real_buy_price and clamps the result to not exceed the day's high. Python subclasses should override _get_real_buy_price instead of this method.

        :param Datetime datetime: the buy timestamp
        :param float price: the planned buy price
        :return: the actual buy price, clamped to not exceed the day's high
        :rtype: float

    .. py:method:: get_real_sell_price(self, datetime, price)

        [Public method] Return the actual sell price. It calls the subclass hook _get_real_sell_price and clamps the result to not fall below the day's low. Python subclasses should override _get_real_sell_price instead of this method.

        :param Datetime datetime: the sell timestamp
        :param float price: the planned sell price
        :return: the actual sell price, clamped to not fall below the day's low
        :rtype: float

    .. py:method:: _get_real_buy_price(self, datetime, price)

        [Override hook] Compute the raw buy price; the base class clamps it to not exceed the day's high before returning the actual trade price.

        :param Datetime datetime: the buy timestamp
        :param float price: the planned buy price
        :return: the raw buy price (the base class returns it clamped to not exceed the day's high)
        :rtype: float

    .. py:method:: _get_real_sell_price(self, datetime, price)

        [Override hook] Compute the raw sell price; the base class clamps it to not fall below the day's low before returning the actual trade price.

        :param Datetime datetime: the sell timestamp
        :param float price: the planned sell price
        :return: the raw sell price (the base class returns it clamped to not fall below the day's low)
        :rtype: float

    .. py:method:: reset(self)

        Reset the part to its initial state.

    .. py:method:: clone(self)

        Create and return a copy of this instance.

    .. py:method:: _calculate(self)

        [Override hook] Subclass calculation hook.

    .. py:method:: _reset(self)

        [Override hook] Subclass reset hook, used to reset internal private state.

    .. py:method:: _clone(self)

        [Override hook] Subclass clone hook.
