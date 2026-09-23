.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

Built-in Technical Indicators
=============================


.. py:function:: ABS([data])

    Get the absolute value

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: ACOS([data])

    The arccosine value

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: AD(kdata)

   The accumulation/distribution line
   
   :param KData kdata: the K-line data
   :rtype: Indicator


.. py:function:: ADJ_FACTOR([kdata])

    Calculate the adjustment factor indicator
    
    Calculate the backward adjustment factor sequence based on the stock's dividend data (the stock sends, the allotments, the conversions, the cash dividends, etc.).
    The adjustment factor indicates how many shares are held now if 1 share was held at the listing, after all the stock sends, the allotments and the conversions.
    It is calculated in a cumulative multiplication way, ensuring the consistency of the adjustment processing of the price, the volume and the amount.
    
    This indicator needs to set the KData context to work properly, set through the setContext() method.
    
    :param KData kdata: optional, pass in the K-line data directly as the context
    :rtype: Indicator
    
    .. warning:: 
       **Important restrictions**:
       
       - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
       - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
       - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
       - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query
    
    **Usage example**::
    
        # Get the adjustment factor of a stock
        stock = sm.getStock("sh000001")
        kdata = stock.getKData(Query(-100))
        adj_factor = ADJ_FACTOR()
        adj_factor.setContext(kdata)
        
        # Or pass in the K-line data directly
        adj_factor = ADJ_FACTOR(kdata)
    
    **Related indicators**:
    
    * :py:func:`ADJ_OPEN` - the adjusted open price
    * :py:func:`ADJ_HIGH` - the adjusted high price
    * :py:func:`ADJ_LOW` - the adjusted low price
    * :py:func:`ADJ_CLOSE` - the adjusted close price
    * :py:func:`ADJ_VOL` - the adjusted volume
    * :py:func:`RECOVER_EQUAL_FORWARD` - the equal-ratio forward adjustment


.. py:function:: ADJ_OPEN()

    Calculate the adjusted open price indicator
    
    Recover the open price backward by the adjustment factor, obtaining the adjusted open price sequence.
    The calculation formula: ADJ_OPEN = ADJ_FACTOR * OPEN
    
    :rtype: Indicator
    
    .. warning:: 
       **Important restrictions**:
       
       - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
       - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
       - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
       - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query
    
    **Related indicators**:
    
    * :py:func:`ADJ_FACTOR` - the adjustment factor
    * :py:func:`RECOVER_EQUAL_FORWARD` - the equal-ratio forward adjustment


.. py:function:: ADJ_HIGH()

    Calculate the adjusted high price indicator
    
    Recover the high price backward by the adjustment factor, obtaining the adjusted high price sequence.
    The calculation formula: ADJ_HIGH = ADJ_FACTOR * HIGH
    
    :rtype: Indicator
    
    .. warning:: 
       **Important restrictions**:
       
       - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
       - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
       - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
       - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query
    
    **Related indicators**:
    
    * :py:func:`ADJ_FACTOR` - the adjustment factor
    * :py:func:`RECOVER_EQUAL_FORWARD` - the equal-ratio forward adjustment


.. py:function:: ADJ_LOW()

    Calculate the adjusted low price indicator
    
    Recover the low price backward by the adjustment factor, obtaining the adjusted low price sequence.
    The calculation formula: ADJ_LOW = ADJ_FACTOR * LOW
    
    :rtype: Indicator
    
    .. warning:: 
       **Important restrictions**:
       
       - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
       - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
       - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
       - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query
    
    **Related indicators**:
    
    * :py:func:`ADJ_FACTOR` - the adjustment factor
    * :py:func:`RECOVER_EQUAL_FORWARD` - the equal-ratio forward adjustment


.. py:function:: ADJ_CLOSE()

    Calculate the adjusted close price indicator
    
    Recover the close price backward by the adjustment factor, obtaining the adjusted close price sequence.
    The calculation formula: ADJ_CLOSE = ADJ_FACTOR * CLOSE
    
    :rtype: Indicator
    
    .. warning:: 
       **Important restrictions**:
       
       - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
       - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
       - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
       - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query
    
    **Related indicators**:
    
    * :py:func:`ADJ_FACTOR` - the adjustment factor
    * :py:func:`RECOVER_EQUAL_FORWARD` - the equal-ratio forward adjustment


.. py:function:: ADJ_VOL()

    Calculate the adjusted volume indicator
    
    Recover the volume backward by the adjustment factor, obtaining the adjusted volume sequence.
    The calculation formula: ADJ_VOL = VOL / ADJ_FACTOR
    
    Note: the volume adjustment uses division, which is opposite to the price adjustment using multiplication. This is because when the share capital increases, the volume corresponding to each share should be reduced accordingly.
    
    :rtype: Indicator
    
    .. warning:: 
       **Important restrictions**:
       
       - **Period restriction**: it is only applicable to the daily period. The non-daily periods such as the weekly and the monthly lines have the alignment problems, and the results may be inaccurate
       - **Depends on the factor management**: it needs to be used together with the factor value storage of the factor management system; call update_all_factors_values() daily to update and save the factor values to ensure the accuracy
       - **The relationship with RECOVER_EQUAL_FORWARD**: this indicator is essentially the same as RECOVER_EQUAL_FORWARD; in the non-factor-management scenarios, it is recommended to use RECOVER_EQUAL_FORWARD directly
       - **The calculation start point**: neither of them calculates from the listing date, but from the start point of the K-line data of the current query
    
    **Related indicators**:
    
    * :py:func:`ADJ_FACTOR` - the adjustment factor
    * :py:func:`RECOVER_EQUAL_FORWARD` - the equal-ratio forward adjustment


.. py:function:: ADVANCE([query=Query(-100), market='SH', stk_type='constant.STOCKTYPE_A'])

    The number of the rising stocks. When a specified context exists and ignore_context is false, the query, market, stk_type parameters will be ignored.

    :param Query query: the query condition
    :param str market: the market it belongs to; when it equals "", get all the markets
    :param int stk_type: the security type; when it is greater than constant.STOCKTYPE_TMP, get all the types of the securities
    :param bool ignore_context: whether to ignore the context. When ignoring, the query, market, stk_type parameters are used forcibly.
    :rtype: Indicator

.. py:function:: AGG_COUNT

    The aggregation function: the non-empty value counting, refer to :func:`AGG_STD`   

.. py:function:: AGG_MAD

    The aggregation function: the average absolute deviation, refer to :func:`AGG_STD`

.. py:function:: AGG_MAX

    The aggregation function: the maximum value, refer to :func:`AGG_STD`

.. py:function:: AGG_MEAN

    The aggregation function: the average value, refer to :func:`AGG_STD`

.. py:function:: AGG_MEDIAN

    The aggregation function: the median, refer to :func:`AGG_STD`    

.. py:function:: AGG_MIN

    The aggregation function: the minimum value, refer to :func:`AGG_STD`

.. py:function:: AGG_PROD

    The aggregation function: the product, refer to :func:`AGG_STD` 

.. py:function:: AGG_SAMPLE(ind[, time="9:35", ktype=Query.MIN, fill_null=False, unit=1])

    The time sampling aggregation indicator, sampling the indicator data at the specified time points.

    If the exactly matching time cannot be found, the valid data closest before the target time is selected.

    :param Indicator ind: the input indicator
    :param str time: the specified sampling time, in the format HH:MM, defaulting to "9:35"
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit
    :return: the indicator data
    :rtype: Indicator

.. py:function:: AGG_SAMPLE_MAX(ind[, start_time="9:30", last_time="10:00", ktype=Query.MIN, fill_null=False, unit=1])

    The maximum in the time period aggregation indicator, counting the maximum of the indicator data within the specified time period [start_time, last_time].

    It includes the start_time and the last_time themselves.

    :param Indicator ind: the input indicator
    :param str start_time: the start time of the time period, in the format HH:MM, defaulting to "9:30"
    :param str last_time: the end time of the time period, in the format HH:MM, defaulting to "10:00"
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit
    :return: the indicator data
    :rtype: Indicator

.. py:function:: AGG_SAMPLE_MIN(ind[, start_time="9:30", last_time="10:00", ktype=Query.MIN, fill_null=False, unit=1])

    The minimum in the time period aggregation indicator, counting the minimum of the indicator data within the specified time period [start_time, last_time].

    It includes the start_time and the last_time themselves.

    :param Indicator ind: the input indicator
    :param str start_time: the start time of the time period, in the format HH:MM, defaulting to "9:30"
    :param str last_time: the end time of the time period, in the format HH:MM, defaulting to "10:00"
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit
    :return: the indicator data
    :rtype: Indicator

.. py:function:: AGG_SAMPLE_MEAN(ind[, start_time="9:30", last_time="10:00", ktype=Query.MIN, fill_null=False, unit=1])

    The average in the time period aggregation indicator, counting the average of the indicator data within the specified time period [start_time, last_time].

    It includes the start_time and the last_time themselves.

    :param Indicator ind: the input indicator
    :param str start_time: the start time of the time period, in the format HH:MM, defaulting to "9:30"
    :param str last_time: the end time of the time period, in the format HH:MM, defaulting to "10:00"
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit
    :return: the indicator data
    :rtype: Indicator

