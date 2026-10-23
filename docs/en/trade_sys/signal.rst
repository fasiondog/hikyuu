.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Signal Generator (SG)
=====================

A signal generator (SG) produces the buy and sell signals used by a trading system.

Common parameters:

    * **alternate** *(bool|True)* : Whether buy and sell signals must alternate. Single-line generators typically derive their signals from curve inflection points, slope changes, and similar rules, so they can emit several consecutive buy signals or several consecutive sell signals; this parameter forces the two to alternate. Two-line crossover generators already alternate buys and sells by construction, in which case this parameter has no effect.
    * **cycle** *(bool|False)* : Marks the signal as consuming the rebalancing days injected by a MultiSystem (e.g. SG_Cycle); it no longer changes the calculation range -- the signal is always calculated once over the whole KData, so it stays stable within the backtest. A cycle signal is a scheduled signal: ``cycle=true`` and ``alternate=true`` are mutually exclusive, setting cycle forces alternate to false (with a warning); a combination that contains a cycle child inherits both the cycle marker and ``alternate=False``, so the later cycle buys are not dropped by the alternation rule. Modes A/B inject the whole run before it starts, while mode C declares the start day when a sub-system is admitted (an already running sub-system is not re-declared, keeping the legacy Portfolio semantics). Only the signals with ``cycle=true`` consume the days; the plain signals are neither affected nor recalculated by a declaration. Note that a cycle buy is a scheduled entry / re-entry of the sub-system (the first entry, after its own exit, or after the SE dropped it and its shadow was cleared), not a portfolio-level rebalancing action -- the portfolio rebalancing is done by the SE/AF. When the sub-system already holds, whether a cycle buy adds to the position depends on the MM (e.g. MM_Nothing cannot trade again once the cash is used up). **Combination limit**: under mode C a cycle signal cannot be combined with ``SE_Signal`` -- the SE snapshots the buy signals already computed by the sub-systems before the run, while mode C declares the cycle day only on admission, so the snapshot stays empty and the sub-system is never admitted (it enters neither the running pool nor a single trade). A selector that admits by a fixed instrument list or rebalancing schedule (e.g. ``SE_Fixed``) is unaffected, and modes A/B push the whole schedule down before the snapshot, so they do work with ``SE_Signal``. Note also that mode C declares the cycle day on admission only: a stop-loss exit inside the same cycle does not re-open by itself, the SE has to drop the sub-system and admit it again.
    * **support_borrow_stock** *(bool|False)* : Allow short-sale signals to be emitted


Built-in Signal Generators
--------------------------

When technical indicators are used to decide entries and exits, the rule almost always comes down to a crossover between a fast line and a slow line, or to an inflection point on a single curve. The built-in signal generators below cover the majority of these cases.

