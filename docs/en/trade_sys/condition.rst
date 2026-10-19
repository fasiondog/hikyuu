.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

System Validity Condition|CN
============================

Built-in System Validity Conditions
-----------------------------------

.. py:function:: CN_OPLine(ind)

    Trades are always sized at the instrument's minimum tradable quantity. The condition computes the op value of the equity curve: the system is valid while the equity curve is above op, and invalid otherwise.

    :param Indicator ind: an Indicator instance
    :return: the system validity condition instance
    :rtype: ConditionBase

    Note: CN_OPLine runs one embedded full backtest per instrument, so its cost is roughly that of a single-instrument backtest; for large portfolios (many subsystems) it nearly doubles the total backtesting cost. Prefer CN_Bool for large portfolios.

    Note: its valid range depends on the SYS_Simple/System default parameters (buy_delay, sell_delay, can_trade_when_high_eq_low, the TC_Zero cost model, no slippage). These defaults are not pinned explicitly, so any change to them silently shifts the valid range.

.. py:function:: CN_Bool(ind)

    A system validity condition driven by a boolean indicator: the system is valid where the indicator's value is greater than 0, and invalid elsewhere (a non-finite value is invalid).

    :param Indicator ind: a boolean indicator computed over KData (bar data)
    :return: the system validity condition instance
    :rtype: ConditionBase


Combination Operators
---------------------

System validity conditions support combination operators, merging several conditions into a new one (the result is itself a ``ConditionBase`` and can be combined further). Each condition's values are aligned bar-by-bar with the trading object.

Validity rule: a bar is valid when its value is finite and ``> 0``, otherwise invalid (NaN and ±inf are invalid); ``is_valid`` follows this rule. "all 0 (invalid)" below means the operator sets those bars to ``0.0``.

.. list-table::
   :header-rows: 1

   * - Operator
     - Both sides
     - Only cond1 (right null)
     - Only cond2 (left null)
     - Both null
   * - ``cn1 + cn2`` (Add)
     - sum of both; NaN if either is non-finite
     - pass through cond1
     - pass through cond2
     - all 0 (invalid)
   * - ``cn1 - cn2`` (Sub)
     - difference of both; NaN if either is non-finite
     - pass through cond1
     - ``-cond2``
     - all 0 (invalid)
   * - ``cn1 | cn2`` (Or)
     - 1.0 if either side >0, else 0
     - normalized to 0/1
     - normalized to 0/1
     - all 0 (invalid)
   * - ``cn1 & cn2`` (And)
     - 1.0 if both sides >0, else 0
     - all 0 (invalid)
     - all 0 (invalid)
     - all 0 (invalid)
   * - ``cn1 * cn2`` (Multi)
     - product of both; NaN if either is non-finite
     - all 0 (invalid)
     - all 0 (invalid)
     - all 0 (invalid)
   * - ``cn1 / cn2`` (Div)
     - quotient of both; NaN if ``cn2 == 0`` or either is non-finite
     - all NaN (invalid)
     - all 0 (invalid)
     - all 0 (invalid)

Note: when either operand is a null pointer, the factory operator logs a warning (Add/Sub/Or: "the single-side mode will be used"; And/Multi/Div: "the result is always invalid") and still constructs per the matrix above.


Custom System Validity Condition
--------------------------------

Quickly define a custom system validity condition that holds no private attributes.

.. py:function:: crtCN(func, params=None, name='crtCN')

    Quickly create a custom system validity condition that holds no private attributes
    
    :param func: the system validity condition function
    :param dict params: the parameter dictionary
    :param str name: the custom name
    :return: the custom system validity condition instance

The custom system validity condition interface:

* :py:meth:`ConditionBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`ConditionBase._clone` - [Required] The clone interface
* :py:meth:`ConditionBase._is_valid` - [Optional] The subclass validity rule, by default "finite and > 0"
* :py:meth:`ConditionBase._reset` - [Optional] Reset the internal member variables


System Validity Condition Base Class
------------------------------------

.. py:class:: ConditionBase

    Base class for system validity conditions
    
    .. py:attribute:: name The condition name
    .. py:attribute:: to Set or get the traded KData (trade object, TO)
    .. py:attribute:: tm Set or get the associated trade manager account
    .. py:attribute:: sg Set or get the associated signal generator
    
    .. py:method:: __init__(self[, name="ConditionBase"])
    
        Constructor
        
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
            
    .. py:method:: is_valid(self, datetime)
    
        Whether the system is valid at the given time
        
        :param Datetime datetime: the specified time
        :return: True means valid | False means invalid

    .. py:method:: get_datetime_list(self)

        Return the dates on which the system is valid. Note that the list contains only valid dates, so its length does not match that of the traded KData (TO).

    .. py:method:: get_values(self)

        Return the actual values as an indicator, aligned bar-by-bar with the trading object (TO). A date is valid when its value is finite and ``> 0``. Boolean/And/Or conditions emit ``1.0`` for valid bars; arithmetic conditions (Add/Sub/Multi/Div) emit the operation result, which can be any real number, and NaN when an operand is missing or non-finite

    .. py:method:: _add_valid(self, datetime[, value=1.0])
    
        Register a time as valid; called from _calculate; repeated calls on the same date accumulate (+=)
        
        :param Datetime datetime: the valid time
        :param float value: the value to add, default 1.0
      
    .. py:method:: reset(self)
    
        The reset operation
    
    .. py:method:: clone(self)
    
        The clone operation        
        
    .. py:method:: _calculate(self)
    
        [Override hook] The subclass calculation interface
    
    .. py:method:: _is_valid(self, datetime, value)
    
        [Override hook] The subclass validity rule; called by is_valid for every date registered through _add_valid; by default a date is valid when its value is finite and ``> 0``
        
        :param Datetime datetime: the queried date
        :param float value: the stored value of the date
        :return: True means valid | False means invalid
    
    .. py:method:: _reset(self)
    
        [Override hook] The subclass reset interface; resets the internal private variables
    
    .. py:method:: _clone(self)
    
        [Override hook] The subclass clone interface
