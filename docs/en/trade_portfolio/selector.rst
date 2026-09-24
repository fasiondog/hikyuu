.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Selector (SE)
=============

The selector (SE) implements the logic for evaluating and choosing instruments and the trading systems applied to them.

Common parameters:

    * **get_n** *(int | 0)* : Take only the first get_n selected systems; when it is less than or equal to 0, all selected systems are taken
    * **depend_on_proto_sys** *(bool |False)* : The prototype systems must be able to run independently

        ::

            Normally the prototype (template) systems do not take part in the calculation.
            In certain special scenarios, however, the selector needs to depend on a
            mirrored (followed) system strategy: the system that is actually executed can
            be regarded as mirroring the buy/sell trades of that followed system; for
            example, basing the selection on its signal generator. (Relying on the SG alone
            is not a rigorous example here, because the prototype system and the actual
            system share the same SG anyway.)
            In this mode, the prototype systems are executed before the selector's own
            calculation, so that the SE can make use of their results while selecting.
            When the actual system and the system it follows are two entirely different
            strategies, a specialized SE can be designed for that case.

For selectors based on multi-factor scores, SE_MultiFactor2 is recommended: it allows a composite filter to be attached to the cross-sectional score records returned by the MF. See: :doc:`scfilter`

::

    # For an SE_MultiFactor2 instance, configure the filter chain: the score is not NaN |
    # split into 10 groups and take group 0 | the price is at least 10 | the traded amount is
    # not in the bottom 20% of that day's ranking | take the top 10
    se.set_scores_filter(SCFilter_IgnoreNan()|SCFilter_Group(10, 0)SCFilter_Price(
            10.) | SCFilter_AmountLimit(0.2) | SCFilter_TopN(10))



Built-in Selectors
------------------

.. py:function:: SE_Fixed([stk_list, sys])

    The fixed selector: it always selects the initially specified instruments together with their system prototypes

    :param list stk_list: initially specified instruments
    :param System sys: trading-system prototype
    :return: selector (SE) instance

.. py:function:: SE_Signal([stk_list, sys])

    The signal-based selector: an instrument is selected solely on the buy signals of its system

    :param list stk_list: initially specified instruments
    :param System sys: trading-system prototype
    :return: selector (SE) instance

