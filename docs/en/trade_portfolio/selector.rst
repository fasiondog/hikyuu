.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Selector Algorithm Part|SE
===============================

Implements the algorithms for evaluating and selecting instruments and system strategies.

Common parameters:

    * **get_n** *(int | 0)* : Keep only the first get_n selected systems; when it is less than or equal to 0, all selected systems are kept
    * **depend_on_proto_sys** *(bool |False)* : Requires the prototype (template) systems to be runnable on their own

        ::

            Normally the prototype (template) systems do not take part in the calculation.
            In some special scenarios, however, the selector has to rely on a companion system
            strategy, i.e. a system that the actually executed system mirrors: the actual
            system's behavior can then be viewed as following the buy/sell trades of that
            mirrored (followed) system; for example, relying on the signal generator (SG) to
            make the selection. (Relying on the SG alone is not a rigorous setup, though,
            because the prototype system and the actual system share the same SG.)
            In this case the prototype system must be executed before the SE performs its own
            calculation, so that the SE can use its results. When the actual system and the
            followed system are entirely different, a dedicated SE can be designed.

For selectors based on multi-factor scoring, SE_MultiFactor2 is recommended, as it allows a composite filter to be applied to the cross-sectional score records returned by the MF. See :doc:`scfilter`.

::

    # For an SE_MultiFactor2 instance, set the filter chain: score is not NaN | split into 10 groups and keep group 0 |
    # price is at least 10 yuan | turnover amount is not in the bottom 20% of the daily ranking | keep the top 10
    se.set_scores_filter(SCFilter_IgnoreNan()|SCFilter_Group(10, 0)|SCFilter_Price(
            10.) | SCFilter_AmountLimit(0.2) | SCFilter_TopN(10))



Built-in Selectors
------------------

.. py:function:: SE_Fixed([stk_list, sys])

    The fixed selector: it always selects the initially specified instruments along with their system strategy prototypes
    
    :param list stk_list: the initially specified instruments
    :param System sys: the system strategy prototype
    :return: the SE selector instance

.. py:function:: SE_Signal([stk_list, sys])

    The signal selector: it selects instruments solely according to the buy signals of their systems

    The selection reads the snapshot of the buy signals computed before the run, so it cannot be combined with a cycle signal (e.g. SG_Cycle) under mode C: the cycle day is declared only when a sub-system is admitted, so the snapshot stays empty. Modes A/B push the whole rebalancing schedule down before the snapshot and do combine.

    :param list stk_list: the initially specified instruments
    :param System sys: the system strategy prototype
    :return: the SE selector instance

