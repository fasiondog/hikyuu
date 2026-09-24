.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Signal Generator (SG)
=====================

A signal generator (SG) produces the buy and sell signals used by a trading system.

Common parameters:

    * **alternate** *(bool|True)* : Whether buy and sell signals must alternate. Single-line generators typically derive their signals from curve inflection points, slope changes, and similar rules, so they can emit several consecutive buy signals or several consecutive sell signals; this parameter forces the two to alternate. Two-line crossover generators already alternate buys and sells by construction, in which case this parameter has no effect.
    * **cycle** *(bool|False)* : Use together with a PF: signals are evaluated only on the PF rebalance cycle
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
                <td>A special SG for use with a PF, treating the PF rebalance cycle as its buy signals</td>
            </tr>
            <tr>
                <td>SG_Add<br>SG_Mul<br>SG_Sub<br>SG_Div</td>
                <td>SG operation helpers</td>
                <td>Because alternate defaults to True for an SG, chaining a form such as "sg1 + sg2 + sg3" can silently carry over the alternate setting of sg1 + sg2<br>It is recommended to use SG_Add(sg1, sg2, False) + sg3 to avoid the alternation problem</td>
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

.. py:function:: SG_Single(ind[, filter_n = 10, filter_p = 0.1])

    Creates a single-line inflection-point signal generator. It judges the trend of the curve with the inflection-point algorithm given in the book "Smarter Trading" [BOOK1]_. The rules are as follows::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - AMA[1] > filter
        or Buy When AMA - AMA[2] > filter
        or Buy When AMA - AMA[3] > filter

    :param Indicator ind: the input indicator curve
    :param int filter_n: the lookback period N, in bars
    :param float filter_p: the filter percentage
    :return: the signal generator

.. py:function:: SG_Single2(ind[, filter_n = 10, filter_p = 0.1])

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

    A special SG for use with a PF, treating the PF rebalance cycle as its buy signals.


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

    .. py:method:: _add_buy_signal(self, datetime)

        Add a buy signal; called from within _calculate.

        :param Datetime datetime: the datetime of the buy signal

    .. py:method:: _add_sell_signal(self, datetime)

        Add a sell signal; called from within _calculate.

        :param Datetime datetime: the datetime of the sell signal

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