.. raw:: html

    <table border="1">
        <thead>
            <tr>
                <th>Code</th>
                <th>Name</th>
                <th>Description</th>
            </tr>
        </thead>
        <tbody>
            <tr>
                <td><a href="#target-section">SG_Cross</a></td>
                <td>Two-line crossover signal generator</td>
                <td>Buy when the fast line crosses above the slow line from below;<br>sell when the fast line crosses below the slow line from above.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_CrossGold</a></td>
                <td>Golden cross signal generator</td>
                <td>Golden cross: buy when the fast line crosses above the slow line from below while both lines point upward;<br>death cross: sell when the fast line crosses below the slow line from above while both lines point downward.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Single</a></td>
                <td>Single-line inflection-point signal generator</td>
                <td>Determines the trend of a single curve with the inflection-point algorithm given in the book "Smarter Trading"</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Single2</a></td>
                <td>Single-line inflection-point signal generator 2</td>
                <td>Determines the trend of a single curve with the inflection-point algorithm given in the book "Smarter Trading"</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Flex</a></td>
                <td>Self-crossing single-line signal generator</td>
                <td>Uses EMA(slow_n) of the indicator itself as the slow line, and the indicator itself as the fast line.<br>Buy when the fast line crosses above the slow line,<br>sell when the fast line crosses below the slow line.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Bool</a></td>
                <td>Boolean signal generator</td>
                <td>Takes two indicators whose results behave like boolean arrays, used as the buy condition and the sell condition respectively.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_OneSide</a></td>
                <td>One-sided signal generator</td>
                <td>Builds a one-sided signal from the input indicator (buys only or sells only);<br>a positive indicator value adds the corresponding signal.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Buy</a></td>
                <td>One-sided buy signal generator</td>
                <td>A shorthand for SG_OneSide</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Sell</a></td>
                <td>One-sided sell signal generator</td>
                <td>A shorthand for SG_OneSide</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Band</a></td>
                <td>Range breakout signal generator</td>
                <td>Band breakout generator; buy when the indicator breaks above the upper band;<br>sell when it falls below the lower band.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_AllwaysBuy</a></td>
                <td>Always-buy signal generator</td>
                <td>A special SG that keeps emitting a buy signal on every bar, usually used together with a PF</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Cycle</a></td>
                <td>PF rebalance cycle buy signal generator</td>
                <td>A special SG that emits a buy signal on the rebalancing days injected by a MultiSystem before the run</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Manual</a></td>
                <td>Manual signal generator</td>
                <td>A special SG whose signals are added by hand, for tests or the external signal import</td>
            </tr>
            <tr>
                <td>SG_Add<br>SG_Mul<br>SG_Sub<br>SG_Div<br>SG_And<br>SG_Or</td>
                <td>SG operation helpers</td>
                <td>Because alternate defaults to True for an SG, chaining a form such as "sg1 + sg2 + sg3" can silently carry over the alternate setting of sg1 + sg2<br>It is recommended to use SG_Add(sg1, sg2, False) + sg3 to avoid the alternation problem<br>Add / sub / mul / div keep the signal amplitudes, and / or keep the direction only (see Signal operations below)</td>
            </tr>
        </tbody>
    </table>
    <p></p>


Two-line Crossover Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Cross(fast, slow)

    The two-line crossover signal generator: buy when the fast line crosses above the slow line from below, and sell when the fast line crosses below the slow line from above. For example, buy when a shorter-period MA crosses above a longer-period MA and sell on the opposite cross::

        SG_Cross(MA(CLOSE(), n=10), MA(CLOSE(), n=30))

    :param Indicator fast: the fast line
    :param Indicator slow: the slow line
    :return: the signal generator


Golden Cross Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_CrossGold(fast, slow)

    The golden cross signal generator. A golden cross occurs when the fast line crosses above the slow line from below while both the fast line and the slow line point upward — buy.
    A death cross occurs when the fast line crosses below the slow line from above while both lines point downward — sell.::

        SG_CrossGold(MA(CLOSE(), n=10), MA(CLOSE(), n=30))

    :param Indicator fast: the fast line
    :param Indicator slow: the slow line
    :return: the signal generator


Single-line Inflection Point Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Single(ind[, filter_n = 20, filter_p = 0.1])

    Creates a single-line inflection-point signal generator. It judges the trend of the curve with the inflection-point algorithm given in the book "Smarter Trading" [BOOK1]_. The rules are as follows::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - AMA[1] > filter
        or Buy When AMA - AMA[2] > filter
        or Buy When AMA - AMA[3] > filter

    :param Indicator ind: the input indicator curve
    :param int filter_n: the lookback period N, in bars
    :param float filter_p: the filter percentage
    :return: the signal generator

.. py:function:: SG_Single2(ind[, filter_n = 20, filter_p = 0.1])

    Creates the second variant of the single-line inflection-point signal generator [BOOK1]_::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - @lowest(AMA,n) > filter
        Sell When @highest(AMA, n) - AMA > filter

    :param Indicator ind: the input indicator curve
    :param int filter_n: the lookback period N, in bars
    :param float filter_p: the filter percentage
    :return: the signal generator

Self-crossing Single-line Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Flex(ind, slow_n)

    Uses EMA(slow_n) of the indicator itself as the slow line and the indicator itself as the fast line. Buy when the fast line crosses above the slow line, and sell when the fast line crosses below the slow line.

    :param Indicator ind: the input indicator
    :param int slow_n: the EMA period of the slow line
    :return: the signal generator


Boolean Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Bool(buy, sell[, alternate=True])

    The boolean signal generator: takes two indicators whose results behave like boolean arrays, used as the buy condition and the sell condition respectively.

    :param Indicator buy: the buy indicator (a positive value at a given position in the result means buy)
    :param Indicator sell: the sell indicator (a positive value at a given position in the result means sell)
    :param bool alternate: whether buys and sells must alternate; defaults to True
    :return: the signal generator


