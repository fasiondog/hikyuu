.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Slippage Algorithm|SP
=====================

Built-in Slippage Algorithms
----------------------------

.. py:function:: SP_FixedPercent([p=0.001, short_p=None])

    Fixed-percentage slippage: the actual buy price equals the planned buy price multiplied by (1 + p), and the actual sell price equals the planned sell price multiplied by (1 - p). p must be in [0, 1).

    :param float p: the fixed percentage offset
    :param float short_p: the fixed percentage used when opening and covering a short, must be in [0, 1); None (default) makes the short side reuse p
    :return: the slippage algorithm instance


.. py:function:: SP_FixedValue([value=0.01, short_value=None])

    Fixed absolute-price slippage: the actual buy price equals the planned buy price plus the offset, and the actual sell price equals the planned sell price minus the offset.

    :param float value: the absolute price offset, must be >= 0
    :param float short_value: the absolute price offset used when opening and covering a short, must be >= 0; None (default) makes the short side reuse value
    :return: the slippage algorithm instance

.. py:function:: SP_Uniform([min_value=-0.05, max_value=0.05, adverse_only=True, short_min_value=None, short_max_value=None])

    Random slippage with a uniform distribution. When ``adverse_only=True`` (default), the sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction (added on buys, subtracted on sells) — a one-sided cost model. When ``adverse_only=False``, the raw signed value is applied directly as zero-mean execution noise (not necessarily symmetric; zero mean when the range is centered at 0).

    :param float min_value: the lower bound of the offset range
    :param float max_value: the upper bound of the offset range, must be >= min_value
    :param bool adverse_only: True (default) folds to unfavorable only; False uses the raw signed offset
    :param float short_min_value: the lower bound of the short-side offset range; None reuses min_value
    :param float short_max_value: the upper bound of the short-side offset range; None reuses max_value
    :return: the slippage algorithm instance

.. py:function:: SP_Normal([mean=0.0, stddev=0.05, adverse_only=True, short_mean=None, short_stddev=None])

    Random slippage with a normal distribution. When ``adverse_only=True`` (default), the sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction, so the actual expected slippage is the folded expectation ``E|X|``, not ``mean``:

        E|X| = stddev * sqrt(2/pi) * exp(-mean^2 / (2 * stddev^2)) + mean * erf(mean / (stddev * sqrt(2)))

    With ``mean`` = 0, ``E|X| = stddev * sqrt(2/pi)``, i.e. about ``0.7979 * stddev``. When ``adverse_only=False``, the raw signed value is applied directly as zero-mean execution noise (not necessarily symmetric; its expectation is ``mean``).

    :param float mean: the mean of the normal distribution
    :param float stddev: the standard deviation of the normal distribution, must be >= 0
    :param bool adverse_only: True (default) folds to unfavorable only; False uses the raw signed offset
    :param float short_mean: the mean of the short-side normal distribution; None reuses mean
    :param float short_stddev: the standard deviation of the short-side normal distribution, must be >= 0; None reuses stddev
    :return: the slippage algorithm instance

.. py:function:: SP_LogNormal([mean=0.0, stddev=0.05, adverse_only=True, short_mean=None, short_stddev=None])

    Random slippage with a log-normal distribution. When ``adverse_only=True`` (default), the centered sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction. When ``adverse_only=False``, the raw centered value (``X - E[X]``) is applied directly as zero-mean execution noise: its expectation is exactly 0, but the distribution is right-skewed and not necessarily symmetric, and ``mean`` only raises the dispersion without producing a drift (unlike ``mean`` in ``SP_Normal``).

    :param float mean: the mean of the log-normal distribution
    :param float stddev: the standard deviation of the log-normal distribution, must be >= 0
    :param bool adverse_only: True (default) folds to unfavorable only; False uses the raw signed offset
    :param float short_mean: the mean of the short-side log-normal distribution; None reuses mean
    :param float short_stddev: the standard deviation of the short-side log-normal distribution, must be >= 0; None reuses stddev
    :return: the slippage algorithm instance

