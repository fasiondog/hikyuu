.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Multi-factor Composition|MF
===========================

A multi-factor model essentially scores the candidate instruments on each cross-section, so in practice it must be used together with a Selector (the strategy selection algorithm).

For the standardization and neutralization used when calculating factors, see: :doc:`normalize`

Common parameters:

    * **fill_null** *(bool|True)*: whether missing data is filled with NaN; otherwise the most recent available value is carried forward
    * **ic_n** *(int|5)*: the N-day forward return used when calculating the IC of the composed factor
    * **spearman** *(bool|True)*: use Spearman rank correlation; otherwise Pearson
    * **mode** *(int|2)*: the sorting mode for the cross-section data: 0 descending, 1 ascending, 2 no sorting
    * **save_all_factors** *(bool|False)*: whether to retain all factor values; affects the get_actor/get_all_factors methods


Built-in Factor Composition Algorithms
--------------------------------------

.. py:function:: MF_Weight(input, stks, weights, query, ref_stk[, ic_n=5, spearman=True, mode=0, save_all_factors=False])

    Combine the input factors using the specified weights: ind1 * weight1 + ind2 * weight2 + ... + indn * weightn. Several input types are supported.

    :param input: the factor input, which can be a FactorSet object or a sequence of Indicators
    :param sequence(stock) stks: the list of securities to calculate
    :param sequence(float) weights: the weight list (must be the same length as the number of factors)
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for date alignment (defaults to sh000001 when unspecified)
    :param int ic_n: the N-day forward return corresponding to the default IC
    :param bool spearman: calculate the correlation coefficient with Spearman by default; otherwise Pearson
    :param int mode: the sorting mode for the cross-section data: 0 descending, 1 ascending, 2 no sorting
    :param bool save_all_factors: whether to retain all factor values; affects the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python

        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        weights = [0.6, 0.4]
        mf1 = MF_Weight(indicators, stocks, weights, query)

        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_Weight(factor_set, stocks, weights, query)


.. py:function:: MF_EqualWeight(input, stks, query, ref_stk[, ic_n=5])

    Combine the input factors with equal weights. Several input types are supported.

    :param input: the factor input, which can be a FactorSet object or a sequence of Indicators
    :param sequence(stock) stks: the list of securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for date alignment (defaults to sh000001 when unspecified)
    :param int ic_n: the N-day forward return corresponding to the default IC
    :param bool spearman: calculate the correlation coefficient with Spearman by default; otherwise Pearson
    :param int mode: the sorting mode for the cross-section data: 0 descending, 1 ascending, 2 no sorting
    :param bool save_all_factors: whether to retain all factor values; affects the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python

        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_EqualWeight(indicators, stocks, query)

        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_EqualWeight(factor_set, stocks, query)


.. py:function:: MF_ICWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    Combine the input factors using rolling IC weights. Several input types are supported.

    :param input: the factor input, which can be a FactorSet object or a sequence of Indicators
    :param sequence(stock) stks: the list of securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for date alignment (defaults to sh000001 when unspecified)
    :param int ic_n: the N-day forward return corresponding to the default IC
    :param int ic_rolling_n: the rolling window for the IC
    :param bool spearman: calculate the correlation coefficient with Spearman by default; otherwise Pearson
    :param int mode: the sorting mode for the cross-section data: 0 descending, 1 ascending, 2 no sorting
    :param bool save_all_factors: whether to retain all factor values; affects the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python

        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICWeight(indicators, stocks, query)

        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_ICWeight(factor_set, stocks, query)


.. py:function:: MF_ICIRWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    Combine the input factors using rolling ICIR weights. Several input types are supported.

    :param input: the factor input, which can be a FactorSet object or a sequence of Indicators
    :param sequence(stock) stks: the list of securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for date alignment (defaults to sh000001 when unspecified)
    :param int ic_n: the N-day forward return corresponding to the default IC
    :param int ic_rolling_n: the rolling window for the IC
    :param bool spearman: calculate the correlation coefficient with Spearman by default; otherwise Pearson
    :param int mode: the sorting mode for the cross-section data: 0 descending, 1 ascending, 2 no sorting
    :param bool save_all_factors: whether to retain all factor values; affects the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python

        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICIRWeight(indicators, stocks, query)

        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_ICIRWeight(factor_set, stocks, query)


Custom Multi-factor Composition Algorithm Base Class
----------------------------------------------------