.. py:function:: SE_MultiFactor(input[, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, spearman=True, mode="MF_ICIRWeight"])

    Create a selector based on multi-factor scoring. It can be constructed in several ways:

    - Pass the MF directly: ``SE_MultiFactor(mf, topn=10)``
    - From a FactorSet: ``SE_MultiFactor(factor_set, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight")``
    - From a sequence of Indicators: ``SE_MultiFactor(indicators, topn=10, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight")``

    :param input: factor input; it can be a MultiFactorBase object, a FactorSet object, or a sequence of Indicators
    :param int topn: select only the first topn systems in each cross-section; when it is less than or equal to 0, no limit is applied
    :param int ic_n: the N-day forward return used by the default IC calculation
    :param int ic_rolling_n: the IC rolling window length
    :param Stock ref_stk: the reference security (defaults to sh000300, the CSI 300, when unspecified)
    :param bool spearman: when true (the default), the correlation is computed with Spearman's rank correlation; otherwise Pearson correlation is used
    :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" — the factor composition algorithm
    :return: selector (SE) instance

    .. code-block:: python

        # From a list of Indicators (the original usage)
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor(indicators, topn=10)

        # From a FactorSet (the newer usage)
        factor_set = FactorSet("my_factors")
        factor_set.add(MA(CLOSE(), 5))
        factor_set.add(MA(CLOSE(), 10))
        selector2 = SE_MultiFactor(factor_set, topn=10)

        # Pass a pre-built MultiFactor object directly
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor(mf, topn=10)

.. py:function:: SE_MultiFactor2(input[, ic_n=5, ic_rolling_n=120, ref_stk=None, spearman=True, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan()])

    Create a selector based on multi-factor scoring. It can be constructed in several ways. See :doc:`scfilter`.

    - Pass the MF directly: ``SE_MultiFactor2(mf, filter)``
    - From a FactorSet: ``SE_MultiFactor2(factor_set, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan())``
    - From a sequence of Indicators: ``SE_MultiFactor2(indicators, ic_n=5, ic_rolling_n=120, ref_stk=None, mode="MF_ICIRWeight", filter=SCFilter_IgnoreNan())``

    SE_MultiFactor2 additionally exposes a dedicated parameter, "mf_recover_type", which is set through set_param. It pins the MF to a fixed recovery (price-adjustment) type when computing scores, for example always using forward adjustment.

    :param input: factor input; it can be a MultiFactorBase object, a FactorSet object, or a sequence of Indicators
    :param int ic_n: the N-day forward return used by the default IC calculation
    :param int ic_rolling_n: the IC rolling window length
    :param Stock ref_stk: the reference security (defaults to sh000300, the CSI 300, when unspecified)
    :param bool spearman: when true (the default), the correlation is computed with Spearman's rank correlation; otherwise Pearson correlation is used
    :param str mode: "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" — the factor composition algorithm
    :param ScoresFilter filter: the score-record filter
    :return: selector (SE) instance

    .. code-block:: python

        # From a list of Indicators
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        selector1 = SE_MultiFactor2(indicators)

        # From a FactorSet
        factor_set = FactorSet("my_factors")
        factor_set.add(MA(CLOSE(), 5))
        factor_set.add(MA(CLOSE(), 10))
        selector2 = SE_MultiFactor2(factor_set)

        # Pass a pre-built MultiFactor object directly
        mf = MF_ICIRWeight(factor_set, stocks, query)
        selector3 = SE_MultiFactor2(mf)

Implementing a Custom Selector
------------------------------

The interface for implementing a custom selector strategy:

* :py:meth:`SelectorBase.is_match_af` - [Required] Check whether the selector is compatible with the AF
* :py:meth:`SelectorBase.get_selected` - [Required] Get the list of system instances selected at the specified datetime
* :py:meth:`SelectorBase._calculate` - [Required] The calculation entry point
* :py:meth:`SelectorBase._reset` - [Optional] Reset the subclass-private state
* :py:meth:`SelectorBase._clone` - [Required] The clone entry point

Selector Base Class
-------------------

.. py:class:: SelectorBase

    Base class for selector strategies; implements the logic for evaluating and selecting instruments and trading systems

    .. py:attribute:: name Name

    .. py:attribute:: proto_sys_list List of prototype (template) systems

    .. py:attribute:: real_sys_list List of the actual systems at run time

    .. py:method:: __init__(self[, name="SelectorBase])

        Constructor

        :param str name: the name

    .. py:method:: get_param(self, name)

        Get the value of the specified parameter

        :param str name: the parameter name
        :return: the parameter value
        :raises out_of_range: raised if no such parameter exists

    .. py:method:: set_param(self, name, value)

        Set a parameter

        :param str name: the parameter name
        :param value: the parameter value
        :type value: int | bool | float | string
        :raises logic_error: Unsupported type! Raised when the parameter value type is not supported

    .. py:method:: reset(self)

        Reset the selector to its initial state

    .. py:method:: clone(self)

        Return a clone of the selector

    .. py:method:: add_stock(self, stock, sys)

        Add an initial instrument together with its corresponding trading-system prototype

        :param Stock stock: the initial instrument to add
        :param System sys: the trading-system prototype

    .. py:method:: add_stock_list(self, stk_list, sys)

        Add a list of initial instruments sharing one trading-system prototype

        :param StockList stk_list: the initial instrument list to add
        :param System sys: the trading-system prototype

    .. py:method:: remove_all(self)

        Remove all previously added prototype systems

    .. py:method:: set_scores_filter(self, scfilter)

        Set the ScoresFilter, replacing the filter currently in place; applicable only to SE_MultiFactor selectors

        :param ScoresFilter filter: the ScoresFilter

    .. py:method:: add_scores_filter(self, scfilter)

        Append a new filter on top of the existing filter chain; applicable only to SE_MultiFactor selectors

        :param ScoresFilter filter: the new filter

    .. py:method:: is_match_af(self)

        [Override hook] Check whether the selector is compatible with the AF

        :param AllocateFundsBase af: the fund allocation algorithm


    .. py:method:: get_selected(self, datetime)

        [Override hook] Get the system instances selected at the specified datetime

        :param Datetime datetime: the specified datetime
        :return: the list of selected system instances
        :rtype: SystemList


    .. py:method:: _calculate(self)

        [Override hook] The subclass calculation hook

    .. py:method:: _reset(self)

        [Override hook] The subclass reset hook; reset the internal private variables

    .. py:method:: _clone(self)

        [Override hook] The subclass clone hook
