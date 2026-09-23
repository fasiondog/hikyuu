.. py:currentmodule:: hikyuu.indicator
.. highlight:: python

Technical Indicator Overview
============================

**Auxiliary Indicators**

* :py:func:`ALIGN` - align the data to the specified reference dates
* :py:func:`CODELIKE` - pattern matching against the security code
* :py:func:`CYCLE` - the portfolio (PF) rebalance-cycle indicator, used mainly for rebalancing-day validation in a portfolio and as a signal generator (SG)
* :py:func:`CVAL` - create a constant-value indicator of the specified length
* :py:func:`CONTEXT` - a standalone context
* :py:func:`DISCARD` - set the discarded (warm-up) length of an indicator's result using indicator formula syntax
* :py:func:`DROPNA` - remove NaN values
* :py:func:`FIXED_START_INDEX` - [supporter-only] pin the start index of the query range used when the indicator is calculated
* :py:func:`FIXED_START_DATE` - [supporter-only] pin the start date of the query range used when the indicator is calculated
* :py:func:`INBLOCK` - whether the security in the current context belongs to the specified sector
* :py:func:`ISNA` - test whether a value is NaN
* :py:func:`ISINF` - test whether a value is positive infinity (+inf)
* :py:func:`ISINFA` - test whether a value is negative infinity (-inf)
* :py:func:`JUMPDOWN` - falling-edge detector: outputs 1.0 when the series crosses from > 0.0 to <= 0.0
* :py:func:`JUMPUP` - rising-edge detector: outputs 1.0 when the series crosses from <= 0.0 to > 0.0
* :py:func:`LASTVALUE` - equivalent to the TDX CONST indicator: broadcasts the final value of the input indicator across the entire result, so every output value equals that final value. It contains a look-ahead bias; do not use it in backtests.
* :py:func:`NAMELIKE` - pattern matching against the security name
* :py:func:`PRICELIST` - wrap a PriceList or the result set of an Indicator as an Indicator; alias: VALUE
* :py:func:`REF` - backward reference (a right shift): the data from N periods earlier
* :py:func:`REFX` - an enhanced REF that can shift left or right. Shifting left introduces look-ahead data; do not use it for backtesting.
* :py:func:`REPLACE` - replace specified values in an indicator; by default replaces NaN with 0.0
* :py:func:`RESULT` - return the corresponding result set of the specified indicator using indicator formula syntax
* :py:func:`SLICE` - extract the data over the half-open range [start, end) of an indicator into a new indicator
* :py:func:`WEAVE` - merge the result sets of two indicators into one
* :py:func:`WITHKTYPE` - [supporter-only] convert the indicator data to the specified bar (K-line/candlestick) type. Convenience wrappers: WITHWEEK, WITHMONTH, WITHQUARTER, WITHYEAR, WITHMIN, WITHMIN5 ...
* :py:func:`ZSCORE` - Z-score normalization


**Market Data Indicators**

* :py:func:`KDATA` - wrap KData (bar data) as an Indicator for use in other indicator calculations
* :py:func:`KDATA_PART` - select and return one of KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL by its name string
* :py:func:`OPEN`  - wrap the open price of the KData as an Indicator
* :py:func:`HIGH`  - wrap the high price of the KData as an Indicator
* :py:func:`LOW`   - wrap the low price of the KData as an Indicator
* :py:func:`CLOSE` - wrap the close price of the KData as an Indicator
* :py:func:`AMO`   - wrap the trading value (amount) of the KData as an Indicator
* :py:func:`VOL`   - wrap the volume of the KData as an Indicator
* :py:func:`ADJ_FACTOR` - the adjustment factor (used together with the factor management system)
* :py:func:`ADJ_OPEN` - the adjusted open price
* :py:func:`ADJ_HIGH` - the adjusted high price
* :py:func:`ADJ_LOW` - the adjusted low price
* :py:func:`ADJ_CLOSE` - the adjusted close price
* :py:func:`ADJ_VOL` - the adjusted volume
* :py:func:`RECOVER_FORWARD` - forward adjustment
* :py:func:`RECOVER_BACKWARD` - backward adjustment
* :py:func:`RECOVER_EQUAL_FORWARD` - equal-weight forward adjustment
* :py:func:`RECOVER_EQUAL_BACKWARD` - equal-weight backward adjustment
* :py:func:`FINANCE` - historical financial data
* :py:func:`HSL` - turnover rate
* :py:func:`CAPITAL` - the floating (tradable) share capital; alias: LIUTONGPAN
* :py:func:`TIMELINE`   - intraday timeline price
* :py:func:`TIMELINEVOL`   - intraday timeline volume
* :py:func:`ZHBOND10` - the China 10-year government bond yield
* :py:func:`ZONGGUBEN` - the total share capital
* :py:func:`STKTYPE` - the security type indicator
* :py:func:`ISLIMITUP` - test whether the security is at its limit-up price
* :py:func:`ISLIMITDOWN` - test whether the security is at its limit-down price


