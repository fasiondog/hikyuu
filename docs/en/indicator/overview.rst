.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

Technical Indicator Overview
============================

**Auxiliary indicators**

* :py:func:`ALIGN` - align by the specified reference dates
* :py:func:`CODELIKE` - the security code pattern matching
* :py:func:`CYCLE` - the PF rebalance cycle indicator, mainly used for the PF rebalance day verification, and as an SG
* :py:func:`CVAL` - create a fixed-value indicator of the specified length
* :py:func:`CONTEXT` - an independent context
* :py:func:`DISCARD` - set the amount of the discarded data of the indicator result in the way of the indicator formula.
* :py:func:`DROPNA` - remove the nan values
* :py:func:`FIXED_START_INDEX` - [Donating user] fix the start index of the query range used when calculating the indicator
* :py:func:`FIXED_START_DATE` - [Donating user] fix the start date of the query range used when calculating the indicator
* :py:func:`INBLOCK` - whether the security of the current context is in the specified block.
* :py:func:`ISNA` - judge whether it is a nan value
* :py:func:`ISINF` - judge whether it is a +inf value
* :py:func:`ISINFA` - judge whether it is a -inf value
* :py:func:`JUMPDOWN` - the edge jump, jumping from <= 0.0 to > 0.0
* :py:func:`JUMPUP` - the edge jump, jumping from > 0.0 to <= 0.0
* :py:func:`LASTVALUE` - equivalent to the TDX CONST indicator. Take the last value of the input indicator as a constant, i.e. all the values in the result are the last value of the input indicator; use with caution. It contains a look-ahead function; use with caution.
* :py:func:`NAMELIKE` - the security name pattern matching
* :py:func:`PRICELIST` - wrap a PriceList or the result set of an Indicator as an Indicator; the same name: VALUE
* :py:func:`REF` - the forward reference (i.e. shifting right), referencing the data several periods ago
* :py:func:`REFX` - the enhancement of REF, which can shift left or right. When shifting left, it is a look-ahead function; do not use it for backtesting.
* :py:func:`REPLACE` - replace the specified values in the indicator, defaulting to replacing the nan values with 0.0
* :py:func:`RESULT` - return the corresponding result set in the specified indicator in the way of the indicator formula
* :py:func:`SLICE` - get the data of the specified range [start, end) in an indicator, generating a new indicator
* :py:func:`WEAVE` - merge the results of the two inds into one ind
* :py:func:`WITHKTYPE` - [Donating user] convert the indicator data to the specified K-line type. The related convenience functions: WITHWEEK, WITHMONTH, WITHQUARTER, WITHYEAR, WITHMIN, WITHMIN5 ...
* :py:func:`ZSCORE` - the ZScore standardization


**Market indicators**

* :py:func:`KDATA` - wrap the KData as an Indicator, used for the calculation of the other indicators
* :py:func:`KDATA_PART` - select and return the indicator KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL by the string
* :py:func:`OPEN`  - wrap the open price of the KData as an Indicator
* :py:func:`HIGH`  - wrap the high price of the KData as an Indicator
* :py:func:`LOW`   - wrap the low price of the KData as an Indicator
* :py:func:`CLOSE` - wrap the close price of the KData as an Indicator
* :py:func:`AMO`   - wrap the amount of the KData as an Indicator
* :py:func:`VOL`   - wrap the volume of the KData as an Indicator
* :py:func:`ADJ_FACTOR` - the adjustment factor (used together with the factor management system)
* :py:func:`ADJ_OPEN` - the adjusted open price
* :py:func:`ADJ_HIGH` - the adjusted high price
* :py:func:`ADJ_LOW` - the adjusted low price
* :py:func:`ADJ_CLOSE` - the adjusted close price
* :py:func:`ADJ_VOL` - the adjusted volume
* :py:func:`RECOVER_FORWARD` - the forward adjustment
* :py:func:`RECOVER_BACKWARD` - the backward adjustment
* :py:func:`RECOVER_EQUAL_FORWARD` - the equal-ratio forward adjustment
* :py:func:`RECOVER_EQUAL_BACKWARD` - the equal-ratio backward adjustment
* :py:func:`FINANCE` - the historical finance information
* :py:func:`HSL` - the turnover rate
* :py:func:`CAPITAL` - the circulating share capital, the same name: LIUTONGPAN
* :py:func:`TIMELINE`   - the time-line price
* :py:func:`TIMELINEVOL`   - the time-line volume
* :py:func:`ZHBOND10` - the 10-year Chinese treasury bond yield
* :py:func:`ZONGGUBEN` - the total share capital
* :py:func:`STKTYPE` - the stock type indicator
* :py:func:`ISLIMITUP` - judge whether it is the limit up
* :py:func:`ISLIMITDOWN` - judge whether it is the limit down
    

