Miscellaneous Functions
=======================

.. note::

    The arrow related functions such as get_inds_views, get_market_view and KData.to_pyarrow require the additional installation of the hikyuu_plugin package.

    ``python -m pip install hikyuu-plugin``

    Before using them, the plugin needs to be imported with ``from hikyuu_plugin.extra import *``.

.. py:function:: get_market_view(stks[, date=Datetime(), market='SH']) -> pandas.DataFrame

    Get the market data of the specified stock collection on the specified trading day, excluding the stocks that are suspended that day and have no data. If the date is not specified, the market data of the last trading day is returned;
    if the market data is also being received, it is the real-time market data.

    Note: this function depends on the daily data
    
    :param list[Stock] stks: the stock list
    :param Datetime date: get the market data of the specified date
    :param str market: the market code
    :return: the last market data of the specified stock list
    :rtype: pandas.DataFrame

    ::

        In [2]: get_market_view([s for s in sm])
        Out[2]: 
            证券代码        证券名称       日期     开盘价     最高价     最低价     收盘价  ...  涨跌幅（%）    振幅(%)  换手率(%)        总市值      流通市值    市净率  动态市盈率
        0     SZ301632       C广东建科 2025-08-13     32.840     46.580     32.040     43.600  ...    28.197589  44.275274  78.480086  1.824922e+06  3.002447e+05  6.827756 -272.500006
        1     SZ159271     恒生指数ETF 2025-08-13      1.002      1.016      1.002      1.016  ...     1.905717   1.397206        NaN           NaN           NaN       NaN         NaN
        2     SH589850   科创50ETF东财 2025-08-13      1.018      1.029      1.017      1.026  ...     0.489716   1.178782        NaN           NaN           NaN       NaN         NaN
        3     SZ180901        润泽REIT 2025-08-13      6.120      6.139      6.040      6.090  ...    -0.620104   1.617647        NaN           NaN           NaN       NaN         NaN
        4     SH589200  科创200ETF工银 2025-08-13      1.037      1.065      1.037      1.063  ...     2.507232   2.700096        NaN           NaN           NaN       NaN         NaN
        ...        ...             ...        ...        ...        ...        ...        ...  ...          ...        ...        ...           ...           ...       ...         ...
        8060  SH600159        大龙地产 2025-08-13      3.000      3.000      2.920      2.930  ...    -1.677852   2.666667   1.624909  2.431909e+05  2.431909e+05  1.408722 -732.499965
        8061  SZ000548        湖南投资 2025-08-13      5.670      5.700      5.630      5.640  ...    -0.529101   1.234568   1.362903  2.815577e+05  2.815399e+05  1.369198   35.250001
        8062  SH603180        金牌家居 2025-08-13     21.260     21.410     20.930     20.970  ...    -1.317647   2.257761   0.946926  3.234771e+05  3.234771e+05  1.153155   22.793478
        8063  SH601003        柳钢股份 2025-08-13      6.060      6.090      6.010      6.040  ...    -0.330033   1.320132   1.529113  1.547927e+06  1.547927e+06  1.792604   15.100000
        8064  SH000129         180波动 2025-08-13  15259.241  15280.421  15175.761  15190.970  ...    -0.368136   0.685879        NaN           NaN           NaN       NaN         NaN

        [8065 rows x 17 columns]

        In [3]: get_market_view([s for s in sm], Datetime(20250812))
        Out[3]: 
            证券代码        证券名称       日期     开盘价     最高价     最低价     收盘价  ...  涨跌幅（%）    振幅(%)  换手率(%)        总市值      流通市值    市净率  动态市盈率
        0     SZ301632       C广东建科 2025-08-12     32.660     40.000     30.020     34.010  ...     4.133497  30.557257  84.592471  1.423523e+06  2.342046e+05  5.325963 -212.562505
        1     SZ159271     恒生指数ETF 2025-08-12      0.993      0.997      0.991      0.997  ...     0.402820   0.604230        NaN           NaN           NaN       NaN         NaN
        2     SH589850   科创50ETF东财 2025-08-12      1.001      1.027      0.996      1.021  ...     1.998002   3.096903        NaN           NaN           NaN       NaN         NaN
        3     SZ180901        润泽REIT 2025-08-12      6.006      6.145      6.003      6.128  ...     1.038747   2.364302        NaN           NaN           NaN       NaN         NaN
        4     SH589200  科创200ETF工银 2025-08-12      1.035      1.039      1.020      1.037  ...    -0.096339   1.835749        NaN           NaN           NaN       NaN         NaN
        ...        ...             ...        ...        ...        ...        ...        ...  ...          ...        ...        ...           ...           ...       ...         ...
        8059  SH600159        大龙地产 2025-08-12      2.990      3.020      2.940      2.980  ...    -0.334448   2.675585   2.259280  2.473410e+05  2.473410e+05  1.432761 -744.999965
        8060  SZ000548        湖南投资 2025-08-12      5.670      5.700      5.640      5.670  ...     0.176678   1.058201   1.138197  2.830554e+05  2.830375e+05  1.376481   35.437501
        8061  SH603180        金牌家居 2025-08-12     21.430     21.520     21.210     21.250  ...    -0.793651   1.446570   0.703566  3.277963e+05  3.277963e+05  1.168552   23.097826
        8062  SH601003        柳钢股份 2025-08-12      6.110      6.180      6.040      6.060  ...    -0.818331   2.291326   1.396695  1.553053e+06  1.553053e+06  1.798540   15.150000
        8063  SH000129         180波动 2025-08-12  15230.810  15321.111  15229.380  15247.100  ...     0.206570   0.602273        NaN           NaN           NaN       NaN         NaN

        [8064 rows x 17 columns]
        


