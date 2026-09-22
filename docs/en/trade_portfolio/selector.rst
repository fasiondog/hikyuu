.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Selector Algorithm Component|SE
===============================

Implements the algorithm for evaluating and selecting the targets and the system strategies.

Common parameters:

    * **get_n** *(int | 0)* : Only take the first get_n selected systems; when it is less than or equal to 0, take all
    * **depend_on_proto_sys** *(bool |False)* : The prototype systems need to be able to run independently

        ::

            Usually the prototype systems do not participate in the calculation, but in some special scenarios
            it is necessary to depend on the companion system strategy; in this case the behavior of the actually
            executed system can be considered to follow the buy/sell trades of the companion system, e.g. relying
            on the SG for the selection (however, since the scenario relying only on the SG is not rigorous,
            because the SG of the prototype system and the actual system is the same)
            In this case, the prototype system needs to be executed before its own calculation, and then the SE can use it.
            For the case where the actual system and the followed system are completely different, a special SE can be designed.

For the multi-factor scoring based selectors, it is recommended to use SE_MultiFactor2, which allows adding a combined filter for the cross-sectional scoring records returned by the MF. See: :doc:`scfilter`

::

    # For an SE_MultiFactor2 instance, set the filter: the score is not Nan | split into 10 groups and take group 0 |
    # the price is greater than or equal to 10 yuan | the amount is not in the last 20% of the daily ranking | take the top 10
    se.set_scores_filter(SCFilter_IgnoreNan()|SCFilter_Group(10, 0)SCFilter_Price(
            10.) | SCFilter_AmountLimit(0.2) | SCFilter_TopN(10))



Built-in Selectors
------------------

.. py:function:: SE_Fixed([stk_list, sys])

    The fixed selector, i.e. always selecting the initially defined targets and their system strategy prototypes
    
    :param list stk_list: the initially defined targets
    :param System sys: the system strategy prototype
    :return: the SE selector instance

.. py:function:: SE_Signal([stk_list, sys])

    The signal selector, selecting only by the system buy signals
    
    :param list stk_list: the initially defined targets
    :param System sys: the system strategy prototype
    :return: the SE selector instance

.. py:function:: SE_MultiFactor(input[, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, spearman=True, mode="MF_ICIRWeight"])

    Create a multi-factor scoring based selector, supporting several creation ways:

    - Specify the MF directly: ``SE_MultiFactor(mf, topn=10)``
    - Use a FactorSet: ``SE_MultiFactor(factor_set, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight")``
    - Use an Indicator sequence: ``SE_MultiFactor(indicators, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight")``
      
    :param input: the factor input, which can be a MultiFactorBase object, a FactorSet object or an Indicator sequence
    :param int topn: only select the first topn systems in the cross-section; when it is less than or equal to 0, there is no limit
    :param int ic_n: the N-day return corresponding to the default IC
    :param int ic_rolling_n: the IC rolling period
    :param Stock ref_stk: the reference security (when unspecified, defaults to sh000300, the CSI 300)
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" the factor composition algorithm name
    :return: the SE selector instance

    .. code-block:: python
    
        # Use an Indicator list (the original way)
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor(indicators, topn=10)
        
        # Use a FactorSet (the new way)
        factor_set = FactorSet("my_factors")
        factor_set.add(MA(CLOSE(), 5))
        factor_set.add(MA(CLOSE(), 10))
        selector2 = SE_MultiFactor(factor_set, topn=10)
        
        # Use the pre-created MultiFactor object directly
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor(mf, topn=10)

.. py:function:: SE_MultiFactor2(input[, ic_n=5, ic_rolling_n=120, ref_stk=None, spearman=True, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan()])

    Create a multi-factor scoring based selector, supporting several creation ways. :doc:`scfilter`

    - Specify the MF directly: ``SE_MultiFactor2(mf, filter)``
    - Use a FactorSet: ``SE_MultiFactor2(factor_set, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan())``
    - Use an Indicator sequence: ``SE_MultiFactor2(indicators, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan())``

    SE_MultiFactor2 has a separate parameter "mf_recover_type", set with set_param. It specifies that the mf uses a fixed recovery type to calculate, e.g. a fixed forward adjustment.

    :param input: the factor input, which can be a MultiFactorBase object, a FactorSet object or an Indicator sequence
    :param int ic_n: the N-day return corresponding to the default IC
    :param int ic_rolling_n: the IC rolling period
    :param Stock ref_stk: the reference security (when unspecified, defaults to sh000300, the CSI 300)
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" the factor composition algorithm name
    :param ScoresFilter filter: the scores filter
    :return: the SE selector instance

    .. code-block:: python
    
        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor2(indicators)
        
        # Use a FactorSet
        factor_set = FactorSet("my_factors")
        factor_set.add(MA(CLOSE(), 5))
        factor_set.add(MA(CLOSE(), 10))
        selector2 = SE_MultiFactor2(factor_set)
        
        # Use the pre-created MultiFactor object directly
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor2(mf)
        
Custom Selector Strategy
------------------------

The custom selector strategy interface:

* :py:meth:`SelectorBase.is_match_af` - [Required] Judge whether it matches the AF
* :py:meth:`SelectorBase.get_selected` - [Required] Get the list of the system instances at the specified moment
* :py:meth:`SelectorBase._calculate` - [Required] The calculation interface
* :py:meth:`SelectorBase._reset` - [Optional] Reset the private attributes
* :py:meth:`SelectorBase._clone` - [Required] The clone interface

Selector Strategy Base Class
----------------------------

.. py:class:: SelectorBase

    The selector strategy base class, implementing the algorithm for evaluating and selecting the targets and the system strategies
    
    .. py:attribute:: name Name

    .. py:attribute:: proto_sys_list The prototype system list

    .. py:attribute:: real_sys_list The actual system list at runtime
    
    .. py:method:: __init__(self[, name="SelectorBase])
    
        The initialization constructor
        
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

    .. py:method:: reset(self)
    
        The reset operation
    
    .. py:method:: clone(self)
    
        The clone operation        
        
    .. py:method:: add_stock(self, stock, sys)

        Add the initial target and its corresponding system strategy prototype
        
        :param Stock stock: the initial target to add
        :param System sys: the system strategy prototype

    .. py:method:: add_stock_list(self, stk_list, sys)
    
        Add the initial target list and its system strategy prototype
        
        :param StockList stk_list: the initial target list to add
        :param System sys: the system strategy prototype
    
    .. py:method:: remove_all(self)
    
        Remove all the added prototype systems

    .. py:method:: set_scores_filter(self, scfilter)

        Set the ScoresFilter, which will replace the existing filter; only applicable to SE_MultiFactor
    
        :param ScoresFilter filter: ScoresFilter

    .. py:method:: add_scores_filter(self, scfilter)

        Add a new filter on top of the existing one; only applicable to SE_MultiFactor

        :param ScoresFilter filter: the new filter    

    .. py:method:: is_match_af(self)

        [Overload interface] Judge whether it matches the AF

        :param AllocateFundsBase af: the asset allocation algorithm


    .. py:method:: get_selected(self, datetime)
    
        [Overload interface] Get the selected system instances at the specified moment
        
        :param Datetime datetime: the specified moment
        :return: the list of the selected system instances
        :rtype: SystemList


     .. py:method:: _calculate(self)

        [Overload interface] The subclass calculation interface

     .. py:method:: _reset(self)
    
        [Overload interface] The subclass reset interface, resetting the internal private variables
    
    .. py:method:: _clone(self)
    
        [Overload interface] The subclass clone interface    
    
    
    