.. py:function:: AGG_QUANTILE(ind[, ktype=Query.MIN, fill_null=False, unit=1, quantile=0.75])

    Aggregate the quantile of the other K-line periods, refer to the AGG_STD help

    :param Indicator ind: the indicator data
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit (the grouping unit of the context K-lines; when using the daily line to calculate the minute-line aggregation, unit=2 means aggregating the minute-lines of 2 days)
    :param float quantile: the quantile, between (0, 1)
    :return: the indicator data
    :rtype: Indicator     

.. py:function:: AGG_STD(ind[, ktype=Query.MIN, fill_null=False, unit=1, ddof=1])

    Aggregate the standard deviation of the other K-line periods, e.g. calculating the standard deviation of the minute-line close prices aggregated when calculating the daily line

        >>> kdata = get_kdata('sh600000', Query(Datetime(20250101), ktype=Query.DAY))
        >>> ind = AGG_STD(CLOSE(), ktype=Query.MIN, fill_null=False, unit=1, ddof=1)
        >>> ind(k)

    :param Indicator ind: the indicator data
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit (the rolling aggregation unit; when using the daily line to calculate the minute-line aggregation, unit=2 means aggregating the minute-lines of 2 days)
    :param int ddof: the degree of freedom (1: the sample standard deviation, 0: the population standard deviation)
    :return: the indicator data
    :rtype: Indicator

.. py:function:: AGG_VAR

    The aggregation function: the variance, refer to :func:`AGG_STD`


.. py:function:: AGG_VWAP([ktype=Query.MIN, fill_null=False, unit=1])

    Aggregate the volume weighted average price of the other K-line periods (Volume Weighted Average Price).
    VWAP is the volume-weighted average price, calculated by multiplying the volume of each minute (or unit time) by the trading price of that minute, then summing all the products, and finally dividing by the total volume.

    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit (the grouping unit of the context K-lines; when using the daily line to calculate the minute-line aggregation, unit=2 means aggregating the minute-lines of 2 days)
    :return: the indicator data
    :rtype: Indicator