**Market-wide indicators**

* :py:func:`ADVANCE` - the number of the rising stocks
* :py:func:`DECLINE` - the number of the falling stocks
* :py:func:`INDEXO` - return the corresponding market open prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index
* :py:func:`INDEXH` - return the corresponding market high prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index
* :py:func:`INDEXL` - return the corresponding market low prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index
* :py:func:`INDEXC` - return the corresponding market close prices, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index
* :py:func:`INDEXA` - return the corresponding market amounts, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index
* :py:func:`INDEXV` - return the corresponding market volumes, which are the SSE Composite Index, the SZSE Component Index, the STAR 50 and the ChiNext Index
* :py:func:`INDEXADV` - the TDX 880005 market-wide number of the rising stocks, which may not be updated intraday!
* :py:func:`INDEXDEC` - the TDX 880005 market-wide number of the falling stocks, which may not be updated intraday!


**Logical Arithmetic Functions**

The indicators themselves directly support the "+", "-", "*", "/", "&" (and), "|" (or), "<", ">", "<=", ">=", "==", "!=" operations.

* :py:func:`BETWEEN` - between (between two numbers)
* :py:func:`CEILING` - round up (rounding in the direction of increasing the value) to an integer
* :py:func:`CROSS` - the cross function
* :py:func:`DOWNNDAY` - the number of the consecutive falling periods
* :py:func:`EVERY` - always exists
* :py:func:`EXIST` - exists; EXIST(X,N) means that the condition X exists within the N periods
* :py:func:`FLOOR` - round down (rounding in the direction of decreasing the value) to an integer
* :py:func:`IF` - get the different values according to the condition
* :py:func:`INTPART` - take the integer part (rounding by decreasing the absolute value, i.e. getting the integer part of the data)
* :py:func:`LAST` - exists in the interval
* :py:func:`LONGCROSS` - the two lines cross after maintaining for a certain period
* :py:func:`NOT` - get the logical negation
* :py:func:`UPNDAY` - the number of the consecutive rising periods
* :py:func:`NDAY` - consecutively greater


**Mathematical Indicators**

* :py:func:`ABS` - get the absolute value
* :py:func:`ACOS` - the arccosine value
* :py:func:`ASIN` - the arcsine value
* :py:func:`ATAN` - the arctangent value
* :py:func:`COS` - the cosine value
* :py:func:`EXP` - e to the power of X
* :py:func:`LN` - get the natural logarithm; LN(X) is the logarithm with the base e
* :py:func:`LOG` - the logarithm with the base 10
* :py:func:`MAX` - the maximum value
* :py:func:`MIN` - the minimum value
* :py:func:`MOD` - get the modulus after taking the integer. This function is only for the TDX compatibility. In fact, the modulus of the indicators can be obtained directly with the % operator.
* :py:func:`POW` - the power
* :py:func:`SIGNED_POWER` - the signed power
* :py:func:`REVERSE` - get the opposite number
* :py:func:`ROUND` - round half up
* :py:func:`ROUNDUP` -  round up, e.g. 10.1 becomes 11 after rounding
* :py:func:`ROUNDDOWN` - round down, e.g. 10.1 becomes 10 after rounding
* :py:func:`SIN` - the sine value
* :py:func:`SGN` - get the sign value
* :py:func:`SLOPE` - calculate the linear regression slope, the goodness of fit R² and the relative maximum residual
* :py:func:`SQRT` - take the square root
* :py:func:`TAN` - the tangent value

