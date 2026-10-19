.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Market Environment Strategy|EV
=================================

Built-in Market Environment Strategies
-----------------------------------------

.. py:function:: EV_TwoLine(fast, slow[, market = 'SH'])

    Fast/slow-line strategy: the market is valid when the market index's fast line is above its slow line, and invalid otherwise.

    :param Indicator fast: the fast-line indicator
    :param Indicator slow: the slow-line indicator
    :param string market: the market name

.. py:function:: EV_Bool(ind[, market = 'SH'])

    A market environment condition driven by a boolean indicator: the market is valid where the indicator's value is greater than 0, and invalid elsewhere.

    :param Indicator ind: a boolean indicator; values greater than 0 mark the market as valid, all other values mark it as invalid
    :param str market: the market whose trading calendar is used

.. py:function:: EV_Manual()

    A manual environment whose valid dates can only be registered through
    :py:meth:`EnvironmentBase._add_valid`, used for the testing or other purposes.

    The registered dates live until the query changes, so set the query first and register the
    dates afterwards. Note that the combination operators re-run each side's ``_calculate``
    through ``setQuery`` (which clears a manual side); to assign the valid dates of a combined
    result manually, call :py:meth:`EnvironmentBase._add_valid` on the result directly.


Combination Operators
---------------------

Market environment strategies can be combined; the result is itself an ``EnvironmentBase`` and can
be combined further.

The validity rule is "the date's stored value is finite and greater than 0", which is exactly what
``is_valid`` tests; "not output" in the tables below means the date is absent from the result
(treated as missing, so ``is_valid`` returns False).

.. list-table::
   :header-rows: 1

   * - Operator
     - Output dates
     - Output value
     - With one side empty
   * - ``ev1 & ev2`` (And)
     - the dates valid on both sides
     - fixed to 1.0
     - always invalid
   * - ``ev1 | ev2`` (Or)
     - the dates valid on at least one side
     - fixed to 1.0
     - single-side mode
   * - ``ev1 + ev2`` (Add)
     - the union of the dates of both sides (invalid dates included)
     - the sum of the two values; a date present on one side only takes that side's value
     - single-side mode
   * - ``ev1 - ev2`` (Sub)
     - the union of the dates of both sides (invalid dates included)
     - the difference of the two values; a date present on the right side only takes ``-ev2``
     - single-side mode
   * - ``ev1 * ev2`` (Multi)
     - the dates where both sides agree (both valid or both invalid)
     - see the table below
     - always invalid
   * - ``ev1 / ev2`` (Div)
     - the dates where both sides agree (both valid or both invalid)
     - see the table below
     - always invalid

The value of ``Multi`` / ``Div`` is decided by the signs of the two sides on that date:

.. list-table::
   :header-rows: 1

   * - Left value
     - Right value
     - ``ev1 * ev2``
     - ``ev1 / ev2``
   * - ``> 0``
     - ``> 0``
     - ``v1*v2`` (positive, valid)
     - ``v1/v2`` (positive, valid)
   * - ``> 0``
     - ``< 0``
     - not output (disagreement, no combined strength)
     - not output (disagreement, no combined strength)
   * - ``< 0``
     - ``> 0``
     - not output (disagreement, no combined strength)
     - not output (disagreement, no combined strength)
   * - ``< 0``
     - ``< 0``
     - ``-(abs(v1)*abs(v2))`` (negative, invalid)
     - ``-(abs(v1)/abs(v2))`` (negative, invalid)
   * - ``= 0`` (either side)
     - any value
     - not output (that side has no value)
     - not output (that side has no value)


Sharing and Concurrency
-----------------------

The environment is bound to no specific instrument and is shared across systems by default (the
``System`` parameter ``shared_ev=True``): several systems attach to the same ``EnvironmentBase``
instance, whose result depends only on the query condition and thus needs to be calculated once.

Internal concurrency semantics:

* The read interfaces (such as ``is_valid``) take a **shared lock** on every call (called once per
  bar during a system backtest);
* Setting ``query`` and ``reset`` take the **exclusive lock**: a query different from the current
  one triggers a full recalculation, an identical one returns immediately;
* ``_add_valid`` takes no lock itself and can only be called from ``_calculate``.

