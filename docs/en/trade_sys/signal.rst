.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Signal Generator|SG
===================

The signal generator is responsible for generating the buy and sell signals.

Common parameters:

    * **alternate** *(bool|True)* : Whether the buy and sell signals appear alternately. The single-line signals usually judge the generation of the signals by the inflection points, the slope, etc.; in this case, consecutive buy signals or consecutive sell signals may appear, and this parameter can be used to control whether the buy and sell signals appear alternately. The two-line crossover signals usually have the buy and sell already appearing alternately, so this parameter is invalid in that case.
    * **cycle** *(bool|False)* : Used with PF, calculated only within the PF rebalance cycle
    * **support_borrow_stock** *(bool|False)* : Support issuing short signals


General Signal Generators
-------------------------

When technical indicators are usually used to judge buying and selling, it is based on the crossover of the fast line and the slow line, or the inflection point of a single curve. The general signal generators below are enough for most cases.

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
                <td>Two-line crossover indicator</td>
                <td>When the fast line crosses the slow line from below upward, buy;<br>when the fast line crosses the slow line from above downward, sell.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_CrossGold</a></td>
                <td>Golden cross indicator</td>
                <td>A golden cross is when the fast line crosses the slow line from below upward and both the fast line and the slow line point upward, buy;<br>when the fast line crosses the slow line from above downward and both the fast line and the slow line point downward, it is a death cross, sell.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Single</a></td>
                <td>Single-line inflection point signal generator</td>
                <td>Generate a single-line inflection point signal generator. Use the curve inflection point algorithm given in the book "Smarter Trading" to judge the curve trend</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Single2</a></td>
                <td>Single-line inflection point signal generator 2</td>
                <td>Generate a single-line inflection point signal generator. Use the curve inflection point algorithm given in the book "Smarter Trading" to judge the curve trend</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Flex</a></td>
                <td>Self-crossover single-line inflection point indicator</td>
                <td>Use its own EMA(slow_n) as the slow line, and itself as the fast line.<br>Buy when the fast line crosses the slow line upward,<br>sell when the fast line crosses the slow line downward.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Bool</a></td>
                <td>Boolean signal generator</td>
                <td>Use Indicators whose operation results are bool-array-like as the buy and sell indicators respectively.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_OneSide</a></td>
                <td>One-side signal generator</td>
                <td>Build a one-side signal (containing only the buy signal or only the sell signal) from the input indicator,<br>if the indicator value is greater than 0, add the signal</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Buy</a></td>
                <td>One-side buy signal generator</td>
                <td>The simplified mode of SG_OneSide</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Sell</a></td>
                <td>One-side sell signal generator</td>
                <td>The simplified mode of SG_OneSide</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Band</a></td>
                <td>Range breakout signal generator</td>
                <td>The indicator range indicator; when the indicator exceeds the upper band, buy;<br>when the indicator falls below the lower band, sell.</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_AllwaysBuy</a></td>
                <td>Always-buy signal generator</td>
                <td>A special SG that issues a buy signal every day continuously, usually used with PF</td>
            </tr>
            <tr>
                <td><a href="#target-section">SG_Cycle</a></td>
                <td>PF rebalance cycle buy signal generator</td>
                <td>A special SG, used with PF, taking the PF rebalance cycle as the buy signal</td>
            </tr>
            <tr>
                <td>SG_Add<br>SG_Mul<br>SG_Sub<br>SG_Div</td>
                <td>SG operation helpers</td>
                <td>Since the alternate of SG defaults to True, when using a form like "sg1 + sg2 + sg3", it is easy to ignore the alternate attribute of sg1 + sg2<br>It is recommended to use SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem</td>
            </tr>        
        </tbody>
    </table>
    <p></p>


Two-line Crossover Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Cross(fast, slow)
    
    The two-line crossover indicator; when the fast line crosses the slow line from below upward, buy; when the fast line crosses the slow line from above downward, sell. E.g.: buy when the 5-day MA crosses the 10-day MA upward, sell when the 5-day MA crosses the 10-day MA downward:: 

        SG_Cross(MA(CLOSE(), n=10), MA(CLOSE(), n=30))

    :param Indicator fast: the fast line
    :param Indicator slow: the slow line
    :return: the signal generator
        
        
Golden Cross Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_CrossGold(fast, slow)

    The golden cross indicator; a golden cross is when the fast line crosses the slow line from below upward and both the fast line and the slow line point upward, buy;
    when the fast line crosses the slow line from above downward and both the fast line and the slow line point downward, it is a death cross, sell.::
    
        SG_CrossGold(MA(CLOSE(), n=10), MA(CLOSE(), n=30))
    
    :param Indicator fast: the fast line
    :param Indicator slow: the slow line
    :return: the signal generator    


Single-line Inflection Point Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Single(ind[, filter_n = 10, filter_p = 0.1])
    
    Generate a single-line inflection point signal generator. Use the curve inflection point algorithm given in the book "Smarter Trading" [BOOK1]_ to judge the curve trend; the formula is as follows::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - AMA[1] > filter
        or Buy When AMA - AMA[2] > filter
        or Buy When AMA - AMA[3] > filter 
    
    :param Indicator ind:
    :param int filter_n: the N-day period
    :param float filter_p: the filter percentage
    :return: the signal generator
    