.. py:function:: AGG_FUNC(ind, agg_func[, ktype=Query.MIN, fill_null=False, unit=1]
      
    Aggregate the indicators of the other K-line periods with a custom function.
    
    Example, calculating the sum of the minute-line close prices aggregated when calculating the daily line:

      >>> kdata = get_kdata('sh600000', Query(Datetime(20250101), ktype=Query.DAY))
      >>> ind = AGG_FUNC(CLOSE(), lambda ds, x: np.sum(x))
      >>> ind(k)

    :param Indicator ind: the indicator to calculate
    :param callable agg_func: the custom aggregation function, whose input parameters are arg1: a datetime list, arg2: a numpy array, and which returns the aggregation result for the list; note that it is a single value
    :param KQuery.KType ktype: the aggregated K-line period
    :param bool fill_null: whether to fill the missing values
    :param int unit: the aggregation period unit (the grouping unit of the context K-lines; when using the daily line to calculate the minute-line aggregation, unit=2 means aggregating the minute-lines of 2 days)
    :return: the aggregation result
    :rtype: Indicator


.. py:function:: ALIGN(data, ref[, fill_null=True])

    Align by the specified reference dates

    :param Indicator data: the input data
    :param DatetimeList|Indicator|KData ref: the specified DatetimeList, Indicator or KData used as the date reference
    :param bool fill_null: the missing data is filled with nan; otherwise, use the data less than and closest to the corresponding date
    :retype: Indicator

.. py:function:: AMA([data, n=10, fast_n=2, slow_n=30])

    The Perry J. Kaufman adaptive moving average [BOOK1]_
    
    :param Indicator data: the input data
    :param int n: the period window for calculating the average, which must be an integer greater than 2
    :param int fast_n: the corresponding fast period N
    :param int slow_n: the N value corresponding to the slow EMA line
    :rtype: Indicator
    
    * result(0): AMA
    * result(1): ER

    
.. py:function:: AMO([data])

    Get the amount; wrap the amount of the KData as an Indicator
    
    :param data: the input data (KData or Indicator)
    :rtype: Indicator


.. py:function:: ASIN([data])

    The arcsine value

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: ATAN([data])

    The arctangent value

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: ATR([kdata, n=14])

    The Average True Range, the simple moving average of the true range TR

    :param KData kdata: the source data to calculate
    :param int n: the period window for calculating the average, which must be an integer greater than 1
    :rtype: Indicator


.. py:function:: ADX([kdata, n=14])

    The Average Directional Index

    ADX belongs to the trend strength indicators; it does not distinguish the up/down direction, but only judges whether there is a trend. It uses the Wilder original formula.

    :param KData kdata: the source data to calculate
    :param int n: the calculation period, defaulting to 14, which must be an integer greater than 1
    :rtype: Indicator

    * result(0): the ADX itself (the trend strength, with the value range 0~100)
    * result(1): +DI (the rising direction line, the bull force)
    * result(2): -DI (the falling direction line, the bear force)

    The judgment criteria:

    - ADX >= 25: there is a clear one-sided trend (both rising/falling are fine)
    - ADX < 25: no trend, a range oscillation
    - The greater the ADX value, the stronger the trend

    **The calculation principle**:

    1. **The true range (TR)**: TR = max(HIGH-LOW, abs(HIGH-REF(CLOSE,1)), abs(LOW-REF(CLOSE,1)))
    2. **The directional movement (+DM/-DM)**: +DM = HIGH - REF(HIGH,1) (if >0 and greater than -DM); -DM = REF(LOW,1) - LOW (if >0 and greater than +DM)
    3. **The Wilder smoothing**: the initial is the simple average of the N periods; afterwards, the recursive formula S_t = S_{t-1} * (N-1)/N + X_t/N is used
    4. **The directional index (±DI)**: +DI = 100 * (the smoothed +DM) / (the smoothed TR); -DI = 100 * (the smoothed -DM) / (the smoothed TR)
    5. **The directional index (DX)**: DX = 100 * abs(+DI - (-DI)) / (+DI + (-DI))
    6. **The Average Directional Index (ADX)**: smooth the DX with Wilder

    **Usage example**::

        # Get the K-line data
        kdata = get_kdata('sh000001', Query(-200))
        
        # Calculate the ADX indicator
        adx = ADX(kdata, 14)
        
        # Get the ADX value (index 0), the +DI value (index 1), the -DI value (index 2)
        adx_value = adx.get(-1, 0)
        pdi_value = adx.get(-1, 1)
        mdi_value = adx.get(-1, 2)


.. py:function:: ADX2([kdata, n=14])

    The Average Directional Index (ADX2) - using the EMA smoothing way

    ADX2 belongs to the trend strength indicators; it does not distinguish the up/down direction, but only judges whether there is a trend.
    The difference from the ADX is that it uses the EMA (the Exponential Moving Average) instead of the Wilder smoothing, being more sensitive to the trend changes.

    :param KData kdata: the source data to calculate
    :param int n: the calculation period, defaulting to 14, which must be an integer greater than 1
    :rtype: Indicator

    * result(0): the ADX2 itself (the trend strength, with the value range 0~100)
    * result(1): +DI (the rising direction line, the bull force)
    * result(2): -DI (the falling direction line, the bear force)

    The judgment criteria:

    - ADX2 >= 25: there is a clear one-sided trend (both rising/falling are fine)
    - ADX2 < 25: no trend, a range oscillation
    - The greater the ADX2 value, the stronger the trend

    **The difference from the ADX**:

    - **ADX**: uses the Wilder smoothing (the smoothing coefficient = 1/N), responding slower but more stable
    - **ADX2**: uses the EMA smoothing (the smoothing coefficient = 2/(N+1)), being more sensitive to the trend changes

    **Usage example**::

        # Get the K-line data
        kdata = get_kdata('sh000001', Query(-200))
        
        # Calculate the ADX2 indicator
        adx2 = ADX2(kdata, 14)
        
        # Get the ADX2 value (index 0), the +DI value (index 1), the -DI value (index 2)
        adx2_value = adx2.get(-1, 0)
        pdi_value = adx2.get(-1, 1)
        mdi_value = adx2.get(-1, 2)


.. py:function:: AVEDEV(data[, n=22])

    The average absolute deviation, getting the N-day average absolute deviation of X

    :param Indicator data: the input data
    :param int|Indicator n: the time window
    :rtype: Indicator


.. py:function:: BACKSET([data, n=2])

    The forward assignment, setting the data from the current position to several periods ago to 1.

    Usage: BACKSET(X,N); if X is not 0, set the values from the current position to N periods ago to 1.

    For example: BACKSET(CLOSE>OPEN,2); if it closes bullish, set the values of this period and the previous period to 1, otherwise 0

    :param Indicator data: the input data
    :param int n|Indicator|IndParam: N periods
    :rtype: Indicator


.. py:function:: BARSCOUNT([data])

    The number of the valid periods, getting the total number of the periods.

    Usage: BARSCOUNT(X), the number of the days from the first valid data to the current.

    For example: BARSCOUNT(CLOSE) gets the total number of the trading days since the listing for the daily data, and the number of the trading minutes of the day for the 1-minute line.

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: BARSLAST([data])

    The position of the last time the condition was satisfied, the number of the periods from the last time the condition was satisfied to the current.

    Usage: BARSLAST(X): the number of the days from the last time X is not 0 to now.

    For example: BARSLAST(CLOSE/REF(CLOSE,1)>=1.1) means the number of the periods from the last limit-up board to the current

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: BARSLASTS([data|val,] n)

    The number of the periods from the Nth time the condition was satisfied to the current.

    Usage: BARSLASTS(X, N): the number of the days from the Nth time X is not 0 to now.

    For example: BARSLASTS(CLOSE/REF(CLOSE,1)>=1.1, 2) means the number of the periods from the 2nd limit-up board to the current.

    Note: when N=1, BARSLASTS(X, 1) is equivalent to BARSLAST(X).

    It supports the dynamic parameters; n can be an integer, an Indicator or an IndParam.

    Usage example::

        # The static parameter
        result = BARSLASTS(cond, 2)
        
        # The dynamic parameter - using CVAL
        result = BARSLASTS(cond, CVAL(cond, 2))
        
        # The dynamic parameter - using IndParam
        result = BARSLASTS(cond, IndParam(n_indicator))
        
        # The dynamic parameter - using an Indicator directly
        result = BARSLASTS(cond, n_indicator)

    :param Indicator data: the input data (optional)
    :param float val: the input value (optional, choose one of data and it)
    :param int|Indicator|IndParam n: the Nth time the condition is satisfied, supporting the dynamic parameters
    :rtype: Indicator


.. py:function:: BARSLASTCOUNT([data])

    Count the number of the periods continuously satisfying the condition

    Usage: BARSLASTCOUNT(X), where X is a condition expression.

    For example: BARSLASTCOUNT(CLOSE>OPEN) means counting the number of the periods continuously closing bullish

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: BARSSINCE([data])

    The number of the periods from the first time the condition was satisfied to the current.

    Usage: BARSSINCE(X): the number of the days from the first time X is not 0 to now.

    For example: BARSSINCE(HIGH>10) means the number of the periods from when the stock price exceeds 10 yuan to the current

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: BETA(ind, ref_ind[, n=10, fill_null=True])

    Calculate the Beta coefficient, e.g. measuring the sensitivity between the asset return and the market return.

    The calculation formula: Beta = Cov(stock_return, market_return) / Var(market_return)

    .. note:: BETA itself does not convert the input data to the return rate (pct_change);
              the input indicator should be the already calculated return rate data.

    :param Indicator ind: the input indicator, such as the stock return indicator
    :param Indicator ref_ind: the reference indicator, such as the market return indicator
    :param int n: the rolling window size (greater than 2 or equal to 0), defaulting to 10. If it is 0, use the length of the input ind
    :param bool fill_null: during the date alignment, whether to fill the missing dates with the nan values, defaulting to true
    :rtype: Indicator

    **Usage example**::

        # Calculate the Beta coefficient of the stock relative to the market
        stock_return = ROC(CLOSE(), 1)
        market_return = ROC(INDEXC(), 1)
        beta = BETA(stock_return, market_return, 60)


.. py:function:: BETWEEN(a, b, c)

    Between (between two numbers)

    Usage: BETWEEN(A,B,C) means that it returns 1 when A is between B and C, otherwise 0

    For example: BETWEEN(CLOSE,MA(CLOSE,10),MA(CLOSE,5)) means that the close price is between the 5-day MA and the 10-day MA

    :param Indicator a: A
    :param Indicator b: B
    :param Indicator c: C
    :rtype: Indicator

    
.. py:function:: BLOCKSETNUM(block, query)

    The cross-sectional statistics (returning the number of the stocks in the block), with two calling ways:

        BLOCKSET(block, query)
        
        BLOCKSET(stks, query)

    :param Block block | sequence stks: the block to count or a stock list
    :param Query query: the statistics range


.. py:function:: CLOSE([data])
   
    Get the close price; wrap the close price of the KData as an Indicator
    
    :param data: the input data (KData or Indicator)
    :rtype: Indicator
    
    
.. py:function:: CAPITAL(kdata)

   Get the circulating share capital (unit: 10,000 shares), the same as LIUTONGPAN
   
   :param KData kdata: the K-line data
   :rtype: Indicator
   

.. py:function:: CEIL([data])

    The same as :py:func:`CEILING`

   
.. py:function:: CEILING([data])

    Round up (rounding in the direction of increasing the value) to an integer
   
    Usage: CEILING(A) returns the closest integer in the direction of increasing the value of A
   
    For example: CEILING(12.3) gets 13; CEILING(-3.5) gets -3
   
    :param data: the input data
    :rtype: Indicator
   

.. py:function:: CYCLE(kdata, [adjust_cycle=1], [adjust_mode='query'], [delay_to_trading_day=True])
          
    The PF rebalance cycle indicator, mainly used for the PF rebalance day verification, and as an SG

    :param KData kdata: the K-line data
    :param int adjust_cycle: the adjustment period
    :param string adjust_mode: the adjustment way
    :param bool delay_to_trading_day: whether the adjustment period is postponed to the trading day
    :rtype: Indicator


.. py:function:: CONTEXT([ind, fill_null=False, use_self_ktype=False, use_self_recover_type=False])
    
    The independent context. It uses the context that the ind comes with. When a new context is specified, it will not change the existing context.
    For example: ind = CLOSE(k1); when a new context ind = ind(k2) is specified, the close price of k2 is used. If you still want to use the close price of k1,
    you need to use ind = CONTEXT(CLOSE(k1)); at this time ind(k2) will still use the close price of k1.
    
    :param Indicator ind: the indicator object
    :param bool fill_null: during the date alignment, the missing dates are filled with the empty values; otherwise, they are filled with the previous values.
    :param bool use_self_ktype: use the KTYPE in its own independent context when calculating the formula
    :param bool use_self_recover_type: use the RECOVER_TYPE in its own independent context when calculating the formula
    :rtype: Indicator

.. py:function:: CONTEXT_K(ind)

    Get the indicator context. The Indicator::getContext() method gets the current context, but it cannot get the specified independent context for the CONTEXT independent context indicator; you need to use this method to get it

    :param Indicator ind: the indicator object
    :rtype: KData

.. py:function:: CORR(ind1, ind2, n)

    Calculate the sample correlation coefficient and the covariance of ind1 and ind2. There are two results in the return; the first is the correlation coefficient, and the second is the covariance.

    :param Indicator ind1: the indicator 1
    :param Indicator ind2: the indicator 2
    :param int n: calculate the correlation coefficient of the data between the two inds by the specified length n. If it is 0, use the length of the input ind.
    :rtype: Indicator    

.. py:function:: COV(ind1, ind2[, n=10, fill_null=True])

    Calculate the sample covariance of ind1 and ind2.

    :param Indicator ind1: the indicator 1
    :param Indicator ind2: the indicator 2
    :param int n: the rolling window size (greater than 2 or equal to 0), defaulting to 10. If it is 0, use the length of the input ind.
    :param bool fill_null: during the date alignment, whether to fill the missing dates with the nan values, defaulting to true
    :rtype: Indicator


.. py:function:: COS([data])

    The cosine value

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: COST(k[, x=10.0])

    The cost distribution

    Usage: COST(k, X) means what the price of the X% profitable positions is

    For example: COST(k, 10) means what the price of the 10% profitable positions is, i.e. 10% of the positions are below this price, and the other 90% are above this price, being the trapped positions. This function is only valid for the daily analysis period.

    :param KData k: the associated K-line data
    :param float x: the x% profitable price, 0~100
    :rtype: Indicator


.. py:function:: CODELIKE([data,] pattern)

    The security code pattern matching
    
    Return whether the security code in the current context matches the specified pattern. It supports the wildcard matching:
    
    - ``*``: matches any sequence of any characters of any length (including the empty sequence)
    - ``?``: matches a single arbitrary character
    
    The return value: it returns 1.0 when the match succeeds, otherwise 0.0. The return values of all the periods are the same.
    
    Usage example::
    
        # Match the stock codes starting with "600"
        kdata = get_kdata('sh600000', Query(-10))
        result = CODELIKE(kdata, "600*")
        
        # Match the stock codes of 6 digits
        result = CODELIKE(kdata, "??????")
        
        # Match the codes starting with "000" and ending with "1"
        result = CODELIKE(kdata, "000*1")
        
        # Match a specific code exactly
        result = CODELIKE(kdata, "600000")

    :param KData data: the K-line data (optional; it can be omitted when a context exists)
    :param str pattern: the matching pattern, supporting the wildcards * and ?
    :return: the matching result indicator, with the same values for all the periods (1.0 or 0.0)
    :rtype: Indicator


.. py:function:: COUNT([data, n=20])

    Count the number of the periods satisfying the condition.
    
    Usage: COUNT(X,N), counting the number of the periods satisfying the condition X within the N periods; if N=0, start from the first valid value.
    
    For example: COUNT(CLOSE>OPEN,20) means counting the number of the periods closing bullish within the 20 periods
    
    :param Indicator data: the condition
    :param int|Indicator|IndParam n: the periods
    :rtype: Indicator


.. py:function:: CROSS(x, y)

    The cross function

    :param x: the variable or the constant, the first line to judge the cross
    :param y: the variable or the constant, the second line to judge the cross
    :rtype: Indicator


.. py:function:: CVAL([data, value=0.0, discard=0])

    data is an Indicator instance; create a constant indicator with the same length as data, whose value is value, and the discard length is the same as data
    
    :param Indicator data: the Indicator instance
    :param float value: the constant value
    :param int discard: the discard quantity
    :rtype: Indicator


.. py:function:: DATE([data])

    Get the year-month-day of this period since 1900. Usage: DATE, e.g. the function returns 1000101, which means January 1, 2000.

    :param data: the input data KData
    :rtype: Indicator


.. py:function:: DAY([data])

    Get the date of this period. Usage: DAY, the valid value range returned by the function is (1-31).

    :param data: the input data KData
    :rtype: Indicator


.. py:function:: DECLINE([query=Query(-100), market='SH', stk_type='constant.STOCKTYPE_A'])

    The number of the falling stocks. When a specified context exists and ignore_context is false, the query, market, stk_type parameters will be ignored.

    :param Query query: the query condition
    :param str market: the market it belongs to; when it equals "", get all the markets
    :param int stk_type: the security type; when it is greater than constant.STOCKTYPE_TMP, get all the types of the securities
    :param bool ignore_context: whether to ignore the context. When ignoring, the query, market, stk_type parameters are used forcibly.
    :rtype: Indicator


.. py:function:: DEVSQ([data, n=10])

    The sum of the squared deviations of the data, getting the N-day sum of the squared deviations of X

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator


.. py:function:: DIFF([data, n=1])

    The difference indicator, i.e. data[i] - data[i-n]
    
    :param Indicator data: the input data
    :param int n: the difference period, defaulting to 1
    :rtype: Indicator


.. py:function:: DISCARD(data, discard)
    
    Set the amount of the discarded data of the indicator result in the way of the indicator formula.

    :param Indicator data: the indicator
    :param int discard: the amount of the discarded data
    :rtype: Indicator


.. py:function:: DMA(ind, a)

    The dynamic moving average

    Usage: DMA(X,A), getting the dynamic moving average of X.

    The algorithm: if Y=DMA(X,A), then Y=A*X+(1-A)*Y', where Y' represents the Y value of the previous period.

    For example: DMA(CLOSE,VOL/CAPITAL) means getting the average price with the turnover rate as the smoothing factor

    :param Indicator ind: the input data
    :param Indicator a: the dynamic coefficient
    :rtype: Indicator


.. py:function:: DROPNA([data])

    Remove the nan values

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: DOWNNDAY(data[, n=3])

    The number of the consecutive falling periods; DOWNNDAY(CLOSE,M) means M consecutive falling periods

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator

    
.. py:function:: EMA([data, n=22])

    The Exponential Moving Average

    :param data: the input data
    :param int|Indciator|IndParam n: the period window for calculating the average, which must be an integer greater than 0 
    :rtype: Indicator
    

.. py:function:: EVERY([data, n=20])

    Always exists

    Usage: EVERY (X,N) means that the condition X always exists within the N periods

    For example: EVERY(CLOSE>OPEN,10) means that all the previous 10 days are the bullish candles

    :param data: the input data
    :param int|Indicator|IndParam n: the period window for calculating the average, which must be an integer greater than 0 
    :rtype: Indicator


.. py:function:: EXIST([data, n=20])

    Exists; EXIST(X,N) means that the condition X exists within the N periods

    :param data: the input data
    :param int|Indicator|IndParam n: the period window for calculating the average, which must be an integer greater than 0 
    :rtype: Indicator


.. py:function:: EXP([data])

    EXP(X) is e to the power of X

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: FACTOR(factor)

    The factor indicator conversion

    Convert a Factor object to an Indicator, so that it can be used in the indicator system.
    This indicator needs to set the K-line context to calculate.

    :param Factor factor: the factor object (choose one of name and it)
    :param str name: the factor name (the convenient version, choose one of factor and it)
    :rtype: Indicator


.. py:function:: FIXED_START_INDEX([ind, start_index=0])

    Fix the start index of the query range used when calculating the indicator

    For some indicators that change with the time start point (such as AD), fixing the start index ensures the calculation from the first data of the stock.

    :param Indicator ind: the input indicator
    :param int start_index: the start index position, defaulting to 0; when it is negative, it means starting the calculation from the time point moved forward by index from the current latest
    :rtype: Indicator


.. py:function:: FIXED_START_DATE([ind, start_date=Datetime.min()])

    Fix the start date of the query range used when calculating the indicator

    For some indicators that change with the time start point (such as AD), fixing the start date ensures the calculation from the specified date.

    :param Indicator ind: the input indicator
    :param Datetime start_date: the start date, defaulting to Datetime.min()
    :rtype: Indicator


.. py:function:: FILTER([data, n=5])

    The signal filter, filtering the signals that appear consecutively.

    Usage: FILTER(X,N): after X satisfies the condition, delete the data within the following N periods, setting them to 0.

    For example: FILTER(CLOSE>OPEN,5) searches for the bullish candles, and the bullish candles appearing again within 5 days are not recorded.

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the filter period
    :rtype: Indicator


.. py:function:: FINANCE([kdata, ix, name])

    Get the historical finance information. (The corresponding historical finance field information can be queried through StockManager.get_history_finance_all_fields)

    When ix and name are used, choose one of them. That is, either use ix or use name to get.

    :param KData kdata: the K-line data
    :param int ix: the historical finance information field index
    :param int name: the historical finance information field name
    :rtype: Indicator


.. py:function:: FLOOR([data])

    Round down (rounding in the direction of decreasing the value) to an integer
    
    Usage: FLOOR(A) returns the closest integer in the direction of decreasing the value of A
    
    For example: FLOOR(12.3) gets 12

    :param data: the input data
    :rtype: Indicator

.. py:function:: GROUP_COUNT(ind[, ktype=Query.DAY,  unit=1])
      
    The custom group cumulative counting

    :param Indicator ind: the indicator to calculate
    :param KQuery.KType ktype: the grouped K-line period
    :param int unit: the grouping period unit (the grouping K-line period unit; when using the daily line to calculate the minute-lines, unit=2 means the minute-lines accumulated by 2 days)
    :rtype: Indicator        

.. py:function:: GROUP_MAX(ind[, ktype=Query.DAY,  unit=1])
      
    The custom group cumulative maximum

    :param Indicator ind: the indicator to calculate
    :param KQuery.KType ktype: the grouped K-line period
    :param int unit: the grouping period unit (the grouping K-line period unit; when using the daily line to calculate the minute-lines, unit=2 means the minute-lines accumulated by 2 days)
    :rtype: Indicator        

.. py:function:: GROUP_MIN(ind[, ktype=Query.DAY,  unit=1])
      
    The custom group cumulative minimum

    :param Indicator ind: the indicator to calculate
    :param KQuery.KType ktype: the grouped K-line period
    :param int unit: the grouping period unit (the grouping K-line period unit; when using the daily line to calculate the minute-lines, unit=2 means the minute-lines accumulated by 2 days)
    :rtype: Indicator        

.. py:function:: GROUP_PROD(ind[, ktype=Query.DAY,  unit=1])
      
    The custom group cumulative product

    :param Indicator ind: the indicator to calculate
    :param KQuery.KType ktype: the grouped K-line period
    :param int unit: the grouping period unit (the grouping K-line period unit; when using the daily line to calculate the minute-lines, unit=2 means the minute-lines accumulated by 2 days)
    :rtype: Indicator

.. py:function:: GROUP_SUM(ind[, ktype=Query.DAY,  unit=1])
      
    The custom group cumulative sum

    :param Indicator ind: the indicator to calculate
    :param KQuery.KType ktype: the grouped K-line period
    :param int unit: the grouping period unit (the grouping K-line period unit; when using the daily line to calculate the minute-lines, unit=2 means the minute-lines accumulated by 2 days)
    :rtype: Indicator    

.. py:function:: GROUP_FUNC(ind, group_func[, ktype=Query.DAY,  unit=1]
      
    The custom group cumulative calculation indicator.
    
    Example, calculating the sum of the minute-line close prices aggregated when calculating the daily line:

      >>> kdata = get_kdata('sh600000', Query(Datetime(20250101), ktype=Query.DAY))
      >>> ind = GROUP_FUNC(CLOSE(), lambda dates, data: data/2.0)
      >>> ind(k)

    :param Indicator ind: the indicator to calculate
    :param callable group_func: the custom group cumulative function, whose input parameters are arg1: a datetime list, arg2: a numpy array, and which returns the cumulative calculation result with the same length as the input; the type must also be np.array
    :param KQuery.KType ktype: the grouped K-line period
    :param int unit: the grouping period unit (the grouping K-line period unit; when using the daily line to calculate the minute-lines, unit=2 means the minute-lines accumulated by 2 days)
    :rtype: Indicator    


.. py:function:: HHV([data, n=20])

    The highest price within the N days; when N=0, start from the first valid value.

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the N-day time window
    :rtype: Indicator
    

.. py:function:: HHVBARS([data, n=20])

    The position of the previous high point, getting the number of the periods from the previous high point to the current.

    Usage: HHVBARS(X,N): getting the number of the periods from the highest value of X within the N periods to the current; N=0 means counting from the first valid value

    For example: HHVBARS(HIGH,0) gets the number of the periods from the historical new high to the current

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the N-day time window
    :rtype: Indicator
    
.. py:function:: HIGH([data])

    Get the high price; wrap the high price of the KData as an Indicator

    :param data: the input data (KData or Indicator) 
    :rtype: Indicator
    
    
.. py:function:: HOUR([data])

    Get the number of the hours of this period. Usage: HOUR, the valid value range returned by the function is (0-23), and the value is 0 for the daily and the longer analysis periods.

    :param data: the input data KData
    :rtype: Indicator


.. py:function:: HSL(kdata)

    Get the turnover rate (the percentage x%), equal to VOL(k) / CAPITAL(k)
    
    :param KData kdata: the K-line data
    :rtype: Indicator
    
.. py:function:: IC(ind, stks[, n=1, spearman=True, strict=True])

    Calculate the IC of the specified factor relative to the reference securities (the RankIC by default)

    The IC originally needs "the factor value at time t → the return at time t+1"; it is changed to calculate "the factor value at time t → the return of the previous N days at time t" (e.g. the return of the past 5 days), and it is called the "current IC". (Otherwise the current values would all be missing NA)
    If the strict "the factor value at time t → the return at time t+1" calculation is needed, please set strict=True (note that in this mode, the last n positions are NA)
    
    :param sequence or Block stks: the securities portfolio
    :param int n: the time window (the corresponding n-day return)
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param bool strict: the strict mode, following the IC definition "the factor value at time t → the return at time t+1"
    :rtype: Indicator


.. py:function:: ICIR(ind, stks[, n=1, rolling_n=120, spearman=True])

    Calculate the IC factor IR = the multi-period average of the IC / the standard deviation of the IC

    :param sequence or Block stks: the securities portfolio
    :param int n: the time window (the corresponding n-day return)
    :param int rolling_n: the rolling period
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson  
    :param bool strict: the strict mode, following the IC definition "the factor value at time t → the return at time t+1"  
    :rtype: Indicator


.. py:function:: IR(p, b[, n=100])

    The Information Ratio (IR)

    The formula: (P-B) / TE
    P: the portfolio return
    B: the benchmark return
    TE: the standard deviation between p and b of each day within the investment period
    In actual use, P is generally the asset curve of the TM, and B is the close price of the CSI 300, e.g.:
    ref_k = sm["sh000300"].get_kdata(query)
    funds = my_tm.get_funds_curve(ref_k.get_datetime.list())
    ir = IR(PRICELIST(funds), ref_k.close, 0)

    If you want to calculate the IR value of the factor IC, please use the ICIR indicator

    :param Indicator p:
    :param Indicator b:
    :param int n: the time window (defaulting to 100); if you only want to use the last value, you can use 0, or specify len(p), len(b)
    :rtype: Indicator

    
.. py:function:: IF(x, a, b)

    The condition function, getting the different values according to the condition.
    
    Usage: IF(X,A,B); if X is not 0, return A, otherwise return B
    
    For example: IF(CLOSE>OPEN,HIGH,LOW) means that if this period closes bullish, return the highest value, otherwise return the lowest value
    
    :param Indicator x: the condition indicator
    :param Indicator a: the candidate indicator a
    :param Indicator b: the candidate indicator b
    :rtype: Indicator
    

.. py:function:: INBLOCK(data, category, name)        

    Whether the security of the current context is in the specified block.

    :param KData data: the specified K-line data (the context)
    :param string category: the block category
    :param string name: the block name
    :rtype: Indicator


.. py:function:: INDEXC([kdata])
    
    Return the corresponding market close prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index


.. py:function:: INDEXH([kdata])
    
    Return the corresponding market high prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index


.. py:function:: INDEXL([kdata])

    Return the corresponding market low prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index


.. py:function:: INDEXO([kdata])
    
    Return the corresponding market open prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index


.. py:function:: INDEXA([kdata])
    
    Return the corresponding market amounts, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index


.. py:function:: INDEXV([kdata])

    Return the corresponding market volumes, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index


.. py:function:: INDEXADV([query])
    
    The TDX 880005 market-wide number of the rising stocks, which may not be updated intraday!


.. py:function:: INDEXDEC([query])
    
    The TDX 880005 market-wide number of the falling stocks, which may not be updated intraday!


.. py:function:: INSUM(block, query, ind, mode)

    Return the calculated values of the corresponding outputs of this indicator of each component in the block by the calculation type.

    Note: when the INSUM uses the modes 4/5, it is equivalent to the RANK function, but it is not suitable for use in the MF; when used in the MF, the calculation amount is at the N x N level, calculating slowly. If you want to use it in the MF, it is recommended to use the RANK [Donating user] indicator directly.

    Usage:
    
        INSUM(block, query, ind, mode)

        INSUM(stks, query, ind, mode)

    :param Block block | sequence stks: the specified block or the security list
    :param Query query: the specified range
    :param Indicator ind: the specified indicator
    :param int mode: the calculation type: 0-accumulation, 1-average, 2-maximum, 3-minimum, 4-descending ranking (the highest indicator value is ranked 1), 5-ascending ranking (the greater the indicator value, the greater the ranking value)
    :rtype: Indicator    


.. py:function:: INTPART([data])

    Take the integer part (rounding by decreasing the absolute value, i.e. getting the integer part of the data)
    
    :param data: the input data
    :rtype: Indicator


.. py:function:: ISNA(ind[, ignore_discard=False])

    Judge whether the indicator is a nan value; if it is a nan value, return 1, otherwise return 0.

    :param Indicator ind: the specified indicator
    :param bool ignore_discard: ignore the discarded data of the indicator


.. py:function:: ISLIMITDOWN([kdata])

    The indicator judging whether the stock is the limit down

    Judge whether the close price of the day reaches the limit-down price according to the different stock types:
    
    - The ordinary A-share stocks: the limit-down range is 10%
    - The Beijing Stock Exchange stocks: the limit-down range is 30%  
    - The ChiNext/STAR Market stocks: the limit-down range is 20%
    - The ST stocks have a limit-down range of 5%, but since the historical date information of the ST mark is lacking, it is not handled for now

    The limit-down judgment logic: the close price of the day <= the close price of the previous day × (1 - the limit-down range)

    :param KData kdata: the K-line data
    :rtype: Indicator


.. py:function:: ISLIMITUP([kdata])

    The indicator judging whether the stock is the limit up

    Judge whether the close price of the day reaches the limit-up price according to the different stock types:
    
    - The ordinary A-share stocks: the limit-up range is 10%
    - The Beijing Stock Exchange stocks: the limit-up range is 30%
    - The ChiNext/STAR Market stocks: the limit-up range is 20%
    - The ST stocks have a limit-up range of 5%, but since the historical date information of the ST mark is lacking, it is not handled for now

    The limit-up judgment logic: the close price of the day >= the close price of the previous day × (1 + the limit-up range)

    :param KData kdata: the K-line data
    :rtype: Indicator


.. py:function:: ISINF(ind)

    Judge whether the indicator is a positive infinity (+inf) value; if it is a +inf value, return 1, otherwise return 0. To judge the negative infinity, use ISINFA.

    :param Indicator ind: the specified indicator
    :rtype: Indicator


.. py:function:: ISINFA(ind)

    Judge whether the indicator is a negative infinity (-inf) value; if it is a -inf value, return 1, otherwise return 0. To judge the positive infinity, use ISINF.

    :param Indicator ind: the specified indicator
    :rtype: Indicator


.. py:function:: JUMPDOWN([ind])

    The edge jump, jumping from <= 0.0 to > 0.0
    
    :param Indicator ind: the indicator
    :rtype: Indicator


.. py:function:: JUMPUP([ind])
    
    The edge jump, jumping from > 0.0 to <= 0.0
    
    :param Indicator ind: the indicator
    :rtype: Indicator


.. py:function:: KALMAN(ind, [q=0.01], [r=0.1])

    The Kalman filter, used to smooth the indicator; the smoothing coefficients q and r can be set, defaulting to q=0.01, r=0.1

    :param Indicator ind: the indicator
    :param float q: the smoothing coefficient
    :param float r: the noise coefficient
    :rtype: Indicator

    
.. py:function:: KDATA([data])

    Wrap the KData as an Indicator, used for the calculation of the other indicators

    :param data: the KData or an Indicator with 6 returned results (such as the Indicator generated by KDATA)
    :rtype: Indicator

    
.. py:function:: KDATA_PART([data, kpart])

    Select and return the indicator KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL by the string, e.g.: KDATA_PART("CLOSE") is equivalent to CLOSE()

    :param data: the input data (KData or Indicator) 
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :rtype: Indicator


.. py:function:: KDJ(kdata[, n=9, m12=3, m2=3])

    The classic KDJ stochastic indicator

    :param KData kdata: the associated K-line data
    :param int n:
    :param int m1:
    :param int m2:
    :return: k, d, j

.. py:function:: LIUTONGPAN(kdata)

   Get the circulating share capital (unit: 10,000 shares), the same as CAPITAL
   
   :param KData kdata: the K-line data
   :rtype: Indicator


.. py:function:: LAST([data, m=10, n=5])

    Exists in the interval.

    Usage: LAST (X,M,N) means that the condition X exists from the previous M periods to the previous N periods.

    For example: LAST(CLOSE>OPEN,10,5) means the bullish candles from the previous 10 days to the previous 5 days.

    :param data: the input data
    :param int m: m periods
    :param int n: n periods
    :rtype: Indicator


.. py:function:: LASTVALUE(ind, [ignore_discard=False])

    Equivalent to the TDX CONST indicator. Take the last value of the input indicator as a constant, i.e. all the values in the result are the last value of the input indicator; use with caution. It contains a look-ahead function; use with caution.

    :param Indicator ind: the indicator
    :param bool ignore_discard: ignore the discarded data of the indicator
    :rtype: Indicator


.. py:function:: LLV([data, n=20])

    The lowest price within the N days; when N=0, start from the first valid value.

    :param data: the input data
    :param int|Indicator|IndParam n: the N-day time window
    :rtype: Indicator


.. py:function:: LLVBARS([data, n=20])

    The position of the previous low point, getting the number of the periods from the previous low point to the current.
    
    Usage: LLVBARS(X,N): getting the number of the periods from the lowest value of X within the N periods to the current; N=0 means counting from the first valid value
    
    For example: LLVBARS(HIGH,20) gets the number of the periods from the 20-day lowest point to the current
    
    :param data: the input data
    :param int|Indicator|IndParam n: the N-day time window
    :rtype: Indicator


.. py:function:: LN([data])

    Get the natural logarithm; LN(X) is the logarithm with the base e

    :param data: the input data
    :rtype: Indicator


.. py:function:: LOG([data])

    The logarithm with the base 10

    :param data: the input data
    :rtype: Indicator


.. py:function:: LONGCROSS(a, b[, n=3])

    The two lines cross after maintaining for a certain period

    Usage: LONGCROSS(A,B,N) means that A is less than B within the N periods, and it returns 1 when A crosses B upward from below in this period, otherwise 0

    For example: LONGCROSS(MA(CLOSE,5),MA(CLOSE,10),5) means that the 5-day MA is maintained for 5 periods and then crosses the 10-day MA in a golden cross

    :param Indicator a:
    :param Indicator b:
    :param int n:
    :rtype: Indicator

    
.. py:function:: LOW([data])

    Get the low price; wrap the low price of the KData as an Indicator
    
    :param data: the input data (KData or Indicator) 
    :rtype: Indicator


.. py:function:: MA([data, n=22])

    The simple moving average
    
    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator
    

.. py:function:: MACD([data, n1=12, n2=26, n3=9])

    The Moving Average Convergence Divergence
    
    :param Indicator data: the input data
    :param int n1: the short-term EMA time window
    :param int n2: the long-term EMA time window
    :param int n3: the EMA smoothing time window of (the short-term EMA - the long-term EMA)
    :rtype: an Indicator with three result sets

    * result(0): MACD_BAR: the MACD bar, i.e. the MACD fast line - the MACD slow line
    * result(1): DIFF: the fast line, i.e. (the short-term EMA - the long-term EMA)
    * result(2): DEA: the slow line, i.e. the n3-period EMA smoothing of the fast line


.. py:function:: MAX(ind1, ind2)

    Get the maximum value; MAX(A,B) returns the greater value of A and B.
    
    :param Indicator ind1: A
    :param Indicator ind2: B
    :rtype: Indicator


.. py:function:: MDD([ind, n=0])

    The maximum drawdown percentage (when n=0, there is no time window limit), which is a positive value by the industry convention


.. py:function:: MDD_CURRENT([data])

    The drawdown percentage from the current point to the historical highest point, which is a positive value by the industry convention
    
    The calculation formula: (the historical highest point - the current value) / the historical highest point * 100%
    
    The difference from MDD: what the MDD calculates is the drawdown to the highest point within the specified window, while what the MDD_CURRENT calculates is the drawdown to the historical highest point from the start point of the sequence to the current point, without a time window limit.
    
    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: MIN(ind1, ind2)

    Get the minimum value; MIN(A,B) returns the smaller value of A and B.
    
    :param Indicator ind1: A
    :param Indicator ind2: B
    :rtype: Indicator
    

.. py:function:: MINUTE([data])

    Get the number of the minutes of this period. Usage: MINUTE, the valid value range returned by the function is (0-59), and the value is 0 for the daily and the longer analysis periods.

    :param data: the input data KData
    :rtype: Indicator


.. py:function:: MOD(ind1, ind2)

    Get the modulus after taking the integer. This function is only for the TDX compatibility. In fact, the modulus of the indicators can be obtained directly with the % operator

    Usage: MOD(A,B) returns the modulus of A by B

    For example: MOD(26,10) returns 6

    :param Indicator ind1:
    :param Indicator ind2:
    :rtype: Indicator


.. py:function:: MONTH([data])

    Get the month of this period. Usage: MONTH, the valid value range returned by the function is (1-12).

    :param data: the input data KData
    :rtype: Indicator


.. py:function:: MRR([ind, n=0])

    The maximum profit percentage (calculated in the opposite direction corresponding to MDD)


.. py:function:: NAMELIKE([data,] pattern)

    The security name pattern matching
    
    Return whether the security name in the current context matches the specified pattern. It supports the wildcard matching:
    
    - ``*``: matches any sequence of any characters of any length (including the empty sequence)
    - ``?``: matches a single arbitrary character
    
    The return value: it returns 1.0 when the match succeeds, otherwise 0.0. The return values of all the periods are the same.
    
    Usage example::
    
        # Match the index names starting with "上证"
        kdata = get_kdata('sh000001', Query(-10))
        result = NAMELIKE(kdata, "上证*")
        
        # Match the names containing "指数"
        result = NAMELIKE(kdata, "*指数*")
        
        # Match the names of 4 characters
        result = NAMELIKE(kdata, "????")
        
        # Match a specific name exactly
        result = NAMELIKE(kdata, "上证指数")

    :param KData data: the K-line data (optional; it can be omitted when a context exists)
    :param str pattern: the matching pattern, supporting the wildcards * and ?
    :return: the matching result indicator, with the same values for all the periods (1.0 or 0.0)
    :rtype: Indicator


.. py:function:: NDAY(x, y[, n=3])

    Consecutively greater; NDAY(X,Y,N) means that the condition X>Y persists for N periods

    :param Indicator x:
    :param Indicator y:
    :param int n: the time window
    :rtype: Indicator


.. py:function:: NOT([data])

    Get the logical negation. NOT(X) returns the negation of X, i.e. it returns 1 when X<=0, otherwise 0.
    
    :param Indicator data: the input data
    :rtype: Indicator
    
    
.. py:function:: OPEN([data])

    Get the open price; wrap the open price of the KData as an Indicator
    
    :param data: the input data (KData or Indicator) 
    :rtype: Indicator


.. py:function:: POW(data, n)

    The power
    
    Usage: POW(A,B) returns A to the power of B
    
    For example: POW(CLOSE,3) gets the 3rd power of the close price
    
    :param data: the input data
    :param int|Indicator|IndParam n: the power
    :rtype: Indicator


.. py:function:: SIGNED_POWER(data, n)

    The signed power
    
    Usage: SIGNED_POWER(A,B) returns A to the power of B, but keeping the original sign
    
    For example: SIGNED_POWER(CLOSE,3) gets the 3rd power of the close price, keeping the original sign
    
    :param data: the input data
    :param int|Indicator|IndParam n: the power
    :rtype: Indicator


.. py:function:: PRICELIST([data=None, discard=0, align_dates=None])
    
    Convert a list, a tuple or an Indicator to an ordinary Indicator
    
    :param sequence data: the input data
    :param int discard: how many data to discard at the front
    :param sequence align_dates: the aligned date list; if it is empty, no alignment is performed
    :rtype: Indicator


.. py:function:: RANK(stks, ref_ind[, mode = 0, fill_null = true, market = 'SH'])

    Calculate the ranking of the indicator value in the specified block

    :param stks: the specified security list or Block
    :param ref_ind: the reference indicator
    :param mode: the sorting way: 0-descending ranking (the highest indicator value is ranked 1), 1-ascending ranking (the greater the indicator value, the greater the ranking value), 2-the descending ranking percentage, 3-the ascending ranking percentage, 4-the descending ranking percentage (0~1), 5-the ascending ranking percentage (0~1)
    :param fill_null: whether to fill the missing values
    :param market: the market the block belongs to
    :return: the ranking of the indicator value in the specified block
    :rtype: Indicator
 

.. py:function:: REF([data, n])

    The forward reference (i.e. shifting right), referencing the data several periods ago.
    
    Usage: REF(X,A) references the value of X A periods ago.
    
    :param Indicator data: the input data
    :param int n: reference the value n periods ago, i.e. shifting right by n positions
    :rtype: Indicator


.. py:function:: REFX(n: int)

    The enhancement of REF (do not use it for backtesting), used to get the value of the nth period in the indicator; when n is positive, count forward from the current period; when it is negative, count backward from the current period.

    Note: this function belongs to the look-ahead function and should not be used for backtesting; it is mainly used for the scenarios that need to get the future data, such as AI.

    :param Indicator ind: the indicator
    :param int n: the number of the periods    


.. py:function:: RECOVER_BACKWARD([data])

    Perform the backward adjustment on the input indicator data (CLOSE|OPEN|HIGH|LOW)

    :param Indicator|KData data: only the CLOSE|OPEN|HIGH|LOW indicators are accepted, or a KData (in this case, the close price of the KData is used by default)
    :rtype: Indicator


.. py:function:: RECOVER_FORWARD([data])

    Perform the forward adjustment on the input indicator data (CLOSE|OPEN|HIGH|LOW)

    :param Indicator|KData data: only the CLOSE|OPEN|HIGH|LOW indicators are accepted, or a KData (in this case, the close price of the KData is used by default)
    :rtype: Indicator


.. py:function:: RECOVER_EQUAL_BACKWARD([data])

    Perform the equal-ratio backward adjustment on the input indicator data (CLOSE|OPEN|HIGH|LOW)

    :param Indicator|KData data: only the CLOSE|OPEN|HIGH|LOW indicators are accepted, or a KData (in this case, the close price of the KData is used by default)
    :rtype: Indicator


.. py:function:: RECOVER_EQUAL_FORWARD([data])

    Perform the equal-ratio forward adjustment on the input indicator data (CLOSE|OPEN|HIGH|LOW)

    :param Indicator|KData data: only the CLOSE|OPEN|HIGH|LOW indicators are accepted, or a KData (in this case, the close price of the KData is used by default)
    :rtype: Indicator


.. py:function:: REPLACE(ind, [old_value=constant.nan, new_value=0.0, ignore_discard=False])
          
    Replace the specified values in the indicator, defaulting to replacing the nan values with 0.0.

    :param Indicator ind: the specified indicator
    :param double old_value: the specified value
    :param double new_value: the replacement value
    :param bool ignore_discard: ignore the discarded data of the indicator
    :rtype: Indicator


.. py:function:: RESULT(data, result_ix)

    Return the specified result set in the specified indicator in the way of the formula indicator

    :param Indicator data: the specified indicator
    :param int result_ix: the specified result set
    :rtype: Indicator


.. py:function:: REVERSE([data])

    Get the opposite number; REVERSE(X) returns -X

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: ROC([data, n=10])

    The rate of change indicator: ((price / prevPrice)-1)*100

    :param data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator


.. py:function:: ROCP([data, n=10])

    The rate of change indicator: (price - prevPrice) / prevPrice

    :param data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator


.. py:function:: ROCR([data, n=10])

    The rate of change indicator: (price / prevPrice)

    :param data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator


.. py:function:: ROCR100([data, n=10])

    The rate of change indicator: (price / prevPrice) * 100

    :param data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator


.. py:function:: RSRS_BULL([kdata, n=20, m=60])

    The RSRS right-skewed standard score indicator (level 4), based on the advanced version of the RSRS corrected by the Everbright research report.

    On the basis of the corrected standard score (Z × R²), it is multiplied by the original β once again, amplifying the scores in the strong bull zone and compressing the scores in the weak bear zone, specifically for the long timing.

    **The calculation layers**:
    
    1. **Level 1 (β)**: calculate the β value with the rolling N-day OLS regression, with the formula: High = α + β · Low
    2. **Level 2 (Z)**: the rolling M-day Z-score standardization, solving the problem of the β center drifting in the different stages
    3. **Level 3 (the corrected standard score)**: Z × R², where R² is the regression goodness of fit, filtering the noise with the poor fitting
    4. **Level 4 (the right-skewed correction)**: Z × R² × β, amplifying the strong bull zone

    :param KData kdata: the K-line data
    :param int n: the regression window, defaulting to 20
    :param int m: the Z-score window, defaulting to 60
    :rtype: Indicator

    **The returned results**:

    * result(0): the level 4 corrected value (the right-skewed correction = Z × R² × β)
    * result(1): the β value (the regression slope)
    * result(2): the R² value (the regression goodness of fit)
    * result(3): the Z value (the Z-score standardized value)

    **Usage example**::

        # Get the K-line data
        kdata = get_kdata('sh000001', Query(-1000))
        
        # Calculate the RSRS_BULL indicator (defaulting to n=20, m=60)
        bull = RSRS_BULL(kdata)
        
        # Get the level 4 corrected value (the result set 0)
        bull_value = bull.get(-1, 0)
        
        # Get the β value (the result set 1)
        beta = bull.get(-1, 1)
        
        # Get the R² value (the result set 2)
        r2 = bull.get(-1, 2)
        
        # Get the Z value (the result set 3)
        z_score = bull.get(-1, 3)
        
        # The indicator is valid only when R² > 0.8
        if r2 > 0.8:
            print(f"Valid RSRS_BULL value: {bull_value}")
        else:
            print("The R² is too low, the indicator is not reliable")


.. py:function:: RSRS_BETA([kdata, n=20])

    The original RSRS (the underlying β) indicator, based on the rolling N-day OLS regression.

    Each K-line contributes a coordinate point (Low[i], High[i]), and the N points within the rolling window are used for the OLS regression.
    The formula: High = α + β · Low

    β is the most original RSRS slope, representing the strength of the support and the resistance.
    The defect: the β center fluctuates greatly in the different market ranges, and it cannot be compared directly across the time periods.

    :param KData kdata: the K-line data
    :param int n: the rolling window, defaulting to 20
    :rtype: Indicator

    **The calculation principle**:
    
    1. Take the lowest price Low[i] and the highest price High[i] of each K-line itself, forming the coordinate point (Low[i], High[i])
    2. Use the N points within the rolling window to perform the OLS linear regression, fitting the line High = α + β · Low
    3. The β value is the regression slope, reflecting the strength of the support and the resistance

    **Usage example**::

        # Get the K-line data
        kdata = get_kdata('sh000001', Query(-200))
        
        # Calculate the RSRS_BETA indicator (defaulting to n=20)
        rsrs_beta = RSRS_BETA(kdata)
        
        # Calculate the RSRS_BETA indicator (a custom window size)
        rsrs_beta = RSRS_BETA(kdata, 10)


.. py:function:: ROUND([data, ndigits=2])

    Round half up

    :param data: the input data
    :param int ndigits: the number of the decimal places to keep
    :rtype: Indicator


.. py:function:: ROUNDDOWN([data, ndigits=2])

    Round down, e.g. 10.1 becomes 10 after rounding

    :param data: the input data
    :param int ndigits: the number of the decimal places to keep
    :rtype: Indicator


.. py:function:: ROUNDUP([data, ndigits=2])

    Round up, e.g. 10.1 becomes 11 after rounding

    :param data: the input data
    :param int ndigits: the number of the decimal places to keep
    :rtype: Indicator


.. py:function:: RSI([data, n=14])

    The Relative Strength Index

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator
    

.. py:function:: SAFTYLOSS([data, n1=10, n2=3, p=2.0])

    The Alexander Elder safety zone stop line, see [BOOK2]_
    
    The calculation description: within the lookback period (generally 10 to 20 days), sum all the lengths of the downward penetrations and divide by the number of the downward penetrations to get the average noise (i.e. the total length of all the lowest prices below the lowest price of the previous day within the lookback period divided by the number of the times), and subtract (the average noise of the previous day multiplied by a multiple) from the lowest price of today to get the stop line. To offset the fluctuation and ensure the upward movement of the stop line, take the highest value within the N days (generally 3 days) on the basis of the above result

    :param Indicator data: the input data
    :param int n1: the lookback time window for calculating the average noise
    :param int n2: take the highest value within the n2 days for the preliminary stop line
    :param float p: the noise coefficient
    :rtype: Indicator
    

.. py:function:: SIN([data])

    The sine value

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: SGN([data])

    Get the sign value; SGN(X) returns 1, 0, -1 when X>0, X=0, X<0 respectively.

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: SLICE(data, start, end, result_index=-1)

    Get the data of the specified range [start, end) in an indicator, generating a new indicator

    :param Indicator|sequence data: the input data
    :param int start: the start position
    :param int end: the end position (exclusive)
    :param int result_index: the result set in the original input data; when it is less than 0 by default, it means all the result sets


.. py:function:: SLOPE(data, n=22)

    Calculate the linear regression slope, the goodness of fit R² and the relative maximum residual; N supports the variables

    :param Indicator|sequence data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator

    **The result sets**:

    * result(0): the slope
    * result(1): the goodness of fit R²
    * result(2): the relative maximum residual RelMaxRes = max|yi - ŷi| / ȳ

    **The explanation of the relative maximum residual**:

    * The numerator: the maximum absolute residual within the window (guaranteeing that no point is far away from the regression line)
    * The denominator: the average of y within the window (eliminating the price dimension, with the percentage meaning)
    * The smaller the indicator = all the K-lines of the whole segment are close to the regression line

    **The examples of the judgment thresholds**:

    * RelMaxRes < 0.03: the farthest K-line deviates from the average price by less than 3%, and all the points are close to the regression line


.. py:function:: SMA([data, n=22, m=2])

    Get the moving average

    Usage: if Y=SMA(X,N,M), then Y=[M*X+(N-M)*Y')/N, where Y' represents the Y value of the previous period
    
    :param Indicator data: the input data
    :param int n: the time window
    :param float m: the coefficient
    :rtype: Indicator


.. py:function:: SPEARMAN(ind1, ind2, n)

    The Spearman correlation coefficient

    :param Indicator ind1: the input parameter 1
    :param Indicator ind2: the input parameter 2
    :param int n: the rolling window (greater than 2 or equal to 0); when it is equal to 0, it means that n actually uses the length of the ind


.. py:function:: SKEW([data, n=10])

    Calculate the unadjusted population skewness within the N periods

    :param Indicator data: the input data
    :param int n: the N-day time window (greater than or equal to 3 or equal to 0); when it is equal to 0, use the actual length of the input data
    :rtype: Indicator


.. py:function:: KURT([data, n=10])

    Calculate the excess kurtosis within the N periods (the unadjusted population kurtosis - 3)

    The Excess Kurtosis is an indicator measuring the kurtosis of the data distribution, used to describe the tail thickness of the data distribution.
    The excess kurtosis of the normal distribution is 0. A positive excess kurtosis means that the distribution has a thicker tail than the normal distribution (leptokurtic),
    and a negative excess kurtosis means that the distribution has a thinner tail than the normal distribution (platykurtic).

    :param Indicator data: the input data
    :param int n: the N-day time window (greater than or equal to 4 or equal to 0); when it is equal to 0, use the actual length of the input data
    :rtype: Indicator


.. py:function:: SQRT([data])

    Take the square root

    Usage: SQRT(X) is the square root of X

    For example: SQRT(CLOSE) is the square root of the close price

    :param data: the input data
    :rtype: Indicator


.. py:function:: STD([data, n=10])

    Calculate the sample standard deviation within the N periods
    
    :param Indicator data: the input data
    :param int n|Indicator|IndParam: the time window
    :rtype: Indicator


.. py:function:: STDEV([data, n=10])

    Calculate the sample standard deviation within the N periods
    
    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator
    

.. py:function:: STDP([data, n=10])

    The population standard deviation; STDP(X,N) is the N-day population standard deviation of X
    
    :param data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator


.. py:function:: STKTYPE([k])

    Get the stock type indicator

    Return the type value (the StockType enumeration value) of the current stock

    Example::

        # Use the default context
        stktype = STKTYPE()
        
        # Specify the K-line data
        stock = sm.getStock("sh000001")
        kdata = stock.getKData(Query(0, 100))
        stktype = STKTYPE(kdata)

    :param KData k: the K-line data context
    :rtype: Indicator


.. py:function:: SUM([data, n=20])

    Get the total sum. SUM(X,N), counting the total sum of X within the N periods; when N=0, start from the first valid value.

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator


.. py:function:: SUMBARS([data,] a)

    Accumulate to the specified number of the periods, the number of the periods from accumulating forward to the specified value until now

    Usage: SUMBARS(X,A): accumulate X forward until it is greater than or equal to A, returning the number of the periods of this interval

    For example: SUMBARS(VOL,CAPITAL) gets the number of the periods from a complete turnover to now

    :param Indicator data: the input data
    :param float|Indicator|IndParam a: the specified accumulated sum
    :rtype: Indicator


.. py:function:: TAN([data])

    The tangent value

    :param Indicator data: the input data
    :rtype: Indicator


.. py:function:: TIME([data])

    Get the hour-minute-second of this period. Usage: TIME, the valid value range returned by the function is (000000-235959).

    :param data: the input data KData
    :rtype: Indicator


.. py:function:: TIMELINE([k])

    The time-line price data

    :param KData k: the context
    :rtype: Indicator
    

.. py:function:: TIMELINEVOL([k])

    The time-line volume data

    :param KData k: the context
    :rtype: Indicator


.. py:function:: TR([kdata])

    The true range (TR) is the maximum of the following three values:
    1. The difference between the high price and the low price of the current period
    2. The absolute value of the difference between the high price of the current period and the close price of the previous period
    3. The absolute value of the difference between the low price of the current period and the close price of the previous period

    :param KData kdata: the K-line data
    :rtype: Indicator


.. py:function:: TS_RANK([data, n=20])

    The time series ranking, calculating the ranking ratio of the current value within the past N periods (Alpha101)

    Usage: TS_RANK(X,N) means the ranking of X within the past N periods (from 1 to N) divided by N.
    For example: TS_RANK(CLOSE,20) means the ranking ratio of the close price within the past 20 periods.

    The definition in Alpha101:
    TS_RANK(x, n) = (rank of x in the last n observations) / n
    where the rank is the ascending ranking, i.e. the smaller values are ranked higher, and the output is the percentile value of 0~1.

    This indicator is the rolling ranking of a single stock on the time axis, used to measure the relative position of the current value in the historical sequence.

    :param Indicator data: the data to calculate
    :param int n: the number of the periods, defaulting to 20
    :rtype: Indicator


.. py:function:: UPNDAY(data[, n=3])

    The number of the consecutive rising periods; UPNDAY(CLOSE,M) means M consecutive rising periods

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator


.. py:function:: VAR([data, n=10])

    The estimated sample variance; VAR(X,N) is the N-day estimated sample variance of X

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator


.. py:function:: VARP([data, n=10])

    The population sample variance; VARP(X,N) is the N-day population sample variance of X

    :param Indicator data: the input data
    :param int|Indicator|IndParam n: the time window
    :rtype: Indicator


.. py:function:: VIGOR([kdata, n=2])

    The Alexander Elder force index [BOOK2]_
    
    The calculation formula: (the close price today - the close price yesterday) * the volume today
    
    :param KData data: the input data
    :param int n: the EMA smoothing window
    :rtype: Indicator
    
    
.. py:function:: VOL([data])

    Get the volume; wrap the volume of the KData as an Indicator

    :param data: the input data (KData or Indicator)
    :rtype: Indicator


.. py:function:: WEAVE(ind1, ind2[, ind3, ind4, ind5, ind6])

    Combine the results of at most 6 Indicators together in one Indicator. E.g. ind = WEAVE(ind1, ind2), then ind contains the multiple results at this time, stored in the order of ind1 and ind2.
    
    :param Indicator ind1: the indicator 1
    :param Indicator ind2: the indicator 2
    :param Indicator ind3: the indicator 3, which can be omitted
    :param Indicator ind4: the indicator 4, which can be omitted
    :param Indicator ind5: the indicator 5, which can be omitted
    :param Indicator ind6: the indicator 6, which can be omitted
    :rtype: Indicator


.. py:function:: WEEK([data])

    Get the day of the week of this period. Usage: WEEK, the valid value range returned by the function is (0-6), and 0 means Sunday.

    :param data: the input data KData
    :rtype: Indicator


.. py:function:: WINNER([ind])
    
    The profitable position ratio

    Usage: WINNER(CLOSE) means the ratio of the profitable positions sold at the current closing price.

    For example: returning 0.1 means 10% of the profitable positions; WINNER(10.5) means the ratio of the profitable positions at the price of 10.5 yuan

    This function is only valid for the daily analysis period, and only for the securities with the circulating share capital dividend data; it is invalid for the indexes, the funds, etc.


.. py:function:: WITHKTYPE([ind, ktype, fill_null=False])

    Convert the indicator data to the specified K-line type. The related convenience functions: WITHWEEK, WITHMONTH, WITHQUARTER, WITHYEAR, WITHMIN, WITHMIN5 ...

    :param Indicator ind: the indicator data
    :param bool fill_null: whether to fill the empty values
    :rtype: Indicator


.. py:function:: YEAR([data])

    Get the year of this period.

    :param data: the input data KData
    :rtype: Indicator


.. py:function:: ZHBOND10([data, default_val])

    Get the 10-year Chinese treasury bond yield

    :param DatetimeList|KDate|Indicator data: the input date reference, preferring the dates in the context
    :param float default_val: if the input date is earlier than the earliest record of the existing treasury bond data, this default value is used


.. py:function:: ZONGGUBEN([data])

   Get the total share capital (unit: 10,000 shares)

   :param KData kdata: the K-line data
   :rtype: Indicator    


.. py:function:: ZSCORE([data, out_extreme, nsigma, recursive])

    Standardize (normalize) the data, with the optional outlier exclusion

    Note: it is not a window rolling; if the standardization with the window rolling is needed, just use (x - MA(x, n)) / STDEV(x, n).
    
    :param Indicator data: the data from which the outliers are to be removed
    :param bool outExtreme: indicate removing the extreme values, defaulting to False
    :param float nsigma: the multiple of sigma used when removing the extreme values, defaulting to 3.0
    :param bool recursive: whether to remove the extreme values recursively, defaulting to False
    :rtype: Indicator