The interface for custom multi-factor composition algorithms:

* :py:meth:`MultiFactorBase._calculate` - [Required] Compute the composed factor


Multi-factor Composition Algorithm Base Class
---------------------------------------------

.. py:class:: MultiFactorBase

    Base class for multi-factor composition.

    .. py:attribute:: name Name
    .. py:attribute:: query Query condition

    .. py:method:: __init__(self)

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

    .. py:method:: clone(self)

        Create and return a copy of this instance.

    .. py:method:: get_ref_stock(self)

        Get the reference security.

    .. py:method:: set_ref_stock(self, stk)

        Set a new reference security.

        :param Stock stk: the newly specified reference security

    .. py:method:: get_stock_list(self)

        Get the security list specified at creation.

    .. py:method:: set_stock_list(self, stks)

        Specify a new security list.

        :param list stks: the specified security list

    .. py:method:: get_stock_list_num(self)

        Get the number of securities in the security list specified at creation.

    .. py:method:: get_datetime_list(self)

        Get the list of reference dates (obtained from the reference security through the query condition).

    .. py:method:: get_ref_indicators(self)

        Get the original factor list supplied at creation.

    .. py:method:: set_ref_indicators(self, inds)

        Replace the original factor list.

        :param list inds: the new original factor list

    .. py:method:: get_factor(self, stock)

        Get the composed factor of the specified security.

        :param Stock stock: the specified security

    .. py:method:: get_all_factors(self)

        Get the list of composed factors of all the securities.

        :return: [factor1, factor2, ...] in the same order as the reference securities

    .. py:method:: get_ic(self[, ndays=0])

        Get the IC series of the composed factor, with the same length as the reference dates.

        For factors weighted with IC/ICIR, ndays should ideally be kept consistent with ic_n;
        for factors combined with equal weights, however, calculating with ic_n is not strictly required.
        Therefore, ndays accepts the special value 0, which means the IC is calculated directly using the ic_n parameter.

        :param int ndays: the forward-return horizon (in days) for the IC
        :rtype: Indicator

    .. py:method:: get_icir(self, ir_n[, ic_n=0])

        Get the ICIR series of the composed factor.

        :param int ir_n: the rolling window size used to calculate the IR
        :param int ic_n: the window used for the IC (the same as ndays in get_ic)

    .. py:method:: get_score(self, date[, start=0, end=Null])

        Get all factor values on the cross-section of the specified date, already sorted in descending order; equivalent to the cross-sectional scores of the securities on that date.

        :param Datetime date: the specified date
        :param int start: the start index of the daily ranking slice
        :param int end: the end index of the daily ranking slice (exclusive)
        :param function func: a callable with the prototype (ScoreRecord)->bool or (Datetime, ScoreRecord)->bool
        :rtype: ScoreRecordList

    .. py:method:: get_all_scores(self)

        Get the scores of all dates, with the same length as the reference dates.

        :return: ScoreRecordList

    .. py:method:: get_all_src_factors(self)

        Get the list of all source factors (if standardization or industry neutralization is specified, the returned list is the processed factor list).

        :rtype: list
        :return: IndicatorList with shape stks x inds

    .. py:method:: set_normalize(self, norm)

        Set the standardization or normalization method (applied to all factors).

        :param NormalizeBase norm: the standardization or normalization method instance


    .. py:method:: add_special_normalize(self, name[, norm=None, category="", style_inds=[]])

        Apply a dedicated standardization/normalization, industry neutralization, or style-factor neutralization to the indicator with the specified name. The standardization, industry neutralization and style-factor neutralization are independent of each other; they may be specified together or separately.

        :param str name: the name of the special normalization
        :param Normalize norm: the special normalization method
        :param str category: for industry neutralization, the sector category
        :param list[Indicator] style_inds: the list of style indicators used for neutralization


    .. py:method:: _calculate(self, stks_inds)

        Calculate the composed factor of the securities for each day. The input parameter is prepared and passed in by the upper-layer function, e.g.:

        The securities to calculate - stk1, stk2
        The original factor list - ind1, ind2
        Then the passed-in stks_inds is: [IndicatorList(stk1)[ind1, ind2], IndicatorList(stk2)[ind1, ind2]]

        :param list stks_inds: the prepared original factor lists of all the securities, in the same order as the security list
        :return: all the newly composed factors, stored in security-list order
