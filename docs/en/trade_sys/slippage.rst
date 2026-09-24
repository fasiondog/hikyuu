.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Slippage Algorithm|SP
=====================

Built-in Slippage Algorithms
----------------------------

.. py:function:: SP_FixedPercent([p=0.001])

    Fixed-percentage slippage: the actual buy price equals the planned buy price multiplied by (1 + p), and the actual sell price equals the planned sell price multiplied by (1 - p).

    :param float p: the fixed percentage offset
    :return: the slippage algorithm instance


.. py:function:: SP_FixedValuet([p=0.001])

    Fixed absolute-price slippage: the actual buy price equals the planned buy price plus the offset, and the actual sell price equals the planned sell price minus the offset.

    :param float p: the absolute price offset
    :return: the slippage algorithm instance

.. py:function:: SP_Uniform([min_value=-0.05, max_value=0.05])

    Random slippage with a uniform distribution: for both buys and sells, the price offset is drawn uniformly from the interval [min_value, max_value].

    :param float min_value: the minimum price offset
    :param float max_value: the maximum price offset
    :return: the slippage algorithm instance

.. py:function:: SP_Normal([mean=0.0, stddev=0.05])

    Random slippage with a normal distribution: for both buys and sells, the price offset is drawn from N(mean, stddev^2), i.e. a normal distribution with the given mean and standard deviation.

    :param float mean: the mean of the normal distribution
    :param float stddev: the standard deviation of the normal distribution
    :return: the slippage algorithm instance

.. py:function:: SP_LogNormal([mean=0.0, stddev=0.05])

    Random slippage with a log-normal distribution: for both buys and sells, the price offset is drawn from a log-normal distribution parameterized by the given mean and standard deviation.

    :param float mean: the mean of the log-normal distribution
    :param float stddev: the standard deviation of the log-normal distribution
    :return: the slippage algorithm instance

.. py:function:: SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1])

    Random slippage with a truncated normal distribution: for both buys and sells, the price offset is drawn from N(mean, stddev^2) truncated to the interval [min_value, max_value].

    :param float mean: the mean of the underlying normal distribution
    :param float stddev: the standard deviation of the underlying normal distribution
    :param float min_value: the lower truncation bound
    :param float max_value: the upper truncation bound
    :return: the slippage algorithm instance


Custom Slippage Algorithm
-------------------------

A custom slippage algorithm implements the following interface:

* :py:meth:`SlippageBase.get_real_buy_price` - [Required] Compute the actual buy price
* :py:meth:`SlippageBase.get_real_sell_price` - [Required] Compute the actual sell price
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

        [Override hook] Compute the actual buy price.

        :param Datetime datetime: the buy timestamp
        :param float price: the planned buy price
        :return: the actual buy price
        :rtype: float

    .. py:method:: get_real_sell_price(self, datetime, price)

        [Override hook] Compute the actual sell price.

        :param Datetime datetime: the sell timestamp
        :param float price: the planned sell price
        :return: the actual sell price
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
