.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Profit Goal Strategy|PG
=======================

Built-in Profit Goal Strategies
-------------------------------

.. py:function:: PG_FixedPercent([p = 0.2])

    Fixed-percentage profit goal: the goal price is the average cost multiplied by (1 + p) for a long or (1 - p) for a short, rounded to the price precision; the current price is used as the cost when there is no position yet.

    :param float p: the percentage
    :return: the profit goal strategy instance

.. py:function:: PG_FixedHoldDays([days=5])

    Profit goal that forces an exit after a fixed number of holding days. Once a long or short position has been held for the given number of trading days (counted by the days present in the instrument's own K-line), it is closed on the next bar regardless of price. setTO must be called first, otherwise there is no trading-day list and the rule never takes effect.

    :param int days: the maximum allowed holding period, counted in trading days; defaults to 5
    :return: the profit goal strategy instance

.. py:function:: PG_NoGoal()

    No profit goal; typically used for testing or comparison.

    :return: the profit goal strategy instance


Return Value Semantics and Trigger Direction
----------------------------------------------

The meaning of the returned value for ``get_goal`` / ``_get_goal`` and ``get_short_goal`` / ``_get_short_goal``:

.. list-table::
    :header-rows: 1
    :widths: 30 70

    * - Return value
      - Meaning
    * - ``constant.null_price``
      - No goal set; never triggers
    * - finite positive
      - The goal price
    * - ``0``
      - Long: the position should be sold (forced exit); Short: no effect (``price <= 0`` never holds)
    * - negative, ``±inf``
      - Invalid; clamped by the base class to ``constant.null_price`` (no goal, no trigger, no warning)

Trigger direction:

.. list-table::
    :header-rows: 1
    :widths: 25 75

    * - Side
      - Trigger condition and action
    * - Long
      - Sell when the current price >= the goal price (trade ``from`` is ``PART_PROFITGOAL``)
    * - Short
      - Cover when the current price <= the goal price (trade ``from`` is ``PART_PROFITGOAL``)

.. note::
    ``PG_FixedHoldDays`` implements its hold-days exit through a sentinel return value (0 for a long, the current price for a short), so the ``goalPrice`` of such a forced exit is not a meaningful goal price.

Custom Profit Goal Strategy
---------------------------

A custom profit goal strategy implements the following interface:

* :py:meth:`ProfitGoalBase._get_goal` - [Required] Return the goal price
* :py:meth:`ProfitGoalBase._get_short_goal` - [Optional] Return the short goal price; returning constant.null_price means no short goal is set
* :py:meth:`ProfitGoalBase._calculate` - [Optional] Subclass calculation hook
* :py:meth:`ProfitGoalBase._clone` - [Required] Subclass clone hook
* :py:meth:`ProfitGoalBase._reset` - [Optional] Reset internal member variables
* :py:meth:`ProfitGoalBase.buy_notify` - [Optional] Receive notification of an actual buy; reserved for handling multiple additions to or reductions of a position
* :py:meth:`ProfitGoalBase.sell_notify` - [Optional] Receive notification of an actual sell; reserved for handling multiple additions to or reductions of a position


Profit Goal Strategy Base Class
-------------------------------

.. py:class:: ProfitGoalBase

    Base class for profit goal strategies.

    .. py:attribute:: name Name
    .. py:attribute:: to Set or get the traded K-line (bar) data (TO)
    .. py:attribute:: tm Set or get the trade manager account

    .. py:method:: __init__(self[, name="ProfitGoalBase"])

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

    .. py:method:: reset(self)

        Reset the part to its initial state.

    .. py:method:: clone(self)

        Create and return a copy of this instance.

    .. py:method:: get_goal(self, datetime, price)

        Return the profit goal price. Returning constant.null_price means no goal is set; returning 0 means the position should be sold.

        :param Datetime datetime: the current time
        :param float price: the current price
        :return: the goal price
        :rtype: float

    .. py:method:: get_short_goal(self, datetime, price)

        Return the short profit goal price. Returning constant.null_price means no short goal is set.

        :param Datetime datetime: the current time
        :param float price: the current price
        :return: the short goal price
        :rtype: float

    .. py:method:: _get_goal(self, datetime, price)

        [Override hook] Return the profit goal price. Returning constant.null_price means no goal is set; returning 0 means the position should be sold.

        :param Datetime datetime: the current time
        :param float price: the current price
        :return: the goal price
        :rtype: float

    .. py:method:: _get_short_goal(self, datetime, price)

        [Override hook] Return the short profit goal price. Returning constant.null_price means no short goal is set; the short position is closed when the current price is <= the returned price. The short cost is taken from the current short position.

        :param Datetime datetime: the current time
        :param float price: the current price
        :return: the short goal price
        :rtype: float

    .. py:method:: buy_notify(self, trade_record)

        [Override hook] Called when the trading system executes an actual buy, notifying the part of the trade. This usually only needs to be overridden when a position is added to or reduced multiple times.

        :param TradeRecord trade_record: the trade record of the actual buy when the fill occurs

    .. py:method:: sell_notify(self, trade_record)

        [Override hook] Called when the trading system executes an actual sell, notifying the part of the trade. This usually only needs to be overridden when a position is added to or reduced multiple times.

        :param TradeRecord trade_record: the trade record of the actual sell when the fill occurs

    .. py:method:: _calculate(self)

        [Override hook] Subclass calculation hook.

    .. py:method:: _reset(self)

        [Override hook] Subclass reset hook, used to reset internal private state.

    .. py:method:: _clone(self)

        [Override hook] Subclass clone hook.