.. py:function:: SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1, adverse_only=True, short_mean=None, short_stddev=None, short_min_value=None, short_max_value=None])

    Random slippage with a truncated normal distribution. When ``adverse_only=True`` (default), the sampled value is folded to a non-negative amplitude and applied only in the unfavorable direction. When ``adverse_only=False``, the raw signed value is applied directly as zero-mean execution noise (not necessarily symmetric; the expectation equals ``mean`` only when the truncation interval is centered on it). The sample is drawn analytically (inverse-CDF) from N(mean, stddev^2) truncated to [min_value, max_value].

    :param float mean: the mean of the underlying normal distribution
    :param float stddev: the standard deviation of the underlying normal distribution, must be >= 0
    :param float min_value: the lower truncation bound
    :param float max_value: the upper truncation bound, must be >= min_value
    :param bool adverse_only: True (default) folds to unfavorable only; False uses the raw signed offset
    :param float short_mean: the mean of the short-side distribution; None reuses mean
    :param float short_stddev: the standard deviation of the short-side distribution, must be >= 0; None reuses stddev
    :param float short_min_value: the lower short-side truncation bound; None reuses min_value
    :param float short_max_value: the upper short-side truncation bound; None reuses max_value
    :return: the slippage algorithm instance

.. note::

    The built-in slippages take both the long-side and the short-side parameters. The short side covers two actions: opening a short (a short sell, served by get_real_sell_short_price) and covering it (a buy back, served by get_real_buy_short_price). A short_* parameter left at None takes the value of its long-side counterpart one parameter at a time when the instance is built, so a newly built instance with no short_* argument keeps the short side exactly as it was before; changing a long-side parameter afterwards does not move the short side. ``adverse_only`` is not per-side and is shared by both.

.. warning::

    An archive serialized by an **older version** (XML / pickle, including a System archive) carries no ``short_*`` key, so after deserialization the short side takes the class default instead of the matching long-side value (for example an old ``SP_FixedPercent(0.05)`` slips its short side by 0.001 after the upgrade). To keep the previous behavior, fill the short parameters in explicitly after loading: ``sp.set_param("short_p", sp.get_param("p"))`` (likewise for the other models).

.. note::

    The random slippage algorithms (SP_Uniform / SP_Normal / SP_LogNormal / SP_TruncNormal) hold an internal random engine that is not thread-safe. For concurrent backtests (batch backtests, WalkForward, etc.) each instance must be cloned; do not set ``shared_sp`` to ``True`` to share one instance across multiple Systems.

.. note::

    Each random slippage carries a ``seed`` parameter (int64, default 0 = a random seed). A non-zero value makes the sequence of one instance reproducible after ``reset()``; ``seed`` is read only in ``reset()``, so setting it via ``set_param("seed", ...)`` takes effect only after an explicit ``reset()`` (inside the framework, ``System`` calls ``reset()`` before each ``run()``).

.. note::

    The reproducibility of the random slippage algorithms has a boundary: ``seed`` guarantees an identical sequence only within the same platform and the same standard library implementation. SP_Normal / SP_LogNormal / SP_Uniform rely on the standard library distributions (``std::normal_distribution`` / ``std::lognormal_distribution`` / ``std::uniform_real_distribution``), whose sampling algorithm is left to the implementation, so the same ``seed`` does not give the same sequence on libstdc++ / libc++ / MSVC; SP_TruncNormal samples through its own inverse CDF and is not affected by that. The sequence of one instance also depends on how many times and in which order the hooks are called. Cross-platform and cross-standard-library reproducibility is not promised.

.. note::

    The four random slippages warn consistently on an absurd parameter (a warning only, the calculation is unchanged): a stddev above 1.0 (SP_Normal / SP_LogNormal) or an amplitude above 1.0 (SP_Uniform / SP_TruncNormal, i.e. the larger absolute bound of the range) logs a warning; the long side and the short_ side are judged separately; the parameter itself has no upper bound.


Custom Slippage Algorithm
-------------------------