.. py:function:: concat_to_df(dates, ind_list[, head_stock_code=True, head_ind_name=False])
    
    Merge the indicators in the list into one pandas DataFrame

    :param DatetimeList dates: the specified date list
    :param sequence ind_list: the list of the calculated indicators
    :param bool head_ind_name: whether the table header uses the indicator names
    :param bool head_stock_code: whether the table header uses the security codes
    :return: the merged DataFrame, with dates as the index (note: the dates column is of the Datetime type)

    ::

        query = Query(-200)
        k_list = [stk.get_kdata(query) for stk in [sm['sz000001'], sm['sz000002']]]
        ma_list = [MA(CLOSE(k)) for k in k_list]
        df = concat_to_df(sm.get_trading_calendar(query), ma_list, head_stock_code=True, head_ind_name=False)
        df

                date	SZ000001	SZ000002
        0	2023-05-12 00:00:00	12.620000	15.060000
        1	2023-05-15 00:00:00	12.725000	15.060000
        2	2023-05-16 00:00:00	12.690000	15.010000
        3	2023-05-17 00:00:00	12.640000	14.952500
        4	2023-05-18 00:00:00	12.610000	14.886000
        ...	...	...	...
        195	2024-03-01 00:00:00	9.950455	9.837273
        196	2024-03-04 00:00:00	9.995909	9.838182
        197	2024-03-05 00:00:00	10.038182	9.816364
        198	2024-03-06 00:00:00	10.070455	9.776818
        199	2024-03-07 00:00:00	10.101364	9.738182


.. py:function:: get_inds_view(stks, inds, date[, cal_len=100, ktype=Query.DAY, market='SH']) -> pandas.DataFrame

    Way 1: get the indicator results of each security on the specified date

      :param stks: the security list
      :param list[Indicator] inds: the indicator list
      :param Datetime date: the specified date
      :param int cal_len: the data length needed to calculate
      :param str ktype: the k-line type
      :param str market: the specified market (used for the date alignment)

    ::

        In [4]: get_inds_view(sm, [OPEN(),CLOSE(),MA(CLOSE()), AMA(CLOSE()), MA(CLOSE(), 20)], Datetime(20250822))
        Out[4]:
            证券代码               证券名称   交易时间      OPEN     CLOSE           MA          AMA           MA
        0     SZ399295                 创价值 2025-08-22  4928.540  5041.720  4710.365909  4909.715525  4718.344500
        1     SH688630               芯碁微装 2025-08-22   124.110   129.000   124.648182   129.966137   127.810500
        2     SH600605               汇通能源 2025-08-22    35.250    34.910    37.600000    37.489924    37.400000
        3     SH000852               中证1000 2025-08-22  7250.190  7362.940  6932.661364  7184.920216  6955.541000
        4     SH000001               上证指数 2025-08-22  3772.280  3825.760  3656.552273  3753.987779  3662.238000
        ...        ...                    ...        ...       ...       ...          ...          ...          ...
        8479  BJ920101               志高机械 2025-08-22    52.630    52.300    54.744286    54.284372    54.744286
        8480  SZ159280  港股通互联网ETF汇添富 2025-08-22     1.019     1.031     1.032600     1.031993     1.032600
        8481  SH563620      自由现金流全指ETF 2025-08-22     1.012     1.013     1.011667     1.011238     1.011667
        8482  SZ159369      创业板50ETF易方达 2025-08-22     1.002     1.039     1.024000     1.022333     1.024000
        8483  SZ159283        通用航空ETF南方 2025-08-22     1.006     1.026     1.016500     1.015444     1.016500

        [8484 rows x 8 columns]

    Way 2: get the indicator results of each security calculated with the specified Query; the result will contain all the trading dates of the specified market included in the specified Query

    get_inds_view(stks, inds, query[, market='SH']) -> pandas.DataFrame

      :param stks: the specified security list
      :param list[Indicator] inds: the specified indicator list
      :param Query query: the query condition
      :param str market: the specified market (used for the date alignment)

    ::

        In [5]: get_inds_view(sm, [OPEN(),CLOSE(),MA(CLOSE()), AMA(CLOSE()), MA(CLOSE(), 20)], Query(-2000))
        Out[5]:
                证券代码         证券名称   交易时间      OPEN     CLOSE           MA          AMA           MA
        0         SZ399295           创价值 2017-06-05  3000.070  3003.850  3003.850000  3003.850000  3003.850000
        1         SZ399295           创价值 2017-06-06  2998.360  3011.500  3007.675000  3007.250000  3007.675000
        2         SZ399295           创价值 2017-06-07  3004.490  3065.660  3027.003333  3033.210000  3027.003333
        3         SZ399295           创价值 2017-06-08  3058.980  3058.970  3034.995000  3040.975491  3034.995000
        4         SZ399295           创价值 2017-06-09  3055.200  3060.110  3040.018000  3046.784218  3040.018000
        ...            ...              ...        ...       ...       ...          ...          ...          ...
        16367995  SZ159283  通用航空ETF南方 2025-08-18       NaN       NaN          NaN          NaN          NaN
        16367996  SZ159283  通用航空ETF南方 2025-08-19       NaN       NaN          NaN          NaN          NaN
        16367997  SZ159283  通用航空ETF南方 2025-08-20       NaN       NaN          NaN          NaN          NaN
        16367998  SZ159283  通用航空ETF南方 2025-08-21     1.012     1.007     1.007000     1.007000     1.007000
        16367999  SZ159283  通用航空ETF南方 2025-08-22     1.006     1.026     1.016500     1.015444     1.016500

        [16368000 rows x 8 columns]