**Statistical Indicators**

* :py:func:`AVEDEV` - the average absolute deviation
* :py:func:`BETA` - the Beta coefficient, measuring the sensitivity between the asset return and the market return
* :py:func:`DEVSQ` - the sum of the squared deviations of the data
* :py:func:`STD` - the estimated standard deviation, the same as STDEV
* :py:func:`STDEV` - calculate the sample standard deviation within the N periods
* :py:func:`STDP` - the population standard deviation
* :py:func:`VAR` - the estimated sample variance
* :py:func:`VARP` - the population sample variance
* :py:func:`CORR` - the sample correlation coefficient and the covariance
* :py:func:`COV` - the sample covariance
* :py:func:`SPEARMAN` - the Spearman correlation coefficient
* :py:func:`SKEW` - the population skewness
* :py:func:`KURT` - the excess kurtosis

**Cross-sectional Statistics**

* :py:func:`BLOCKSETNUM` - return the number of the stocks in the block
* :py:func:`INSUM` - return the calculated values of the corresponding outputs of this indicator of each component in the block by the calculation type. The calculation types: 0-accumulation, 1-average, 2-maximum, 3-minimum, 4-ranking (1 corresponds to the lowest indicator value), 5-ranking (starting from 1, corresponding to the highest indicator value).
* :py:func:`RANK` - [Donating user] calculate the ranking of the indicator value in the specified block

**Technical Indicators**

* :py:func:`AD` - the accumulation/distribution line
* :py:func:`ADX` - the Average Directional Index, the trend strength indicator (Wilder smoothing)
* :py:func:`ADX2` - the Average Directional Index (ADX2), the trend strength indicator (EMA smoothing)
* :py:func:`AMA` - the Perry J. Kaufman adaptive moving average [BOOK1]_
* :py:func:`ATR` - the average true range; the simple moving average of the true range TR
* :py:func:`BACKSET` - the forward assignment, setting the data from the current position to several periods ago to 1
* :py:func:`BARSCOUNT` - the number of the valid periods, getting the total number of the periods.
* :py:func:`BARSLAST` - the position of the last time the condition was satisfied, the number of the periods from the last time the condition was satisfied to the current
* :py:func:`BARSLASTS` - the number of the periods from the Nth time the condition was satisfied to the current (supporting the dynamic parameters)
* :py:func:`BARSLASTCOUNT` - count the number of the periods continuously satisfying the condition
* :py:func:`BARSSINCE` - the number of the periods from the first time the condition was satisfied to the current
* :py:func:`COUNT` - count the number of the periods satisfying the condition
* :py:func:`COST` - the cost distribution
* :py:func:`DIFF` - the difference indicator, i.e. data[i] - data[i-n]
* :py:func:`DMA` - the dynamic moving average
* :py:func:`EMA` - the Exponential Moving Average
* :py:func:`FILTER` - the signal filter, filtering the signals that appear consecutively
* :py:func:`HHV` - the highest price within the N days
* :py:func:`HHVBARS` - the position of the previous high point, getting the number of the periods from the previous high point to the current
* :py:func:`KALMAN` - the Kalman filter
* :py:func:`KDJ` - the classic stochastic indicator
* :py:func:`LLV` - the lowest price within the N days
* :py:func:`LLVBARS` - the position of the previous low point, getting the number of the periods from the previous low point to the current
* :py:func:`MA`  - the simple moving average
* :py:func:`MACD` - the Moving Average Convergence Divergence
* :py:func:`MDD` - the maximum drawdown percentage
* :py:func:`MDD_CURRENT` - the drawdown percentage from the current point to the historical highest point
* :py:func:`ROC` - the rate of change indicator: ((price / prevPrice)-1)*100
* :py:func:`ROCP` - the rate of change indicator: (price - prevPrice) / prevPrice
* :py:func:`ROCR` - the rate of change indicator: (price / prevPrice)
* :py:func:`ROCR100` - the rate of change indicator: (price / prevPrice) * 100
* :py:func:`RSRS_BULL` - the RSRS right-skewed standard score indicator (level 4), based on the advanced version of the RSRS corrected by the Everbright research report
* :py:func:`RSRS_BETA` - the original RSRS (the underlying β) indicator, based on the rolling N-day OLS regression
* :py:func:`RSI` - the Relative Strength Index
* :py:func:`SMA` - the moving average
* :py:func:`SAFTYLOSS` - the Alexander Elder safety zone stop line
* :py:func:`SUM` - get the total sum
* :py:func:`SUMBARS` - accumulate to the specified number of the periods, the number of the periods from accumulating forward to the specified value until now
* :py:func:`TR` - the true range
* :py:func:`TS_RANK` - the time series ranking (Alpha101), calculating the ranking ratio of the current value within the past N periods
* :py:func:`VIGOR` - the Alexander Elder force index