A custom slippage algorithm implements the following interface:

* :py:meth:`SlippageBase._getRealBuyPrice` - [Required] Compute the raw buy price (the System part validates it, snaps it to the market's minimum price tick and clamps it into the bar's [low, high] range)
* :py:meth:`SlippageBase._getRealSellPrice` - [Required] Compute the raw sell price (the System part validates it, snaps it to the market's minimum price tick and clamps it into the bar's [low, high] range)
* :py:meth:`SlippageBase._calculate` - [Required] Subclass calculation hook
* :py:meth:`SlippageBase._clone` - [Required] Subclass clone hook
* :py:meth:`SlippageBase._reset` - [Optional] Reset internal member variables
* :py:meth:`SlippageBase._getRealBuyShortPrice` - [Optional] Compute the raw price of covering a short, delegating to ``_getRealBuyPrice`` when it is not overridden
* :py:meth:`SlippageBase._getRealSellShortPrice` - [Optional] Compute the raw price of opening a short, delegating to ``_getRealSellPrice`` when it is not overridden

Notes on a custom implementation:

* Assigning the ``to`` property calls ``_calculate`` once when the kdata is not empty and differs from the last calculated one (assigning the same kdata again does not call it again); ``reset()`` clears ``to`` and calls ``_reset()``; ``clone()`` calls ``_clone()``, and the clone keeps ``to`` but is marked as not calculated, so the next ``to`` assignment recalculates it.
* Do not override ``get_real_buy_price`` / ``get_real_sell_price``; implement ``_get_real_buy_price`` / ``_get_real_sell_price`` only: the base versions only forward the call (returning the plan price as-is when it is not positive) and the price authority belongs to the System part.
* A hook must return a finite positive number: a non-finite value (NaN / Inf) or a value ``<= 0`` is invalid and the System part falls back to the plan price with a warning.
* The System part first snaps the price to the market's minimum price tick and then clamps it into the bar's [low, high] range: when the slippage pushes the price outside that range (e.g. a buy price above the day's high or a sell price below the day's low), the clamp weakens and can even cancel the slippage.
* In a Python subclass, ``_calculate`` must not call ``super()._calculate()`` (infinite recursion).
* Override ``_get_real_buy_short_price`` / ``_get_real_sell_short_price`` only when the short side needs a different slippage from the long side; when they are not overridden the base class delegates them to ``_get_real_buy_price`` / ``_get_real_sell_price`` and an existing implementation keeps its behavior.


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

    .. py:method:: get_real_buy_short_price(self, datetime, price)

        [Public method] Return the raw price used to cover a short (buying the stock back) by calling the subclass hook _get_real_buy_short_price, which falls back to _get_real_buy_price when that hook is not overridden. The System part validates the result the same way as get_real_buy_price does.

        :param Datetime datetime: the buy timestamp
        :param float price: the planned buy price
        :return: the raw price of the slippage model for covering a short
        :rtype: float

    .. py:method:: get_real_sell_short_price(self, datetime, price)

        [Public method] Return the raw price used to open a short (a short sell) by calling the subclass hook _get_real_sell_short_price, which falls back to _get_real_sell_price when that hook is not overridden.

        :param Datetime datetime: the sell timestamp
        :param float price: the planned sell price
        :return: the raw price of the slippage model for opening a short
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

    .. py:method:: _get_real_buy_short_price(self, datetime, price)

        [Override hook] Compute the raw price of covering a short; override it only when the short side should slip differently from the long side, the default delegates to _get_real_buy_price.

        :param Datetime datetime: the buy timestamp
        :param float price: the planned buy price
        :return: the raw price for covering a short
        :rtype: float

    .. py:method:: _get_real_sell_short_price(self, datetime, price)

        [Override hook] Compute the raw price of opening a short; override it only when the short side should slip differently from the long side, the default delegates to _get_real_sell_price.

        :param Datetime datetime: the sell timestamp
        :param float price: the planned sell price
        :return: the raw price for opening a short
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