.. py:function:: SG_Single2(ind[, filter_n = 10, filter_p = 0.1])
    
    Generate the single-line inflection point signal generator 2 [BOOK1]_::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - @lowest(AMA,n) > filter
        Sell When @highest(AMA, n) - AMA > filter
    
    :param Indicator ind:
    :param int filter_n: the N-day period
    :param float filter_p: the filter percentage
    :return: the signal generator
   
Self-crossover Single-line Inflection Point Indicator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Flex(ind, slow_n)

    Use its own EMA(slow_n) as the slow line and itself as the fast line; buy when the fast line crosses the slow line upward, and sell when the fast line crosses the slow line downward.

    :param Indicator ind:
    :param int slow_n: the period of the slow line EMA
    :return: the signal generator


Boolean Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Bool(buy, sell[, alternate=True])

    The boolean signal generator; use Indicators whose operation results are bool-array-like as the buy and sell indicators respectively.
    
    :param Indicator buy: the buy indicator (a position > 0 in the result Indicator means buy)
    :param Indicator sell: the sell indicator (a position > 0 in the result Indicator means sell)
    :param bool alternate: whether to buy and sell alternately, defaults to True
    :return: the signal generator


One-side Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_OneSide(ind, is_buy)
          
    Build a one-side signal (containing only the buy signal or only the sell signal) from the input indicator; if the indicator value is greater than 0, add the signal
    
    :param Indicator ind: the input indicator
    :param bool is_buy: build a buy signal, otherwise a sell signal


.. py:function:: SG_Buy(ind)

    The one-side buy signal, a simplification of SG_OneSide

    :param Indicator ind: the input indicator
    :return: the signal generator


.. py:function:: SG_Sell(ind)

    The one-side sell signal, a simplification of SG_OneSide

    :param Indicator ind: the input indicator
    :return: the signal generator


Range Breakout Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Band(ind, lower, upper)
          
    The indicator range indicator; when the indicator exceeds the upper band, buy;
    when the indicator falls below the lower band, sell.
    
    ::

        SG_Band(MA(C, n=10), 100, 200)
        SG_Band(CLOSE, MA(LOW), MA(HIGH))


Always-buy Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_AllwaysBuy()
    
    A special SG that issues a buy signal every day continuously, usually used with PF


PF Position Adjustment Period Buy Signal Generator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. py:function:: SG_Cycle()
    
    A special SG, used with PF, taking the PF rebalance cycle as the buy signal


Custom Signal Generator
-----------------------

Quickly create a custom signal generator without private attributes.

.. py:function:: crtSG(func, params={}, name='crtSG')

    Quickly create a custom signal generator without private attributes
    
    :param func: the signal strategy function
    :param {} params: the parameter dictionary
    :param str name: the custom name
    :return: the custom signal generator instance
    
Example:

.. literalinclude:: ../../examples/quick_crtsg.py      

The custom signal generator interface:

* :py:meth:`SignalBase._calculate` - [Required] The subclass calculation interface
* :py:meth:`SignalBase._clone` - [Required] The clone interface
* :py:meth:`SignalBase._reset` - [Optional] Reset the internal member variables

Example 1 (without private variables, the turtle trading strategy):

.. literalinclude:: ../../examples/Turtle_SG.py                
                
Example 2 (with private attributes):

::

    class SignalPython(SignalBase):
        def __init__(self):
            super(SignalPython, self).__init__("SignalPython")
            self._x = 0 # private attribute
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

    The signal generator base class
    
    .. py:attribute:: name Name
    
    .. py:method:: __init__(self[, name="SignalBase"])
    
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
                
    .. py:method:: should_buy(self, datetime)
    
        Whether it can be bought at the specified moment
    
        :param Datetime datetime: the specified moment
        :rtype: bool
    
    .. py:method:: should_sell(self, datetime)
    
        Whether it can be sold at the specified moment
        
        :param Datetime datetime: the specified moment
        :rtype: bool

    .. py:method:: next_time_should_buy(self)

        Whether it can be bought at the next moment, equivalent to whether the last moment indicated a buy

    .. py:method:: next_time_should_sell(self)

        Whether it can be sold at the next moment, equivalent to whether the last moment indicated a sell
    
    .. py:method:: get_buy_signal(self)
    
        Get the list of all the buy indication dates
        
        :rtype: DatetimeList
    
    .. py:method:: get_sell_signal(self)
    
        Get the list of all the sell indication dates
        
        :rtype: DatetimeList
    
    .. py:method:: _add_buy_signal(self, datetime)
    
        Add a buy signal, called in _calculate
        
        :param Datetime datetime: the date indicating the buy
    
    .. py:method:: _add_sell_signal(self, datetime)
    
        Add a sell signal, called in _calculate

        :param Datetime datetime: the date indicating the sell
        
    .. py:method:: reset(self)
    
        The reset operation
    
    .. py:method:: clone(self)
    
        The clone operation
    
    .. py:method:: _calculate(self, kdata)
    
        [Override hook] The subclass calculation interface
    
    .. py:method:: _reset(self)
    
        [Override hook] The subclass reset interface, resetting the internal private variables
    
    .. py:method:: _clone(self)
    
        [Override hook] The subclass clone interface