.. py:function:: df_to_ind(df, col_name, col_date=None)
    
    Convert the specified column of a pandas.DataFrame to an Indicator

    :param df: pandas.DataFrame
    :param col_name: the specified column name
    :param col_date: the specified date column name (ignored when it is None; otherwise this column is the corresponding reference date)
    :return: Indicator

    ::

        # Example: get the 10-year US treasury yield from akshare:
        import akshare as ak
        df = ak.bond_zh_us_rate("19901219")
        x = df_to_ind(df, '美国国债收益率10年', '日期')


.. py:function:: parallel_run_sys(sys_list, query[, reset=False, reset_all=False]) -> List[FundsList]

    Run multiple systems in parallel, and return a list of FundsList, the assets of each account (within the query time range)

    :param sys_list: the system list
    :param query: the query condition
    :param bool reset: whether to reset according to the sharing attributes of the system parts before executing
    :param bool reset_all: forcibly reset all the parts

.. py:function:: parallel_run_pf(pf_list, query[, force=False]) -> List[FundsList]

    Execute multiple portfolio strategies in parallel, and return a list of FundsList, the assets of each account (within the query time range)

    :param list pf_list: the portfolio list
    :param Query query: the query condition
    :param bool force: force recalculating


.. py:function:: multi_regression(stk, query, *inds) -> list

    Perform a multiple linear regression analysis on the stock, using the return of the stock close price as the dependent variable and the input indicators as the independent variables for the multiple linear regression.

    The regression model is: Y = alpha + beta1*X1 + beta2*X2 + ... + betan*Xn

    .. note::

        NaN handling strategy: if any factor or the return at a time point is NaN, the data at that time point is discarded, but it does not affect the data at the other time points and the other factors.

    :param Stock stk: the stock object
    :param KQuery query: the K-line query condition, used to get the time range and the data type needed for the regression analysis
    :param Indicator \*inds: one or more indicators as the independent variables (the factors)
    :return: the list of the regression coefficients; the first element is alpha (the intercept), followed by each beta coefficient
    :rtype: list

    ::

        >>> stk = getStock('sh000001')
        >>> result = multi_regression(stk, KQuery(-252), MA(CLOSE(), 5), MACD(CLOSE())[0], RSI(CLOSE(), 14))
        >>> alpha = result[0]  # the intercept
        >>> beta1 = result[1]  # the coefficient of the first factor
        >>> beta2 = result[2]  # the coefficient of the second factor
        >>> beta3 = result[3]  # the coefficient of the third factor


.. py:function:: multi_regression_full(stk, query, \*inds) -> list

    Perform a multiple linear regression analysis on the stock (the full version), returning the complete regression result, including the coefficients, the residual sequence, the residual sum of squares and the R² value

    :param Stock stk: the stock object
    :param KQuery query: the K-line query condition
    :param Indicator \*inds: one or more indicators as the independent variables (the factors)
    :return: the list of the regression results, in the format:
             [alpha, beta1, beta2, ..., betan, e1, e2, ..., en, RSS, R²]
             - alpha: the intercept
             - beta1~betan: the coefficient of each factor
             - e1~en: the residual of each data point (the actual value - the predicted value)
             - RSS: the residual sum of squares
             - R²: the coefficient of determination
    :rtype: list

    ::

        >>> stk = getStock('sh000001')
        >>> result = multi_regression_full(stk, KQuery(-252), MA(CLOSE(), 5), MA(CLOSE(), 10))
        >>> alpha = result[0]
        >>> beta1 = result[1]
        >>> beta2 = result[2]
        >>> residuals = result[3:-2]  # the residual sequence
        >>> RSS = result[-2]
        >>> R_squared = result[-1]