.. py:function:: SE_MultiFactor(input[, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, spearman=True, mode="MF_ICIRWeight"])

    Create a selector based on multi-factor scoring. It supports several forms of input:

    - Specify the MF directly: ``SE_MultiFactor(mf, topn=10)``
    - Use a FactorSet: ``SE_MultiFactor(factor_set, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight")``
    - Use an Indicator sequence: ``SE_MultiFactor(indicators, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight")``
      
    :param input: the factor input: a MultiFactorBase object, a FactorSet object, or an Indicator sequence
    :param int topn: select only the top topn systems on each cross-section; when it is less than or equal to 0, no limit is applied
    :param int ic_n: the N-day return horizon used in the default IC calculation
    :param int ic_rolling_n: the IC rolling window
    :param Stock ref_stk: the reference security (when unspecified, defaults to sh000001, the Shanghai Composite Index)
    :param bool spearman: when True (the default), use the Spearman correlation coefficient; otherwise use Pearson
    :param str mode: the factor composition algorithm name: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight"
    :return: the SE selector instance

    .. code-block:: python
    
        # From an Indicator list (the legacy way)
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor(indicators, topn=10)
        
        # From a FactorSet (the newer way)
        factor_set = FactorSet("my_factors")
        factor_set.add(MA(CLOSE(), 5))
        factor_set.add(MA(CLOSE(), 10))
        selector2 = SE_MultiFactor(factor_set, topn=10)
        
        # From a pre-created MultiFactor object, passed directly
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor(mf, topn=10)

.. py:function:: SE_MultiFactor2(input[, ic_n=5, ic_rolling_n=120, ref_stk=None, spearman=True, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan()])

    Create a selector based on multi-factor scoring; it supports several forms of input. See :doc:`scfilter`.

    - Specify the MF directly: ``SE_MultiFactor2(mf, filter)``
    - Use a FactorSet: ``SE_MultiFactor2(factor_set, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan())``
    - Use an Indicator sequence: ``SE_MultiFactor2(indicators, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan())``

    SE_MultiFactor2 exposes an additional parameter, "mf_recover_type", which is set via set_param. It forces the MF to be calculated with a fixed recovery (price-adjustment) type, e.g. a fixed forward adjustment.

    :param input: the factor input: a MultiFactorBase object, a FactorSet object, or an Indicator sequence
    :param int ic_n: the N-day return horizon used in the default IC calculation
    :param int ic_rolling_n: the IC rolling window
    :param Stock ref_stk: the reference security (when unspecified, defaults to sh000001, the Shanghai Composite Index)
    :param bool spearman: when True (the default), use the Spearman correlation coefficient; otherwise use Pearson
    :param str mode: the factor composition algorithm name: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight"
    :param ScoresFilter filter: the cross-sectional scores filter
    :return: the SE selector instance

    .. code-block:: python
    
        # From an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor2(indicators)
        
        # From a FactorSet
        factor_set = FactorSet("my_factors")
        factor_set.add(MA(CLOSE(), 5))
        factor_set.add(MA(CLOSE(), 10))
        selector2 = SE_MultiFactor2(factor_set)
        
        # From a pre-created MultiFactor object, passed directly
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor2(mf)

Multi-Factor Selector v1 vs v2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``SE_MultiFactor`` keeps the legacy interface, while ``SE_MultiFactor2`` offers more flexible control
through a composable score-filter chain. The main differences:

.. list-table::
   :header-rows: 1
   :widths: 18 41 41

   * - Aspect
     - SE_MultiFactor (v1)
     - SE_MultiFactor2 (recommended)
   * - Score filtering
     - ``ignore_null`` (True by default), ``ignore_le_zero`` (False by default); extra filters may be appended with ``add_scores_filter``
     - A filter chain, defaulting to ``SCFilter_IgnoreNan()``; combine with ``set_scores_filter`` / ``add_scores_filter``
   * - Truncation
     - The ``topn`` parameter, 10 by default; ``<=0`` means no limit
     - No ``topn``; truncated by ``get_n``, or controlled by the filters
   * - Ordering
     - ``reverse=False`` takes the top N by the highest score; ``True`` takes the last N with the lowest scores
     - Keeps the cross-sectional order produced by the MF; no ``reverse``
   * - Buy-signal filtering
     - ``only_should_buy=True`` keeps only the systems issuing a buy signal on that datetime
     - Requires a custom filter
   * - Recovery type
     - ``mf_recover_type``; with a given MF, ``keep_mf_recover_type`` keeps its original query recovery type
     - ``mf_recover_type`` only; always overrides the query recovery type of a given MF

.. note:: ``topn`` / ``ignore_null`` / ``ignore_le_zero`` / ``only_should_buy`` / ``reverse`` exist only in v1; v2 expresses the same semantics with the filter chain plus ``get_n``.

Custom Selector Strategy
------------------------

The selector strategy interface for customization consists of the following override hooks:

* :py:meth:`SelectorBase.get_selected` - [Required] Return the list of system instances at the specified datetime
* :py:meth:`SelectorBase._calculate` - [Required] The calculation hook
* :py:meth:`SelectorBase._reset` - [Optional] Reset private attributes
* :py:meth:`SelectorBase._clone` - [Required] The clone hook; it must return a new instance of the same Python subclass

``_clone`` is invoked by the base ``clone()`` through the trampoline, and the C++ side keeps using the
returned value as a ``SelectorBase`` instance. It must therefore build and return a new object of the same
Python subclass (usually by copying the private attributes); returning anything that is not a ``SelectorBase``
subclass instance (e.g. ``None``) raises a conversion error.

Selector Strategy Base Class
----------------------------

.. py:class:: SelectorBase

    The selector strategy base class, which implements the algorithms for evaluating and selecting instruments and system strategies
    
    .. py:attribute:: name The selector name

    .. py:attribute:: proto_sys_list The list of prototype (template) systems

    .. py:attribute:: real_sys_list The list of actual systems at runtime
    
    .. py:method:: __init__(self[, name="SelectorBase])
    
        Constructor.
        
        :param str name: the selector name
        
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
        :raises logic_error: Unsupported type! The parameter type is not supported  

    .. py:method:: reset(self)
    
        Reset the selector.
    
    .. py:method:: clone(self)
    
        Return a clone of the selector. The runtime selection cache is not cloned: calculate again before selecting.
        
    .. py:method:: add_stock(self, stock, sys)

        Add an initial instrument and its corresponding system strategy prototype.
        
        :param Stock stock: the initial instrument to add
        :param System sys: the system strategy prototype

    .. py:method:: add_stock_list(self, stk_list, sys)
    
        Add a list of initial instruments together with their shared system strategy prototype.
        
        :param StockList stk_list: the list of initial instruments to add
        :param System sys: the system strategy prototype
    
    .. py:method:: remove_all(self)
    
        Remove all prototype systems that have been added.

    .. py:method:: set_scores_filter(self, scfilter)

        Set the ScoresFilter, replacing the existing filter; applicable only to SE_MultiFactor
    
        :param ScoresFilter filter: ScoresFilter

    .. py:method:: add_scores_filter(self, scfilter)

        Append an additional filter on top of the existing one; applicable only to SE_MultiFactor

        :param ScoresFilter filter: the new filter    


    .. py:method:: get_selected(self, datetime)
    
        [Override hook] Return the selected system instances at the specified datetime.
        
        :param Datetime datetime: the specified datetime
        :return: the list of selected system instances
        :rtype: SystemList


     .. py:method:: _calculate(self)

        [Override hook] The subclass calculation hook.

     .. py:method:: _reset(self)
    
        [Override hook] The subclass reset hook, used to reset internal private state.
    
    .. py:method:: _clone(self)

        [Override hook] The subclass clone hook; it must return a new instance of the same Python subclass
    
    
    
