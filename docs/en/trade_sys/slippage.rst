.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Slippage Algorithm|SP
=====================

Built-in Slippage Algorithms
----------------------------

.. py:function:: SP_FixedPercent([p=0.001])

    Fixed-percentage slippage: the actual buy price equals the planned buy price multiplied by (1 + p), and the actual sell price equals the planned sell price multiplied by (1 - p). p must be in [0, 1).

    :param float p: the fixed percentage offset
    :return: the slippage algorithm instance


.. py:function:: SP_FixedValue([value=0.01])

    Fixed absolute-price slippage: the actual buy price equals the planned buy price plus the offset, and the actual sell price equals the planned sell price minus the offset.

    :param float value: the absolute price offset, must be >= 0
    :return: the slippage algorithm instance

.. py:function:: SP_Uniform([min_value=-0.05, max_value=0.05])

    Random slippage with a uniform distribution. [min_value, max_value] is the offset *amplitude range*: the sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction (added on buys, subtracted on sells), so it is a one-sided offset rather than a symmetric +/- offset.

    :param float min_value: the lower bound of the offset amplitude range
    :param float max_value: the upper bound of the offset amplitude range, must be >= min_value
    :return: the slippage algorithm instance

.. py:function:: SP_Normal([mean=0.0, stddev=0.05])

    Random slippage with a normal distribution. The sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction (added on buys, subtracted on sells), so the actual expected slippage is the folded expectation ``E|X|``, not ``mean``:

        E|X| = stddev * sqrt(2/pi) * exp(-mean^2 / (2 * stddev^2)) + mean * erf(mean / (stddev * sqrt(2)))

    With ``mean`` = 0, ``E|X| = stddev * sqrt(2/pi)``, i.e. about ``0.7979 * stddev``: ``SP_Normal(0.0, 0.05)`` gives an expected slippage of about 4%. The ``mean`` only becomes visible when it is comparable to ``stddev`` (``SP_Normal(0.05, 0.05)`` gives about 5.8%); as it grows far beyond ``stddev``, ``E|X|`` approaches ``mean`` - ``mean`` only shifts the center of the distribution before folding and is not the expected slippage itself

    :param float mean: the mean of the normal distribution (the center before folding; not the actual expected slippage)
    :param float stddev: the standard deviation of the normal distribution, must be >= 0
    :return: the slippage algorithm instance

.. py:function:: SP_LogNormal([mean=0.0, stddev=0.05])

    Random slippage with a log-normal distribution. The sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction (added on buys, subtracted on sells).

    :param float mean: the mean of the log-normal distribution
    :param float stddev: the standard deviation of the log-normal distribution, must be >= 0
    :return: the slippage algorithm instance

.. py:function:: SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1])

    Random slippage with a truncated normal distribution. The sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction, and the sample is drawn analytically (inverse-CDF) from N(mean, stddev^2) truncated to [min_value, max_value].

    :param float mean: the mean of the underlying normal distribution
    :param float stddev: the standard deviation of the underlying normal distribution, must be >= 0
    :param float min_value: the lower truncation bound
    :param float max_value: the upper truncation bound, must be >= min_value
    :return: the slippage algorithm instance

.. note::

    The random slippage algorithms (SP_Uniform / SP_Normal / SP_LogNormal / SP_TruncNormal) hold an internal random engine that is not thread-safe. For concurrent backtests (batch backtests, WalkForward, etc.) each instance must be cloned; do not set ``shared_sp`` to ``True`` to share one instance across multiple Systems.

.. note::

    Each random slippage carries a ``seed`` parameter (int64, default 0 = a random seed). A non-zero value makes the sequence of one instance reproducible after ``reset()``; ``seed`` is read only in ``reset()``, so setting it via ``set_param("seed", ...)`` takes effect only after an explicit ``reset()`` (inside the framework, ``System`` calls ``reset()`` before each ``run()``).

.. note::

    The reproducibility of the random slippage algorithms has a boundary: ``seed`` guarantees an identical sequence only within the same platform and the same standard library implementation. SP_Normal / SP_LogNormal / SP_Uniform rely on the standard library distributions (``std::normal_distribution`` / ``std::lognormal_distribution`` / ``std::uniform_real_distribution``), whose sampling algorithm is left to the implementation, so the same ``seed`` does not give the same sequence on libstdc++ / libc++ / MSVC; SP_TruncNormal samples through its own inverse CDF and is not affected by that. The sequence of one instance also depends on how many times and in which order the hooks are called. Cross-platform and cross-standard-library reproducibility is not promised.


Custom Slippage Algorithm
-------------------------

A custom slippage algorithm implements the following interface:

* :py:meth:`SlippageBase._getRealBuyPrice` - [Required] Compute the raw buy price (the System part validates it, snaps it to the market's minimum price tick and clamps it into the bar's [low, high] range)
* :py:meth:`SlippageBase._getRealSellPrice` - [Required] Compute the raw sell price (the System part validates it, snaps it to the market's minimum price tick and clamps it into the bar's [low, high] range)
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

        [Public method] Return the raw buy price of the slippage model by calling the subclass hook _get_real_buy_price. The System part is the only caller in the framework: it validates the result (an invalid value falls back to the plan price), snaps it to the market's minimum price tick and clamps it into the bar's [low, high] range before the trade. Python subclasses should override _get_real_buy_price instead of this method.

        :param Datetime datetime: the buy timestamp
        :param float price: the planned buy price
        :return: the raw buy price of the slippage model
        :rtype: float

    .. py:method:: get_real_sell_price(self, datetime, price)

        [Public method] Return the raw sell price of the slippage model by calling the subclass hook _get_real_sell_price. The System part is the only caller in the framework: it validates the result (an invalid value falls back to the plan price), snaps it to the market's minimum price tick and clamps it into the bar's [low, high] range before the trade. Python subclasses should override _get_real_sell_price instead of this method.

        :param Datetime datetime: the sell timestamp
        :param float price: the planned sell price
        :return: the raw sell price of the slippage model
        :rtype: float

    .. py:method:: _get_real_buy_price(self, datetime, price)

        [Override hook] Compute the raw buy price; the slippage algorithm only produces the price of the model, while the validity check and the day-range clamp are applied by the System part.

        :param Datetime datetime: the buy timestamp
        :param float price: the planned buy price
        :return: the raw buy price of the slippage model
        :rtype: float

    .. py:method:: _get_real_sell_price(self, datetime, price)

        [Override hook] Compute the raw sell price; the slippage algorithm only produces the price of the model, while the validity check and the day-range clamp are applied by the System part.

        :param Datetime datetime: the sell timestamp
        :param float price: the planned sell price
        :return: the raw sell price of the slippage model
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