**Market-Wide and Index Indicators**

* :py:func:`ADVANCE` - the number of advancing (rising) securities
* :py:func:`DECLINE` - the number of declining (falling) securities
* :py:func:`INDEXO` - return the open prices of the corresponding market indices: the SSE Composite Index, SZSE Component Index, STAR 50 Index, and ChiNext Index
* :py:func:`INDEXH` - return the high prices of the corresponding market indices: the SSE Composite Index, SZSE Component Index, STAR 50 Index, and ChiNext Index
* :py:func:`INDEXL` - return the low prices of the corresponding market indices: the SSE Composite Index, SZSE Component Index, STAR 50 Index, and ChiNext Index
* :py:func:`INDEXC` - return the close prices of the corresponding market indices: the SSE Composite Index, SZSE Component Index, STAR 50 Index, and ChiNext Index
* :py:func:`INDEXA` - return the trading values of the corresponding market indices: the SSE Composite Index, SZSE Component Index, STAR 50 Index, and ChiNext Index
* :py:func:`INDEXV` - return the volumes of the corresponding market indices: the SSE Composite Index, SZSE Component Index, STAR 50 Index, and ChiNext Index
* :py:func:`INDEXADV` - the TDX 880005 market-wide advancing-security count, which may not be updated intraday
* :py:func:`INDEXDEC` - the TDX 880005 market-wide declining-security count, which may not be updated intraday


**Logical and Comparison Functions**

Indicators natively support the operators "+", "-", "*", "/", "&" (logical and), "|" (logical or), "<", ">", "<=", ">=", "==" and "!=".

* :py:func:`BETWEEN` - test whether a value lies between two given numbers
* :py:func:`CEILING` - round toward positive infinity to the nearest integer
* :py:func:`CROSS` - crossing detector: whether one series crosses above the other
* :py:func:`DOWNNDAY` - consecutive-down indicator: 1 when the input has fallen over each of the last N periods
* :py:func:`EVERY` - all-periods test: EVERY(X, N) outputs 1 when X is true (nonzero) in every one of the most recent N periods; N = 0 means every period from the first valid bar up to the current bar
* :py:func:`EXIST` - within-window existence: EXIST(X, N) outputs 1 when condition X has been true at least once during the last N periods
* :py:func:`FLOOR` - round toward negative infinity to the nearest integer
* :py:func:`IF` - select different values according to a condition
* :py:func:`INTPART` - take the integer part (truncate toward zero)
* :py:func:`LAST` - interval-persistence test: LAST(X, M, N) outputs 1 when condition X holds on every bar from M periods ago through N periods ago (M >= N); M = 0 starts from the first bar and N = 0 ends at the current bar
* :py:func:`LONGCROSS` - sustained-then-cross detector: two lines cross only after one has stayed on one side of the other for a given number of periods
* :py:func:`NOT` - logical negation
* :py:func:`UPNDAY` - consecutive-up indicator: 1 when the input has risen over each of the last N periods
* :py:func:`NDAY` - consecutive comparison: NDAY(X, Y, N) outputs 1 when X has been greater than Y for N consecutive periods


**Mathematical Indicators**