One-sided Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_OneSide(ind, is_buy)

    Builds a one-sided signal from the input indicator (containing only buy signals or only sell signals); if the indicator value is positive, the corresponding signal is added.

    :param Indicator ind: the input indicator
    :param bool is_buy: True to build a buy-only signal, otherwise a sell-only signal


.. py:function:: SG_Buy(ind)

    The one-sided buy-only signal; a shorthand for SG_OneSide.

    :param Indicator ind: the input indicator
    :return: the signal generator


.. py:function:: SG_Sell(ind)

    The one-sided sell-only signal; a shorthand for SG_OneSide.

    :param Indicator ind: the input indicator
    :return: the signal generator


Range Breakout Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Band(ind, lower, upper)

    The band breakout signal generator: buy when the indicator breaks above the upper band,
    and sell when the indicator falls below the lower band.

    ::

        SG_Band(MA(C, n=10), 100, 200)
        SG_Band(CLOSE, MA(LOW), MA(HIGH))


Always-buy Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_AllwaysBuy()

    A special SG that keeps emitting a buy signal on every bar, usually used together with a PF.


PF Rebalance Cycle Buy Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Cycle()

    A special SG that emits a buy signal on the rebalancing days injected by a MultiSystem before the run; it stays silent when no day is injected.
    A custom Python cycle signal can read the injected days in ``_calculate`` via ``get_cycle_dates()``.
    A cycle signal usually fits the selectors that admit on a schedule (e.g. ``SE_Fixed``); see the **cycle** common-parameter note above for the ``SE_Signal`` combination limit.
    The days follow the driving axis: when the instrument has no bar on such a day (a halt, or a calendar day without a bar under ``axis_mode=calendar``), the scheduled buy is not filled and is not shifted to the first bar after the halt.


Manual Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Manual()

    A special SG whose signals are added by hand, for tests or the external signal import. Its
    ``_calculate`` is empty: the preset signals are its input and are kept both when it is used
    directly and as a sub-signal of an operator, assigning the trading object or resetting does not
    clear them.


Signal Operations
^^^^^^^^^^^^^^^^^

An SG supports two kinds of operations: ``+ - * /`` (``SG_Add`` / ``SG_Sub`` / ``SG_Mul`` / ``SG_Div``) and ``& |`` (``SG_And`` / ``SG_Or``).

- The arithmetic operations keep the amplitude of the signals: ``sg * 2`` doubles every daily signal value, and ``sg1 + sg2`` merges the signals of the same day into one net value.
- The logic operations keep the direction only and drop the amplitudes: a hit is written as ``1.0`` for a buy and ``-1.0`` for a sell ( ``(sg1 * 3) & sg2`` gives 1.0, not 3.0). Use the arithmetic operations to keep an amplitude. On a same-day conflict ``|`` lets the sell win.


Custom Signal Generator
-----------------------

Quickly create a custom signal generator that holds no private attributes.

.. py:function:: crtSG(func, params={}, name='crtSG')

    Quickly create a custom signal generator without private attributes.

    :param func: the signal strategy function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom signal generator instance

Example:

.. literalinclude:: ../../examples/quick_crtsg.py

The custom signal generator interface:

* :py:meth:`SignalBase._calculate` - [Required] The computation hook implemented by the subclass
* :py:meth:`SignalBase._clone` - [Required] The clone hook
* :py:meth:`SignalBase._reset` - [Optional] Reset the internal member variables

Example 1 (without private state, the turtle trading strategy):

.. literalinclude:: ../../examples/Turtle_SG.py

Example 2 (with private attributes):

::

    class SignalPython(SignalBase):
        def __init__(self):
            super(SignalPython, self).__init__("SignalPython")
            self._x = 0  # private attribute
            self.setParam("test", 30)

        def _reset(self):
            self._x = 0

        def _clone(self):
            p = SignalPython()
            p._x = self._x
            return p

        def _calculate(self, k):
            self._addBuySignal(Datetime(201201210000))
            self._addSellSignal(Datetime(201201300000))


Signal Generator Base Class
---------------------------