**Time Indicators**

* :py:func:`DATE` - get the year-month-day of this period since 1900
* :py:func:`TIME` - get the hour-minute-second of this period
* :py:func:`YEAR` - get the year of this period
* :py:func:`MONTH` - get the month of this period
* :py:func:`WEEK` - get the day of the week of this period; the valid value range returned by the function is (0-6), and 0 means Sunday
* :py:func:`DAY` - get the date of this period
* :py:func:`HOUR` - get the number of the hours of this period
* :py:func:`MINUTE` - get the number of the minutes of this period

**Factor Indicators**

* :py:func:`FACTOR` - the factor indicator conversion, converting a Factor object to an Indicator
* :py:func:`IC` - calculate the factor IC value
* :py:func:`IR` - used to calculate the IR of the account return and the reference return
* :py:func:`ICIR` - calculate the IR value of the factor IC


**Aggregation Indicators [Donating user]**

* :py:func:`AGG_COUNT` - the aggregation function: the non-empty value counting
* :py:func:`AGG_MAD` - the aggregation function: the average absolute deviation
* :py:func:`AGG_MAX` - the aggregation function: the maximum value
* :py:func:`AGG_MIN` - the aggregation function: the maximum value
* :py:func:`AGG_MEAN` - the aggregation function: the average value
* :py:func:`AGG_MEDIAN` - the aggregation function: the median
* :py:func:`AGG_PROD` - the aggregation function: the product
* :py:func:`AGG_SAMPLE` - the aggregation function: the time sampling (sampling the indicator data at the specified time points)
* :py:func:`AGG_SAMPLE_MAX` - the aggregation function: the maximum in the time period (counting the maximum of the indicator data within the specified time period)
* :py:func:`AGG_SAMPLE_MIN` - the aggregation function: the minimum in the time period (counting the minimum of the indicator data within the specified time period)
* :py:func:`AGG_SAMPLE_MEAN` - the aggregation function: the average in the time period (counting the average of the indicator data within the specified time period)
* :py:func:`AGG_SUM` - the aggregation function: the total sum
* :py:func:`AGG_STD` - the aggregation function: the standard deviation
* :py:func:`AGG_VAR` - the aggregation function: the variance
* :py:func:`AGG_QUANTILE` - the aggregation function: the quantile
* :py:func:`AGG_VWAP` - the aggregation function: the volume-weighted average price
* :py:func:`AGG_FUNC` - the aggregation function: the custom aggregation function

**Grouping Indicators [Donating user]**

* :py:func:`GROUP_COUNT` - the grouping function: the non-empty value counting
* :py:func:`GROUP_MAX` - the grouping function: the group cumulative maximum
* :py:func:`GROUP_MIN` - the grouping function: the group cumulative minimum
* :py:func:`GROUP_MEAN` - the grouping function: the group cumulative average
* :py:func:`GROUP_PROD` - the grouping function: the group cumulative product
* :py:func:`GROUP_SUM` - the grouping function: the group cumulative total sum
* :py:func:`GROUP_FUNC` - the grouping function: the custom grouping calculation function


**Other Conversion Aids**

* :py:func:`concat_to_df` - merge the indicator list into a DataFrame
* :py:func:`df_to_ind` - convert the specified column of a DataFrame to an indicator