* :py:func:`ABS` - absolute value
* :py:func:`ACOS` - arccosine
* :py:func:`ASIN` - arcsine
* :py:func:`ATAN` - arctangent
* :py:func:`COS` - cosine
* :py:func:`EXP` - e raised to the power X
* :py:func:`LN` - natural logarithm; LN(X) is the logarithm with base e
* :py:func:`LOG` - base-10 logarithm
* :py:func:`MAX` - maximum value
* :py:func:`MIN` - minimum value
* :py:func:`MOD` - modulo after truncating to an integer. Provided only for TDX compatibility; in practice the % operator can be applied directly to indicators.
* :py:func:`POW` - power
* :py:func:`SIGNED_POWER` - signed power (preserves the sign of the base)
* :py:func:`REVERSE` - arithmetic negation (the opposite number)
* :py:func:`ROUND` - round half up
* :py:func:`ROUNDUP` - round up toward positive infinity, e.g. 10.1 becomes 11
* :py:func:`ROUNDDOWN` - round down toward zero, e.g. 10.1 becomes 10
* :py:func:`SIN` - sine
* :py:func:`SGN` - sign function
* :py:func:`SLOPE` - linear-regression slope, goodness of fit R², and relative maximum residual
* :py:func:`SQRT` - square root
* :py:func:`TAN` - tangent

**Statistical Indicators**

* :py:func:`AVEDEV` - average absolute deviation
* :py:func:`BETA` - beta coefficient, measuring the sensitivity of the asset's returns to the market returns
* :py:func:`DEVSQ` - sum of squared deviations
* :py:func:`STD` - estimated standard deviation; identical to STDEV
* :py:func:`STDEV` - sample standard deviation over N periods
* :py:func:`STDP` - population standard deviation
* :py:func:`VAR` - estimated sample variance
* :py:func:`VARP` - population variance
* :py:func:`CORR` - sample correlation coefficient and covariance
* :py:func:`COV` - sample covariance
* :py:func:`SPEARMAN` - Spearman rank correlation coefficient
* :py:func:`SKEW` - population skewness
* :py:func:`KURT` - excess kurtosis

**Cross-Sectional Statistics**

* :py:func:`BLOCKSETNUM` - return the number of securities in the sector
* :py:func:`INSUM` - aggregate the corresponding output of this indicator across every constituent of the sector according to a calculation type. Types: 0-sum, 1-mean, 2-maximum, 3-minimum, 4-rank (1 = lowest indicator value), 5-rank (starting from 1 for the highest indicator value).
* :py:func:`RANK` - [supporter-only] calculate the rank of the indicator value within the specified sector

**Technical Indicators**

* :py:func:`AD` - accumulation/distribution line
* :py:func:`ADX` - Average Directional Index, a trend-strength indicator (Wilder smoothing)
* :py:func:`ADX2` - Average Directional Index (ADX2), a trend-strength indicator (EMA smoothing)
* :py:func:`AMA` - Perry J. Kaufman's adaptive moving average [BOOK1]_
* :py:func:`ATR` - average true range: the simple moving average of the true range TR
* :py:func:`BACKSET` - backward assignment: when the condition holds, set the current bar and the preceding N-1 bars to 1
* :py:func:`BARSCOUNT` - the number of valid bars, i.e. the total number of periods with data
* :py:func:`BARSLAST` - bars since the most recent occurrence: the number of periods from the last bar on which the condition was true to the current bar
* :py:func:`BARSLASTS` - the number of periods from the Nth most recent occurrence of the condition to the current bar (supports dynamic parameters)
* :py:func:`BARSLASTCOUNT` - count how many consecutive bars satisfy the condition
* :py:func:`BARSSINCE` - the number of periods from the first bar on which the condition was true to the current bar
* :py:func:`COUNT` - count the number of periods that satisfy the condition
* :py:func:`COST` - cost distribution
* :py:func:`DIFF` - period difference, i.e. data[i] - data[i-n]
* :py:func:`DMA` - dynamic moving average
* :py:func:`EMA` - Exponential Moving Average
* :py:func:`FILTER` - signal filter: suppress signals that appear consecutively
* :py:func:`HHV` - highest value over the past N periods
* :py:func:`HHVBARS` - bars since the most recent highest value: the number of periods from the previous peak to the current bar
* :py:func:`KALMAN` - Kalman filter
* :py:func:`KDJ` - the classic stochastic oscillator
* :py:func:`LLV` - lowest value over the past N periods
* :py:func:`LLVBARS` - bars since the most recent lowest value: the number of periods from the previous trough to the current bar
* :py:func:`MA`  - simple moving average
* :py:func:`MACD` - Moving Average Convergence Divergence
* :py:func:`MDD` - maximum drawdown percentage
* :py:func:`MDD_CURRENT` - drawdown percentage from the current bar back to the historical peak
* :py:func:`ROC` - rate of change: ((price / prevPrice) - 1) * 100
* :py:func:`ROCP` - rate of change: (price - prevPrice) / prevPrice
* :py:func:`ROCR` - rate of change: price / prevPrice
* :py:func:`ROCR100` - rate of change: (price / prevPrice) * 100
* :py:func:`RSRS_BULL` - the RSRS right-skew standard-score indicator (level 4), based on the enhanced RSRS corrected in the Everbright Securities research report
* :py:func:`RSRS_BETA` - the original RSRS (underlying beta) indicator, based on rolling N-day OLS regression
* :py:func:`RSI` - Relative Strength Index
* :py:func:`SMA` - moving average
* :py:func:`SAFTYLOSS` - Alexander Elder's SafeZone stop line
* :py:func:`SUM` - cumulative sum
* :py:func:`SUMBARS` - bars-to-target: the number of periods needed for the cumulative sum counted backward from the current bar to reach a specified value
* :py:func:`TR` - true range
* :py:func:`TS_RANK` - time-series rank (Alpha101): the percentile rank of the current value within the past N periods
* :py:func:`VIGOR` - Alexander Elder's Force Index