.. py:class:: SignalBase

    The base class for all signal generators.

    .. py:attribute:: name

        The signal generator name

    .. py:method:: __init__(self[, name="SignalBase"])

        :param str name: the signal generator name

    .. py:method:: get_param(self, name)

        Get the value of the specified parameter.

        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: no such parameter

    .. py:method:: set_param(self, name, value)

        Set the value of a parameter.

        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! The parameter type is not supported

    .. py:method:: should_buy(self, datetime)

        Whether a buy signal exists at the specified datetime.

        :param Datetime datetime: the specified datetime
        :rtype: bool

    .. py:method:: should_sell(self, datetime)

        Whether a sell signal exists at the specified datetime.

        :param Datetime datetime: the specified datetime
        :rtype: bool

    .. py:method:: next_time_should_buy(self)

        Whether buying is allowed at the next bar; equivalent to asking whether the last bar indicated a buy.

    .. py:method:: next_time_should_sell(self)

        Whether selling is allowed at the next bar; equivalent to asking whether the last bar indicated a sell.

    .. py:method:: get_buy_signal(self)

        Get the list of all dates flagged as buy signals.

        :rtype: DatetimeList

    .. py:method:: get_sell_signal(self)

        Get the list of all dates flagged as sell signals.

        :rtype: DatetimeList

    .. py:method:: get_buy_value(self, datetime)

        Get the buy signal value at the specified datetime; a value less than or equal to 0 means
        no buy signal.

        :param Datetime datetime: the specified datetime
        :rtype: float

    .. py:method:: get_sell_value(self, datetime)

        Get the sell signal value at the specified datetime; a value greater than or equal to 0
        means no sell signal.

        :param Datetime datetime: the specified datetime
        :rtype: float

    .. py:method:: get_value(self, datetime)

        Get the net signal value at the specified datetime (the buy value plus the sell value):
        positive means a buy intent, negative a sell intent, and about zero means no signal.

        :param Datetime datetime: the specified datetime
        :rtype: float

    .. py:method:: get_cycle_dates(self)

        Get the rebalancing days injected by a MultiSystem (sorted and deduplicated). A custom cycle
        signal reads them in ``_calculate`` to know on which days the scheduled signals must be
        emitted.

        :rtype: DatetimeList

    .. py:method:: set_cycle_dates(self, dates)

        Declare the rebalancing days consumed by the cycle signals, usually injected by a
        MultiSystem before the run; the other signals ignore them. The dates are sorted and
        deduplicated, and the signal is recalculated at once when the trading object is already
        set.

        :param dates: the Datetime sequence (list / tuple / DatetimeList are all accepted)

    .. py:method:: _add_signal(self, datetime, value)

        Add a signal value; called from within _calculate.

        Multiple inputs on the same date are merged into one net value: a positive net keeps only
        the buy signal, a negative net keeps only the sell signal (the existing opposite record of
        that date is dropped), and a net of about zero leaves no signal on that date (the records
        already written for it are removed). ``_add_buy_signal`` and ``_add_sell_signal`` follow
        the same merge rule.

        :param Datetime datetime: the datetime of the signal
        :param float value: the signal value; positive for a buy intent, negative for a sell intent

    .. py:method:: _add_buy_signal(self, datetime[, value=1.0])

        Add a buy signal; called from within _calculate. ``value`` must be greater than 0.

        The value is merged with the same-date signals into one net value: while the net stays
        positive the buy signal is recorded; when it turns negative no buy signal is recorded for
        that date and the existing sell record is updated to the net; a net of about zero removes
        both records of that date.

        :param Datetime datetime: the datetime of the buy signal
        :param float value: the signal value, defaults to 1.0

    .. py:method:: _add_sell_signal(self, datetime[, value=-1.0])

        Add a sell signal; called from within _calculate. ``value`` must be less than 0.

        The value is merged with the same-date signals into one net value: while the net stays
        negative the sell signal is recorded; when it turns positive no sell signal is recorded
        for that date and the existing buy record is updated to the net; a net of about zero
        removes both records of that date.

        :param Datetime datetime: the datetime of the sell signal
        :param float value: the signal value, defaults to -1.0

    .. py:method:: reset(self)

        Reset the generator to its initial state.

    .. py:method:: clone(self)

        Return a clone (copy) of the generator.

    .. py:method:: _calculate(self, kdata)

        [Override hook] The computation hook implemented by the subclass.

    .. py:method:: _reset(self)

        [Override hook] The subclass reset hook, which resets the internal private variables.

    .. py:method:: _clone(self)

        [Override hook] The subclass clone hook.
