.. py:currentmodule:: hikyuu.trade_sys
.. highlight:: python

Multi-factor Composition|MF
===========================

The multi-factor is essentially scoring the candidate targets on the cross-section, so it actually needs to be used together with the Selector (strategy selection algorithm).

For the standardization and neutralization when calculating the factors, see: :doc:`normalize`

Common parameters:

    * **fill_null** *(bool|True)*: whether to fill the nan value when the data is missing, otherwise fill with the most recent value
    * **ic_n** *(int|5)*: the ic_n day return used when calculating the IC of the composed factor
    * **spearman** *(bool|True)*: use spearman to get the correlation coefficient, otherwise pearson
    * **mode** *(int|2)*: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    * **save_all_factors** *(bool|False)*: whether to save all the factor values, affecting the get_actor/get_all_factors methods


Built-in Factor Composition Algorithms
--------------------------------------

.. py:function:: MF_Weight(input, stks, weights, query, ref_stk[, ic_n=5, spearman=True, mode=0, save_all_factors=False])

    Compose the factor by the specified weights = ind1 * weight1 + ind2 * weight2 + ... + indn * weightn, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequence(stock) stks: the list of the securities to calculate
    :param sequence(float) weights: the weight list (must be the same length as the number of the factors)
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods
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

    Compose the factor with the equal weights, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequence(stock) stks: the list of the securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods    
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_EqualWeight(indicators, stocks, query)
        
        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_EqualWeight(factor_set, stocks, query)


.. py:function:: MF_ICWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    Compose the factor with the rolling IC weights, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequence(stock) stks: the list of the securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param int ic_rolling_n: the IC rolling period
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods    
    :rtype: MultiFactorBase

    .. code-block:: python
    
        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICWeight(indicators, stocks, query)
        
        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_ICWeight(factor_set, stocks, query)


.. py:function:: MF_ICIRWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    Compose the factor with the rolling ICIR weights, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequence(stock) stks: the list of the securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param int ic_rolling_n: the IC rolling period
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods    
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

The custom multi-factor composition algorithm interface:

* :py:meth:`MultiFactorBase._calculate` - [Required] Calculate the composed factor


Multi-factor Composition Algorithm Base Class
---------------------------------------------

.. py:class:: MultiFactorBase

    The multi-factor composition base class
    
    .. py:attribute:: name Name
    .. py:attribute:: query Query condition

    .. py:method:: __init__(self)
    
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

    .. py:method:: clone(self)
    
        The clone operation 

    .. py:method:: get_ref_stock(self)

        Get the reference security

    .. py:method:: set_ref_stock(self, stk)

        Reset the reference security

        :param Stock stk: the newly specified reference security

    .. py:method:: get_stock_list(self)

        Get the security list specified at the creation

    .. py:method:: set_stock_list(self, stks)

        Re-specify the security list

        :param list stks: the specified security list

    .. py:method:: get_stock_list_num(self)

        Get the number of the securities in the security list specified at the creation

    .. py:method:: get_datetime_list(self)

        Get the reference date list (obtained from the reference security through the query condition)

    .. py:method:: get_ref_indicators(self)

        Get the original factor list input at the creation

    .. py:method:: set_ref_indicators(self, inds)

        Reset the original factor list

        :param list inds: the new original factor list

    .. py:method:: get_factor(self, stock)

        Get the new composed factor of the specified security

        :param Stock stock: the specified security

    .. py:method:: get_all_factors(self)

        Get the list of the composed factors of all the securities

        :return: [factor1, factor2, ...] in the same order as the reference securities

    .. py:method:: get_ic(self[, ndays=0])

        Get the IC of the composed factor, with the same length as the reference dates.

        For the new factors weighted with IC/ICIR, it is best to keep ndays consistent with ic_n;
        but for the new factors calculated with the equal weights, it is not necessarily required to calculate with ic_n.
        Therefore, ndays has a special value 0, which means calculating the IC directly with the ic_n parameter
     
        :param int ndays: the ndays-day return of the ic
        :rtype: Indicator

    .. py:method:: get_icir(self, ir_n[, ic_n=0])

        Get the ICIR of the composed factor

        :param int ir_n: the n window for calculating the IR
        :param int ic_n: the n window for calculating the IC (the same as ndays in get_ic)

    .. py:method:: get_score(self, date[, start=0, end=Null])

        Get all the factor values of the cross-section on the specified date, already sorted descending, equivalent to the cross-section scores of the securities on that date.

        :param Datetime date: the specified date
        :param int start: the start of the daily ranking to take
        :param int end: the end of the daily ranking to take (exclusive)
        :param function func: a callable object with the prototype (ScoreRecord)->bool or (Datetime, ScoreRecord)->bool
        :rtype: ScoreRecordList

    .. py:method:: get_all_scores(self)

        Get all the scores of all the dates, with the same length as the reference dates

        :return: ScoreRecordList

    .. py:method:: get_all_src_factors(self)

        Get the list of all the original factors (if the standardization or the industry neutralization is specified, the returned list is the processed factor list)

        :rtype: list
        :return: list IndicatorList stks x inds

    .. py:method:: set_normalize(self, norm)

        Set the standardization or normalization method (affecting all the factors)
    
        :param NormalizeBase norm: the standardization or normalization method instance

    
    .. py:method:: add_special_normalize(self, name[, norm=None, category="", style_inds=[]])
        
        Apply a specific standardization/normalization, industry neutralization or style factor neutralization operation to the indicator with the specified name. The standardization operation, the industry neutralization and the style factor neutralization are independent of each other; they can be specified together or separately.

        :param str name: the special normalization method name
        :param Normalize norm: the special normalization method
        :param str category: for the industry neutralization, specify the block category
        :param list[Indicator] style_inds: the list of the style indicators used for the neutralization


    .. py:method:: _calculate(self, stks_inds)

        Calculate the composed factor of the securities for each day; the input parameter is calculated and passed in by the upper layer function, e.g.:

        The security list to calculate - stk1, stk2
        The original factor list - ind1, ind2
        Then the passed stks_inds is: [IndicatorList(stk1)[ind1, ind2], IndicatorList(stk2)[ind1, ind2]]

        :param list stks_inds: the calculated original factor lists of all the securities, in the same order as the security list
        :return: all the new factors, stored in the order of the security list