**Date and Time Indicators**

* :py:func:`DATE` - the year-month-day of the bar, encoded as the number of years elapsed since 1900
* :py:func:`TIME` - the hour-minute-second of the bar
* :py:func:`YEAR` - the year of the bar
* :py:func:`MONTH` - the month of the bar
* :py:func:`WEEK` - the weekday of the bar; valid return values are 0-6, with 0 meaning Sunday
* :py:func:`DAY` - the day of the month of the bar
* :py:func:`HOUR` - the hour of the bar
* :py:func:`MINUTE` - the minute of the bar

**Factor Indicators**

* :py:func:`FACTOR` - factor conversion: turn a Factor object into an Indicator
* :py:func:`IC` - calculate the factor IC (information coefficient)
* :py:func:`IR` - calculate the information ratio of the account returns against a reference return
* :py:func:`ICIR` - calculate the IR of the factor IC series


**Aggregation Indicators [supporter-only]**

* :py:func:`AGG_COUNT` - aggregation: count of non-null values
* :py:func:`AGG_MAD` - aggregation: mean absolute deviation
* :py:func:`AGG_MAX` - aggregation: maximum value
* :py:func:`AGG_MIN` - aggregation: minimum value
* :py:func:`AGG_MEAN` - aggregation: mean
* :py:func:`AGG_MEDIAN` - aggregation: median
* :py:func:`AGG_PROD` - aggregation: product
* :py:func:`AGG_SAMPLE` - aggregation: time sampling (sample the indicator data at the specified time points)
* :py:func:`AGG_SAMPLE_MAX` - aggregation: interval maximum (the maximum of the indicator data within the specified time interval)
* :py:func:`AGG_SAMPLE_MIN` - aggregation: interval minimum (the minimum of the indicator data within the specified time interval)
* :py:func:`AGG_SAMPLE_MEAN` - aggregation: interval mean (the mean of the indicator data within the specified time interval)
* :py:func:`AGG_SUM` - aggregation: total sum
* :py:func:`AGG_STD` - aggregation: standard deviation
* :py:func:`AGG_VAR` - aggregation: variance
* :py:func:`AGG_QUANTILE` - aggregation: quantile
* :py:func:`AGG_VWAP` - aggregation: volume-weighted average price
* :py:func:`AGG_FUNC` - aggregation: custom aggregation function

**Grouping Indicators [supporter-only]**

* :py:func:`GROUP_COUNT` - grouping: count of non-null values
* :py:func:`GROUP_MAX` - grouping: group-wise cumulative maximum
* :py:func:`GROUP_MIN` - grouping: group-wise cumulative minimum
* :py:func:`GROUP_MEAN` - grouping: group-wise cumulative mean
* :py:func:`GROUP_PROD` - grouping: group-wise cumulative product
* :py:func:`GROUP_SUM` - grouping: group-wise cumulative sum
* :py:func:`GROUP_FUNC` - grouping: custom group calculation function


**Other Conversion Helpers**

* :py:func:`concat_to_df` - concatenate a list of indicators into a DataFrame
* :py:func:`df_to_ind` - convert a specified column of a DataFrame into an indicator