Usage constraints:

* All systems sharing one environment must use the **same query condition** (the same k-line type,
  recover type and range), otherwise every differing ``setQuery`` reruns the full calculation
  under the exclusive lock and the concurrent backtest degrades into serialized recalculations;
* Concurrent threads sharing one environment contend on the shared lock's cache line; in extremely
  hot call paths each system can own a cloned environment instead (i.e. ``shared_ev=False`` on the
  ``System``);
* For a Python subclass, setting ``query`` and ``reset`` call back into Python while the lock is
  held (needing the GIL), so they must not run concurrently with the other calls on the same
  object (see the base class description below).


Custom Market Environment Strategy
-------------------------------------

The custom market environment strategy interface:

* :py:meth:`EnvironmentBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`EnvironmentBase._clone` - [Required] The clone interface
* :py:meth:`EnvironmentBase._is_valid` - [Optional] The custom validity rule (by default "finite and > 0")
* :py:meth:`EnvironmentBase._reset` - [Optional] Reset the internal member variables

Market Environment Strategy Base Class
-----------------------------------------

.. py:class:: EnvironmentBase

    Base class for market environment strategies

    .. py:attribute:: name The strategy name
    .. py:attribute:: query Set or get the query conditions

    .. py:method:: __init__(self[, name='EnvironmentBase'])

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

        Whether the market is valid at the given time

        :param Datetime datetime: the specified time
        :return: True means valid | False means invalid

        The validity rule is "the stored value of the date is finite and greater than 0"; a date
        that was never registered through ``_add_valid`` is invalid.

        The rule is provided by :py:meth:`_is_valid`, which can be overridden for a value-based
        rule.

    .. py:method:: _is_valid(self, datetime, value)

        [Override hook] The custom validity rule, called by :py:meth:`is_valid` for every date
        registered through ``_add_valid``

        :param Datetime datetime: the queried date
        :param float value: the stored value of the date
        :return: True means valid | False means invalid

        The base implementation is "finite and greater than 0"; subclasses usually need no change
        and may override it to judge on the concrete value. The System judges the market validity
        solely through :py:meth:`is_valid` (one call per bar), so overriding this hook directly
        changes the trading behavior; note that the sides of the combination operators keep using
        the built-in "> 0" rule.

    .. py:method:: get_value(self, datetime)

        Get the stored value of the specified date

        :param Datetime datetime: the specified time
        :return: the stored value; 0.0 for a date that was never registered

        0.0 is returned both for an invalid date and for an absent one; use :py:meth:`is_valid`
        to tell the validity.

    .. py:method:: get_values(self)

        Get the values as a time-series indicator: one record per date registered through
        ``_add_valid``, ordered by date

        :return: Indicator

        The date set and the meaning of the value depend on the operator that produced this
        object (e.g. ``Add``/``Sub`` keep the union of both sides including invalid dates, while
        ``And``/``Or`` keep only the valid dates); only a finite value greater than 0 is valid.

    .. py:method:: _add_valid(self, datetime[, value=1.0])

        Register a time as valid; called from _calculate

        :param Datetime datetime: the valid time
        :param float value: defaulting to 1.0; greater than 0 means valid, and less than or equal to 0 means invalid

        A non-finite value (NaN/inf) is discarded, and adding the same date twice accumulates the
        value instead of replacing it.

        It takes no lock itself, so it can only be called from ``_calculate`` (which runs while the
        object is locked); calling it from elsewhere would race with ``is_valid``.

    .. py:method:: reset(self)

        The reset operation

    .. py:method:: clone(self)

        The clone operation

    .. py:method:: _calculate(self)

        [Override hook] The subclass calculation interface

    .. py:method:: _reset(self)

        [Override hook] The subclass reset interface; resets the internal private variables

    .. py:method:: _clone(self)

        [Override hook] The subclass clone interface

    .. note::

        For a Python subclass, setting ``query`` calls back into ``_calculate`` and ``reset`` into
        ``_reset`` while the mutex is held, and both need the GIL, so they must not run concurrently
        with the other calls on the same object; the ``_clone`` callback of ``clone`` is invoked
        without holding the lock and carries no such restriction.
